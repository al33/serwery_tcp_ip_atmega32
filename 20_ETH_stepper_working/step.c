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

extern volatile uint8_t ms2_flag;
extern uint8_t step_x;

enum edir {lewo, prawo, stop};
enum edir dir = lewo;


void silnik_stop(void);
void kroki_lewo(void);
void kroki_prawo(void);
void check_and_step(void);

	void silnik_stop(void) {
		PORTD &= ~(A1|A2|B1|B2);
	}

	/* funkcja wykonuj¹ca cyklicznie kroki (obrót w lewo) */
	void kroki_lewo(void) {
		static uint8_t kr;

		if( kr == 0 ) { KROK1; }
		if( kr == 1 ) { KROK2; }
		if( kr == 2 ) { KROK3; }
		if( kr == 3 ) { KROK4; }

		if( ++kr > 3 ) kr=0;
	}

	/* funkcja wykonuj¹ca cyklicznie kroki (obrót w prawo) */
	void kroki_prawo(void) {
		static uint8_t kr;

		if( kr == 0 ) { KROK4; }
		if( kr == 1 ) { KROK3; }
		if( kr == 2 ) { KROK2; }
		if( kr == 3 ) { KROK1; }

		if( ++kr > 3 ) kr=0;
	}
	void check_and_step(void)
	{
		if(ms2_flag && step_x > 0)
		{
			kroki_lewo();
			ms2_flag=0;
		}
	}
