#ifndef MOTOR_CONFIG_H
#define MOTOR_CONFIG_H

/* Deliberately disabled: enabling this runs both motors forward continuously. */
#ifndef MOTOR_POLARITY_TEST_ENABLED
#define MOTOR_POLARITY_TEST_ENABLED 0U
#endif

#define MOTOR_TEST_DUTY      3600U /* 50% of the TIM8 period (7199). */

#endif /* MOTOR_CONFIG_H */
