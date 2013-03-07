/*
 * url.c
 *
 *  Created on: 26-02-2013
 *      Author: Robert
 */

#include "url.h"
#include "step.h"

uint8_t analyse_get_url(char *str) {
	char result[10];
	uint8_t axis = 0;
	uint8_t steps_received = 0;
	uint8_t action = 0;

	//sprawdzanie js
	if (strncmp("/slider.js",str,9)==0)
		return JS_ACTION;
	//sprawdzanie css
	else if (strncmp("/style.css", str, 9)==0)
		return CSS_ACTION;

	// str jest koncem adresu url po /
	if (find_key_val(str, result, 5, "sw")) {
		action = atoi(result);
		switch(action){
			case STEPPER_ACTION:
				if(find_key_val(str, result,5,"ox")) {
						steps_received = atoi(result);
						axis = OX;
				}
				if(find_key_val(str, result,5,"oy")) {
						steps_received = atoi(result);
						axis = OY;
				}
				prepare_steps(axis, steps_received);
				return STEPPER_ACTION;
		}
	}
	return UNKNOWN_ACTION;
}

uint8_t find_key_val(char *str,char *strbuf, uint8_t maxlen,char *key) {
	uint8_t found=0;
	uint8_t i=0;
	char *kp;
	kp=key;
	while(*str &&  *str!=' ' && *str!='\r' && found==0){
		if (*str == *kp){
			// At the beginning of the key we must check
			// if this is the start of the key otherwise we will
			// match on 'foobar' when only looking for 'bar', by andras tucsni
			if (kp==key &&  ! ( *(str-1) == '?' || *(str-1) == '&' ) ) goto NEXT;
			kp++;
			if (*kp == '\0'){
				str++;
				kp=key;
				if (*str == '='){
					found=1;
				}
			}
		}else{
			kp=key;
		}
NEXT:
		str++;
	}
	if (found==1){
		// copy the value to a buffer and terminate it with '\0'
		while(*str &&  *str!=' ' && *str!='\r' && *str!='&' && i<maxlen-1){
			*strbuf=*str;
			i++;
			str++;
			strbuf++;
		}
		*strbuf='\0';
	}
	// return 1 if found (the string in strbuf might still be an empty string)
	// otherwise return 0
	return(found);
}
