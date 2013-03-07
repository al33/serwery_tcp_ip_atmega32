/*
 * led.c
 *
 *  Created on: 06-03-2013
 *      Author: Robert
 */

#include "led.h"

void led_init(void) {
    // Dioda LED na PD7:
    DDRD|= (1<<PD7);
    PORTD &= ~(1<<PORTD7);// Dioda OFF

    //Init diod LED dla silnikow
    DDRD |= S1_LED;
    DDRD |= S2_LED;

    //Init diod LED dla przesylu danych
    DDRD |= DATA_REC_LED;
    led_startup_on();
}

//blokowanie diody przy uruchomieniu ukladu
void led_block(void) {
	led_block_flag = 1;
}
//odblokowanie diody po uruchomieniu ukladu
void led_unblock(void) {
	led_block_flag = 0;
}
//Wlaczanie diod przy uruchomieniu ukladu
void led_startup_on(void)  {
	led_block();
	S1_LED_ON;
	S2_LED_ON;
	DATA_REC_LED_ON;
}
//Gaszenie i odblokowanie diod po uruchomieniu ukladu
void led_startup_off(void) {
	S1_LED_OFF;
	S2_LED_OFF;
	DATA_REC_LED_OFF;
	led_unblock();
}


void s1_led_on(void) {
	if(!led_block_flag)
		S1_LED_ON;
}

void s1_led_off(void) {
	if(!led_block_flag)
		S1_LED_OFF;
}

void s2_led_on(void) {
	if(!led_block_flag)
		S2_LED_ON;
}

void s2_led_off(void) {
	if(!led_block_flag)
		S2_LED_OFF;
}

void data_led_on(void) {
	if(!led_block_flag)
		DATA_REC_LED_ON;
}

void data_led_off(void) {
	if(!led_block_flag)
		DATA_REC_LED_OFF;
}
