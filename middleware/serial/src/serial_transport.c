#include "serial_transport.h"

#include "mx_usart1.h"
#include "stm32_hal.h"
#include "task.h"

#include <stddef.h>
#include <stdint.h>

#define SERIAL_RX_RING_SIZE 256U
#define SERIAL_TX_RING_SIZE 256U
#define SERIAL_DMA_RX_SIZE  64U

static lwrb_t rx_rb;
static lwrb_t tx_rb;
static uint8_t rx_rb_data[SERIAL_RX_RING_SIZE];
static uint8_t tx_rb_data[SERIAL_TX_RING_SIZE];
static uint8_t dma_rx_data[SERIAL_DMA_RX_SIZE];
static TaskHandle_t notify_task;
static volatile uint32_t dma_rx_position;
static volatile uint32_t tx_dma_length;
static volatile uint32_t tx_dma_complete_length;
static volatile uint8_t tx_dma_active;
static volatile uint8_t tx_dma_complete_pending;
static volatile uint8_t tx_dma_error_pending;
static volatile uint8_t rx_restart_pending;

static void serial_notify_from_isr(void) {
    BaseType_t higher_priority_task_woken = pdFALSE;

    if (notify_task != NULL) {
        (void)xTaskNotifyFromISR(notify_task, 0U, eNoAction, &higher_priority_task_woken);
        portYIELD_FROM_ISR(higher_priority_task_woken);
    }
}

static void serial_copy_dma_rx(uint32_t size_byte) {
    if (size_byte > SERIAL_DMA_RX_SIZE) {
        size_byte = SERIAL_DMA_RX_SIZE;
    }
    if (size_byte > dma_rx_position) {
        (void)lwrb_write(&rx_rb, &dma_rx_data[dma_rx_position],
                         (lwrb_sz_t)(size_byte - dma_rx_position));
        dma_rx_position = size_byte;
    }
}

static void serial_restart_rx(void) {
    dma_rx_position = 0U;
    if (HAL_UART_ReceiveToIdle_DMA(mx_usart1_uart_gethandle(), dma_rx_data,
                                   SERIAL_DMA_RX_SIZE) != HAL_OK) {
        rx_restart_pending = 1U;
    }
}

static void serial_service_rx_restart(void) {
    if (rx_restart_pending != 0U &&
        HAL_UART_GetRxState(mx_usart1_uart_gethandle()) == HAL_UART_RX_STATE_IDLE) {
        rx_restart_pending = 0U;
        serial_restart_rx();
    }
}

BaseType_t serial_transport_init(TaskHandle_t task) {
    notify_task = task;
    dma_rx_position = 0U;
    tx_dma_length = 0U;
    tx_dma_complete_length = 0U;
    tx_dma_active = 0U;
    tx_dma_complete_pending = 0U;
    tx_dma_error_pending = 0U;
    rx_restart_pending = 0U;

    if (lwrb_init(&rx_rb, rx_rb_data, sizeof(rx_rb_data)) == 0U ||
        lwrb_init(&tx_rb, tx_rb_data, sizeof(tx_rb_data)) == 0U) {
        return pdFAIL;
    }

    if (HAL_UART_ReceiveToIdle_DMA(mx_usart1_uart_gethandle(), dma_rx_data,
                                   SERIAL_DMA_RX_SIZE) != HAL_OK) {
        return pdFAIL;
    }
    return pdPASS;
}

void serial_transport_deinit(void) {
    if (HAL_UART_GetRxState(mx_usart1_uart_gethandle()) == HAL_UART_RX_STATE_ACTIVE) {
        (void)HAL_UART_AbortReceive(mx_usart1_uart_gethandle());
    }
    if (HAL_UART_GetTxState(mx_usart1_uart_gethandle()) == HAL_UART_TX_STATE_ACTIVE) {
        (void)HAL_UART_AbortTransmit(mx_usart1_uart_gethandle());
    }
    notify_task = NULL;
    dma_rx_position = 0U;
    tx_dma_length = 0U;
    tx_dma_complete_length = 0U;
    tx_dma_active = 0U;
    tx_dma_complete_pending = 0U;
    tx_dma_error_pending = 0U;
    rx_restart_pending = 0U;
}

lwrb_t* serial_transport_rx_buffer(void) {
    return &rx_rb;
}

lwrb_t* serial_transport_tx_buffer(void) {
    return &tx_rb;
}

void serial_transport_poll_tx(void) {
    void* address;
    lwrb_sz_t length;

    serial_service_rx_restart();

    taskENTER_CRITICAL();
    if (tx_dma_error_pending != 0U) {
        tx_dma_error_pending = 0U;
        tx_dma_complete_pending = 0U;
        tx_dma_complete_length = 0U;
        tx_dma_active = 0U;
        tx_dma_length = 0U;
    } else if (tx_dma_complete_pending != 0U) {
        (void)lwrb_skip(&tx_rb, tx_dma_complete_length);
        tx_dma_complete_pending = 0U;
        tx_dma_complete_length = 0U;
        tx_dma_length = 0U;
        tx_dma_active = 0U;
    }
    if (tx_dma_active != 0U) {
        taskEXIT_CRITICAL();
        return;
    }

    length = lwrb_get_linear_block_read_length(&tx_rb);
    address = lwrb_get_linear_block_read_address(&tx_rb);
    if (length == 0U || address == NULL) {
        taskEXIT_CRITICAL();
        return;
    }

    tx_dma_length = length;
    tx_dma_active = 1U;
    taskEXIT_CRITICAL();

    if (HAL_UART_Transmit_DMA(mx_usart1_uart_gethandle(), address, (uint32_t)length) != HAL_OK) {
        taskENTER_CRITICAL();
        tx_dma_active = 0U;
        tx_dma_length = 0U;
        taskEXIT_CRITICAL();
    }
}

void HAL_UART_RxHalfCpltCallback(hal_uart_handle_t* huart) {
    if (huart == mx_usart1_uart_gethandle()) {
        serial_copy_dma_rx(SERIAL_DMA_RX_SIZE / 2U);
        serial_notify_from_isr();
    }
}

void HAL_UART_RxCpltCallback(hal_uart_handle_t* huart, uint32_t size_byte,
                             hal_uart_rx_event_types_t rx_event) {
    (void)rx_event;
    if (huart == mx_usart1_uart_gethandle()) {
        serial_copy_dma_rx(size_byte);
        serial_restart_rx();
        serial_notify_from_isr();
    }
}

void HAL_UART_TxCpltCallback(hal_uart_handle_t* huart) {
    if (huart == mx_usart1_uart_gethandle()) {
        tx_dma_complete_length = tx_dma_length;
        tx_dma_complete_pending = 1U;
        serial_notify_from_isr();
    }
}

void HAL_UART_ErrorCallback(hal_uart_handle_t* huart) {
    if (huart == mx_usart1_uart_gethandle()) {
        const hal_uart_tx_state_t tx_state = HAL_UART_GetTxState(huart);
        const hal_uart_rx_state_t rx_state = HAL_UART_GetRxState(huart);

        if (tx_dma_active != 0U && tx_state == HAL_UART_TX_STATE_IDLE) {
            tx_dma_error_pending = 1U;
        }
        if (rx_state == HAL_UART_RX_STATE_IDLE) {
            rx_restart_pending = 1U;
        }
        serial_notify_from_isr();
    }
}
