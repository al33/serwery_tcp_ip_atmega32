/*
 * step.c
 *
 *  Created on: 19-09-2012
 *      Author: Robert
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "step.h"

volatile uint8_t s1_flag;	/* flaga tykni�cia timera co 1 sekund� */
volatile uint8_t sekundy;	/* licznik sekund 0-59 */

//volatile uint8_t ms2_flag;
volatile uint16_t ms2_cnt;

enum edir {lewo, prawo, stop};
enum edir dir = lewo;

uint8_t steps=0;

void silnik_stop(void);
void kroki_lewo(void);
void kroki_prawo(void);

	void silnik_stop(void) {
		PORTD &= ~(A1|A2|B1|B2);
	}

	/* funkcja wykonuj�ca cyklicznie kroki (obr�t w lewo) */
	void kroki_lewo(void) {
		static uint8_t kr;

		if( kr == 0 ) { KROK1; }
		if( kr == 1 ) { KROK2; }
		if( kr == 2 ) { KROK3; }
		if( kr == 3 ) { KROK4; }

		if( ++kr > 3 ) kr=0;
	}

	/* funkcja wykonuj�ca cyklicznie kroki (obr�t w prawo) */
	void kroki_prawo(void) {
		static uint8_t kr;

		if( kr == 0 ) { KROK4; }
		if( kr == 1 ) { KROK3; }
		if( kr == 2 ) { KROK2; }
		if( kr == 3 ) { KROK1; }

		if( ++kr > 3 ) kr=0;
	}



	/* ================= PROCEDURA OBS�UGI PRZERWANIA � COMPARE MATCH */
	/* pe�ni funkcj� timera programowego wyznaczaj�cego podstaw� czasu = 2ms oraz 1s */
	//ISR(TIMER0_COMP_vect)
	//{
	//	ms2_flag = 1;	/* ustawiamy flag� co 2ms */
	//
	//	if(++ms2_cnt>499) {	/* gdy licznik ms > 499 (min�a 1 sekunda) */
	//		s1_flag=1;	/* ustaw flag� tykni�cia sekundy */
	//		sekundy++;	/* zwi�ksz licznik sekund */
	//		if(sekundy>59) sekundy=0; /* je�li ilo�� sekund > 59 - wyzeruj */
	//		ms2_cnt=0;	/* wyzeruj licznik setnych ms */
	//	}
	//}
