/*
 * led.h
 *
 *  Created on: 08-12-2012
 *      Author: Robert
 */

#ifndef LED_H_
#define LED_H_

#define S1_LED (1<<PD6)
#define S1_LED_ON PORTD &= ~S1_LED
#define S1_LED_OFF PORTD |= S1_LED
#define S2_LED (1<<PD5)
#define S2_LED_ON PORTD &= ~S2_LED
#define S2_LED_OFF PORTD |= S2_LED

#define DATA_REC_LED (1<<PC0)
#define DATA_REC_LED_ON PORTC &= ~DATA_REC_LED
#define DATA_REC_LED_OFF PORTC |= DATA_REC_LED
#define DATA_SEND_LED (1<<PC1)
#define DATA_SEND_LED_ON PORTC &= ~DATA_SEND_LED
#define DATA_SEND_LED_OFF PORTC |= DATA_SEND_LED


#endif /* LED_H_ */
