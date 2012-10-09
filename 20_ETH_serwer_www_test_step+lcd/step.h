/*
 * step.h
 *
 *  Created on: 04-10-2012
 *      Author: Robert
 */

#ifndef STEP_H_
#define STEP_H_

/* przydatne definicje pinów steruj¹cych */
#define A1 (1<<PD6)
#define A2 (1<<PD5)
#define B1 (1<<PD4)
#define B2 (1<<PD3)

/* definicje kroków steruj¹cych prac¹ silnika */
#define KROK1 PORTD |= A1|B1; PORTD &= ~(A2|B2)
#define KROK2 PORTD |= A2|B1; PORTD &= ~(A1|B2)
#define KROK3 PORTD |= A2|B2; PORTD &= ~(A1|B1)
#define KROK4 PORTD |= A1|B2; PORTD &= ~(A2|B1)

void silnik_stop(void);
void kroki_lewo(void);
void kroki_prawo(void);
void check_and_step(void);

#endif /* STEP_H_ */
