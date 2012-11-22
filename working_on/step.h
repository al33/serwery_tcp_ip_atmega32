/*
 * step.h
 *
 *  Created on: 04-10-2012
 *      Author: Robert
 */

#ifndef STEP_H_
#define STEP_H_

/* definicje pinów steruj¹cych na osi OX */
/*
#define A1 (1<<PD6)
#define A2 (1<<PD5)
#define B1 (1<<PD4)
#define B2 (1<<PD3)
*/
#define A1 (1<<PA0)
#define A2 (1<<PA1)
#define B1 (1<<PA2)
#define B2 (1<<PA3)

/* definicje pinow sterujacych na osi OY */

#define C1 (1<<PC7)
#define C2 (1<<PC6)
#define D1 (1<<PC5)
#define D2 (1<<PC4)




/* definicje kroków steruj¹cych prac¹ silnika na osi OX */
#define KROK1H PORTA |= A1|B1; PORTA &= ~(A2|B2)
#define KROK2H PORTA |= A2|B1; PORTA &= ~(A1|B2)
#define KROK3H PORTA |= A2|B2; PORTA &= ~(A1|B1)
#define KROK4H PORTA |= A1|B2; PORTA &= ~(A2|B1)

/* definicje kroków steruj¹cych prac¹ silnika na osi OY */
#define KROK1V PORTC |= C1|D1; PORTC &= ~(C2|D2)
#define KROK2V PORTC |= C2|D1; PORTC &= ~(C1|D2)
#define KROK3V PORTC |= C2|D2; PORTC &= ~(C1|D1)
#define KROK4V PORTC |= C1|D2; PORTC &= ~(C2|D1)

void silnik_stop(void);
void kroki_lewo(void);
void kroki_prawo(void);
void kroki_gora(void);
void kroki_dol(void);

#endif /* STEP_H_ */
