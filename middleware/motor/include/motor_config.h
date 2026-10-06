#ifndef MOTOR_CONFIG_H
#define MOTOR_CONFIG_H

/* Deliberately disabled: enabling this runs A then B forward for ten seconds. */
#ifndef MOTOR_POLARITY_TEST_ENABLED
#define MOTOR_POLARITY_TEST_ENABLED 0U
#endif

#define MOTOR_TEST_DUTY      3600U /* 50% of the TIM8 period (7199). */
#define MOTOR_TEST_HOLD_MS   10000U
#define MOTOR_TEST_PAUSE_MS  1000U

#endif /* MOTOR_CONFIG_H */
