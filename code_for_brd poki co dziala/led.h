/*
 * led.h
 *
 *  Created on: 08-12-2012
 *      Author: Robert
 */

#ifndef LED_H_
#define LED_H_

#define S1_LED (1<<PD4)
#define S1_LED_ON PORTD &= ~S1_LED
#define S1_LED_OFF PORTD |= S1_LED
#define S2_LED (1<<PD5)
#define S2_LED_ON PORTD &= ~S2_LED
#define S2_LED_OFF PORTD |= S2_LED

#define DATA_REC_LED (1<<PD6)
#define DATA_REC_LED_ON PORTD &= ~DATA_REC_LED
#define DATA_REC_LED_OFF PORTD |= DATA_REC_LED

#include <avr/io.h>

uint8_t led_block_flag;

#endif /* LED_H_ */
