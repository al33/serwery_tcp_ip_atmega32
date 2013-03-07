/*
 * pages.h
 *
 *  Created on: 26-02-2013
 *      Author: Robert
 */

#ifndef PAGES_H_
#define PAGES_H_

#include <avr/io.h>
#include <stdio.h>

#include "step.h"
#include "lib/ip_arp_udp_tcp.h"

#define BUFFER_SIZE 850
extern uint8_t buf[BUFFER_SIZE+1];
extern volatile uint8_t ms2_flag;
//zmienne pozycji silnikow
extern uint8_t steps_state_h;
extern uint8_t steps_state_v;

uint16_t http200ok(void);
uint16_t http200okjs(void);
uint16_t http200okcss(void);
uint16_t print_js(void);
uint16_t print_css(void);
uint16_t print_webpage(uint8_t *buf);

#endif /* PAGES_H_ */
