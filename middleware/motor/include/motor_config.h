#ifndef MOTOR_CONFIG_H
#define MOTOR_CONFIG_H

/* Deliberately disabled: enabling this runs A then B forward for one second. */
#ifndef MOTOR_POLARITY_TEST_ENABLED
#define MOTOR_POLARITY_TEST_ENABLED 0U
#endif

#define MOTOR_TEST_DUTY      2520U /* 35% of the TIM8 period (7199). */
#define MOTOR_TEST_HOLD_MS   1000U
#define MOTOR_TEST_PAUSE_MS  1000U

#endif /* MOTOR_CONFIG_H */
