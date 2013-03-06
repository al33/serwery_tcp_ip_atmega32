/*
 * led.c
 *
 *  Created on: 06-03-2013
 *      Author: Robert
 */

#include "led.h"

void led_startup_block(void) {
	led_block_flag = 1;
}

void led_startup_unblock(void) {
	led_block_flag = 0;
}
