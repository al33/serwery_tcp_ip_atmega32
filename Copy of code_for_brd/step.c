/*
 * step.c
 *
 *  Created on: 04-10-2012
 *      Author: Robert
 */

#include "step.h"

uint8_t start_stepper = 0;
uint8_t steps_state_h = 0;
uint8_t steps_state_v = 0;
uint8_t steps_received = 0;
uint8_t steps_todo = 0;
uint8_t steps_cmd = 0;
uint8_t right_dir = 0;
uint8_t left_dir = 0;
uint8_t up_dir = 0;
uint8_t down_dir = 0;

extern uint8_t ms2_flag;


void silnik_hold(void){
	PORTA |= (A1);
	//PORTA |= (C1);
	steps_todo = 0;
}

void silnik_stop(void) {
	//PORTA &= ~(A1|A2|B1|B2);
	PORTA &= ~(C1|C2|D1|D2);
}

/* funkcja wykonuj¹ca cyklicznie kroki (obrót w lewo) */
void kroki_lewo(void) {
	static uint8_t kr;

	if( kr == 0 ) { KROK1H; }
	if( kr == 1 ) { KROK2H; }
	if( kr == 2 ) { KROK3H; }
	if( kr == 3 ) { KROK4H; }

	if( ++kr > 3 ) kr=0;
}

/* funkcja wykonuj¹ca cyklicznie kroki (obrót w prawo) */
void kroki_prawo(void) {
	static uint8_t kr;

	if( kr == 0 ) { KROK4H; }
	if( kr == 1 ) { KROK3H; }
	if( kr == 2 ) { KROK2H; }
	if( kr == 3 ) { KROK1H; }

	if( ++kr > 3 ) kr=0;
}

/* funkcja wykonuj¹ca cyklicznie kroki (obrót w gore) */
void kroki_gora(void) {
	static uint8_t kr;

	if( kr == 0 ) { KROK1V; }
	if( kr == 1 ) { KROK2V; }
	if( kr == 2 ) { KROK3V; }
	if( kr == 3 ) { KROK4V; }

	if( ++kr > 3 ) kr=0;
}

/* funkcja wykonuj¹ca cyklicznie kroki (obrót w dol) */
void kroki_dol(void) {
	static uint8_t kr;

	if( kr == 0 ) { KROK4V; }
	if( kr == 1 ) { KROK3V; }
	if( kr == 2 ) { KROK2V; }
	if( kr == 3 ) { KROK1V; }

	if( ++kr > 3 ) kr=0;
}

void stepper_move(void) {
	if(start_stepper && steps_todo){
		if(left_dir){
			s1_led_on();
			if(ms2_flag){
				kroki_lewo();
				steps_todo --;
				ms2_flag=0;
			}
		}
		if(right_dir)
		{
			s1_led_on();
			if(ms2_flag){
				kroki_prawo();
				steps_todo --;
				ms2_flag=0;
			}
		}
		if(down_dir){
			s2_led_on();
			if(ms2_flag){
				kroki_dol();
				steps_todo --;
				ms2_flag=0;
			}
		}
		if(up_dir){
			s2_led_on();
			if(ms2_flag){
				kroki_gora();
				steps_todo --;
				ms2_flag=0;
			}
		}
	}
	else{
		silnik_hold();
		silnik_stop();
		start_stepper = 0;
		s1_led_off();
		s2_led_off();
	}
}
//wyliczenie ilosci i kierunku krokow
void prepare_steps(uint8_t axis, uint8_t steps_received){
	if(axis == OX){
		if(steps_state_h > steps_received){
			left_dir = 1;
			right_dir = 0;
			up_dir = 0;
			down_dir = 0;
			steps_todo = (steps_state_h - steps_received);
			steps_state_h = steps_received;
		}
		else if(steps_received > steps_state_h){
			left_dir = 0;
			right_dir = 1;
			up_dir = 0;
			down_dir = 0;
			steps_todo = (steps_received - steps_state_h);
			steps_state_h = steps_received;
		}
	}
	else{
		//Krecenie po osi OY
		if(steps_state_v > steps_received){
			down_dir = 1;
			up_dir = 0;
			left_dir = 0;
			right_dir = 0;
			steps_todo = (steps_state_v - steps_received);
			steps_state_v = steps_received;
		}
		else if(steps_received > steps_state_v){
			down_dir = 0;
			up_dir = 1;
			left_dir = 0;
			right_dir = 0;
			steps_todo = (steps_received - steps_state_v);
			steps_state_v = steps_received;
		}

	}
}

void stepper_run() {
	start_stepper = 1;
}
