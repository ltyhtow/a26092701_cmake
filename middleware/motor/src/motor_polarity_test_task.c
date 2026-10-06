#include "motor_polarity_test_task.h"

#include "mx_tim8.h"

#include <stdint.h>

#define MOTOR_POLARITY_TEST_TASK_STACK_DEPTH_WORDS 256U

static void motor_outputs_set(hal_tim_handle_t *timer,
                              uint32_t motor_a_in1,
                              uint32_t motor_a_in2,
                              uint32_t motor_b_in1,
                              uint32_t motor_b_in2) {
    (void)HAL_TIM_OC_SetCompareUnitPulse(timer, HAL_TIM_OC_COMPARE_UNIT_1,
                                         motor_a_in1);
    (void)HAL_TIM_OC_SetCompareUnitPulse(timer, HAL_TIM_OC_COMPARE_UNIT_2,
                                         motor_a_in2);
    (void)HAL_TIM_OC_SetCompareUnitPulse(timer, HAL_TIM_OC_COMPARE_UNIT_3,
                                         motor_b_in1);
    (void)HAL_TIM_OC_SetCompareUnitPulse(timer, HAL_TIM_OC_COMPARE_UNIT_4,
                                         motor_b_in2);
    (void)HAL_TIM_GenerateEvent(timer, HAL_TIM_SW_EVENT_UPD);
}

static BaseType_t motor_outputs_start(hal_tim_handle_t *timer) {
    if (HAL_TIM_OC_StartChannel(timer, HAL_TIM_CHANNEL_1) != HAL_OK ||
        HAL_TIM_OC_StartChannel(timer, HAL_TIM_CHANNEL_2) != HAL_OK ||
        HAL_TIM_OC_StartChannel(timer, HAL_TIM_CHANNEL_3) != HAL_OK ||
        HAL_TIM_OC_StartChannel(timer, HAL_TIM_CHANNEL_4) != HAL_OK ||
        HAL_TIM_Start(timer) != HAL_OK ||
        HAL_TIM_BREAK_EnableMainOutput(timer) != HAL_OK) {
        return pdFAIL;
    }
    return pdPASS;
}

static void motor_polarity_test_task_entry(void *argument) {
    hal_tim_handle_t *timer = mx_tim8_gethandle();

    (void)argument;
    motor_outputs_set(timer, MOTOR_TEST_DUTY, 0U, MOTOR_TEST_DUTY, 0U);
    if (motor_outputs_start(timer) != pdPASS) {
        motor_outputs_set(timer, 0U, 0U, 0U, 0U);
        (void)HAL_TIM_BREAK_DisableMainOutput(timer);
        (void)HAL_TIM_Stop(timer);
        vTaskDelete(NULL);
        return;
    }

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000U));
    }
}

BaseType_t motor_polarity_test_task_start(TaskHandle_t *task_handle) {
    if (task_handle == NULL) {
        return pdFAIL;
    }
    return xTaskCreate(motor_polarity_test_task_entry, "MotorTest",
                       MOTOR_POLARITY_TEST_TASK_STACK_DEPTH_WORDS, NULL, 1U,
                       task_handle);
}
