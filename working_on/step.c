/*
 * step.c
 *
 *  Created on: 04-10-2012
 *      Author: Robert
 */



#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
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


void silnik_stop(void);
void kroki_lewo(void);
void kroki_prawo(void);
void kroki_gora(void);
void kroki_dol(void);

	void silnik_stop(void) {
		PORTA &= ~(A1|A2|B1|B2);
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

