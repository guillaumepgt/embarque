#include "servo.h"
#define PERIOD_TIMER   10
#define SERVO_TIMER_ID TIMER1_ID
#define SERVO_CHANNEL  TIM_CHANNEL_1
static uint16_t servo_position = 0;
static uint16_t current_position;

void SERVO_init(void)
{
	BSP_TIMER_run_us(TIMER1_ID, PERIOD_TIMER*1000, false);
	BSP_TIMER_enable_PWM(TIMER1_ID, TIM_CHANNEL_1, 150, false, false);
	SERVO_set_position(50);
}


void SERVO_set_position(uint16_t position)
{
	current_position = position;
    if (position > 99)
        position = 100;

    servo_position = position;

    uint16_t duty = 2*position+50;

    BSP_TIMER_set_duty(SERVO_TIMER_ID, SERVO_CHANNEL, duty);
}


void SERVO_process_test(void) {

	static uint16_t position = 0;
	SERVO_set_position(position);

	if( BSP_UART_button(UART2_ID) ) {

		position = (position > 99)?0:(position+10);
		SERVO_set_position(position);
	}
}

uint16_t SERVO_get_position() {
	return current_position;
}
