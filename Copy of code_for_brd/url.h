/*
 * url.h
 *
 *  Created on: 26-02-2013
 *      Author: Robert
 */

#ifndef URL_H_
#define URL_H_

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STEPPER_ACTION 2	//wymagana wartosc 2
#define JS_ACTION 3
#define CSS_ACTION 4
#define UNKNOWN_ACTION 5

uint8_t analyse_get_url(char *);
uint8_t find_key_val(char *, char *, uint8_t, char *);

#endif /* URL_H_ */
