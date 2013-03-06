/*
 * step.h
 *
 *  Created on: 04-10-2012
 *      Author: Robert
 */

#include <avr/io.h>
#include <util/delay.h>
#include "led.h"

#ifndef STEP_H_
#define STEP_H_

#define OX 1
#define OY 2

/* definicje pinów steruj¹cych na osi OX */
/*
#define A1 (1<<PD6)
#define A2 (1<<PD5)
#define B1 (1<<PD4)
#define B2 (1<<PD3)
*/
#define A1 (1<<PA3)
#define A2 (1<<PA2)
#define B1 (1<<PA1)
#define B2 (1<<PA0)

/* definicje pinow sterujacych na osi OY */
#define C1 (1<<PA7)
#define C2 (1<<PA6)
#define D1 (1<<PA5)
#define D2 (1<<PA4)


/* definicje kroków steruj¹cych prac¹ silnika na osi OX */
#define KROK1H PORTA |= A1|B1; PORTA &= ~(A2|B2)
#define KROK2H PORTA |= A2|B1; PORTA &= ~(A1|B2)
#define KROK3H PORTA |= A2|B2; PORTA &= ~(A1|B1)
#define KROK4H PORTA |= A1|B2; PORTA &= ~(A2|B1)

/* definicje kroków steruj¹cych prac¹ silnika na osi OY */
/*
#define KROK1V PORTC |= C1|D1; PORTC &= ~(C2|D2)
#define KROK2V PORTC |= C2|D1; PORTC &= ~(C1|D2)
#define KROK3V PORTC |= C2|D2; PORTC &= ~(C1|D1)
#define KROK4V PORTC |= C1|D2; PORTC &= ~(C2|D1)
*/
#define KROK1V PORTA |= C1|D1; PORTA &= ~(C2|D2)
#define KROK2V PORTA |= C2|D1; PORTA &= ~(C1|D2)
#define KROK3V PORTA |= C2|D2; PORTA &= ~(C1|D1)
#define KROK4V PORTA |= C1|D2; PORTA &= ~(C2|D1)


void silnik_stop(void);
void kroki_lewo(void);
void kroki_prawo(void);
void kroki_gora(void);
void kroki_dol(void);
void silnik_hold(void);
void stepper_move(void);
void prepare_steps(uint8_t axis, uint8_t steps_received);
void stepper_run(void);

#endif /* STEP_H_ */
