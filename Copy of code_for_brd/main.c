/*********************************************
 * vim:sw=8:ts=8:si:et
 * To use the above modeline in vim you must have "set modeline" in your .vimrc
 * Author: Guido Socher
 * Copyright: GPL V2
 *
 * Tuxgraphics AVR webserver/ethernet board
 *
 * http://tuxgraphics.org/electronics/
 * Chip type           : Atmega88/168/328 with ENC28J60
 *
 *
 * MODYFIKACJE: Robert Mleczko --- ATMEGA32, Silniki krokowe, www, html5 + js
 *
 *
 *********************************************/

#include <avr/interrupt.h>
#include <util/delay.h>

#include "lib/ip_arp_udp_tcp.h"
#include "lib/enc28j60.h"
#include "lib/net.h"

#include "step.h"
#include "led.h"
#include "pages.h"
#include "url.h"

volatile uint8_t ms2_flag;
volatile uint8_t seconds2_flag;	/* flaga tykniêcia timera co 1 sekundê */
volatile uint16_t ms2_cnt;
volatile uint8_t s1_stop_flag = 0; //flaga wylaczenia silnika s1 po dotknieciu krancowki na INT1
volatile uint8_t s2_stop_flag = 0; //flaga wylaczenia silnika s2 po dotknieciu krancowki na INT0

// ustalamy adres MAC
static uint8_t mymac[6] = {0x00,0x55,0x58,0x10,0x00,0x29};
// ustalamy adres IP urz¹dzenia
static uint8_t myip[4] = {192,168,1,110};

// server listen port for www
#define MYWWWPORT 80

//#define BUFFER_SIZE 850
//static uint8_t buf[BUFFER_SIZE+1];
uint8_t buf[BUFFER_SIZE+1];

//Inicjalizacja led i silnika
void led_step_init(void){
/*	uint8_t init_flag = 0;
	while(init_flag == 0){
		if(sekundy == 0){
			S1_LED_ON;
			S2_LED_ON;
			DATA_REC_LED_ON;
		}
		else{
			seconds2_flag = 1;
		}
	}
	S1_LED_OFF;
	S2_LED_OFF;
	DATA_REC_LED_OFF;*/
	//Krecenie silnikiem s1 az do dotkniecia krancowki na INT1
	while(s1_stop_flag == 0){
		if(ms2_flag){
			kroki_lewo();
	        ms2_flag = 0;
	    }
	}
	silnik_hold(); //wazne by wylaczac silnik po kazdorazowym jego uzyciu!!!, czas ustawienia silnika nie moze byc wyzszy niz 1s
	//Krecenie silnikiem s2 az do dotkniecia krancowki na INT0
	while(s2_stop_flag == 0){
		if(ms2_flag){
			kroki_dol();
			ms2_flag = 0;
		}
	}
	//silnik_stop();
	silnik_hold();
}

void init() {
	//ustawienie TIMER0 dla F_CPU=20MHz
	TCCR0 |= (1<<WGM01);				 //tryb CTC
	TCCR0 |= (1<<CS02)|(1<<CS00);		 //preskaler = 1024
	//OCR0 = 48;							//przepelnienie dla 400Hz IDEANE KROKI!
	//OCR0 = 97;		//200Hz
	//OCR0 = 130;			//150Hz
	OCR0 = 195;		//100Hz
	TIMSK |= (1<<OCIE0);				 //zezwolenie na przerwanie CompareMatch
	//przerwanie wykonywane z czêstotliwoœci¹ ok 2,5ms (400 razy na sekundê)

	//Ustawienie przerwañ na INT1, zbocze opadaj¹ce
	MCUCR |= (1<<ISC11);
	GICR |= (1<<INT1);
	PORTD |= (1<<PD3);

	//Ustawienie przerwan na INT0, zbocze opadajace
	MCUCR |= (1<<ISC01);
	GICR |= (1<<INT0);
	PORTD |= (1<<PD2);

    silnik_stop();

    //initialize the hardware driver for the enc28j60
    enc28j60Init(mymac);
    enc28j60PhyWrite(PHLCON,0x476);

    init_ip_arp_udp_tcp(mymac,myip,MYWWWPORT);

    led_startup_on();
}


int main(void){
    uint16_t buffer_position;
    uint16_t response_length = 0;

	init();
	led_init();

    sei(); //odblokowanie przerwan

	led_step_init(); //inicjalizacja silnika i diod

	while(1){
		stepper_move();

		// read packet, handle ping and wait for a tcp packet:
		buffer_position = packetloop_icmp_tcp(buf,enc28j60PacketReceive(BUFFER_SIZE, buf));
		if(buffer_position){
			data_led_on();
		}
		else {
			data_led_off();
			continue;
		}

		// protocol other then GET
		if (strncmp("GET ",(char *) &(buf[buffer_position]),4) != 0){
			response_length = http200ok();
			www_server_reply(buf,response_length);
			continue;
		}

		if (strncmp("/ ", (char *) &(buf[buffer_position+4]), 2) == 0){
			response_length = print_webpage(buf);
		}
		else switch(analyse_get_url((char *) &(buf[buffer_position+4]))) {
			case UNKNOWN_ACTION:
				response_length = fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 401 Unauthorized\r\nContent-Type: text/html\r\n\r\n<h1>401 Unauthorized</h1>"));
				break;
			case STEPPER_ACTION:
				stepper_run();
				response_length = print_webpage(buf);
				break;
			case JS_ACTION:
				//response_length = http200okjs();
				response_length = print_js();
				break;
			case CSS_ACTION:
				//response_length = http200okcss();
				response_length = print_css();
				break;
			default:
				response_length = print_webpage(buf);
				break;
		}
		www_server_reply(buf,response_length);
	}
}

/* ================= PROCEDURA OBS£UGI PRZERWANIA – COMPARE MATCH */
/* pe³ni funkcjê timera programowego wyznaczaj¹cego podstawê czasu = 2,5ms */
ISR(TIMER0_COMP_vect){
	ms2_flag = 1;	/* ustawiamy flagê co 2ms */

	if(++ms2_cnt>200) {	/* gdy licznik ms > 499 (minê³a 1 sekunda) */
		led_startup_off();
	}
}

//OBSLUGA PRZERWANIA INT1
ISR(INT1_vect){
	s1_stop_flag = 1;
	silnik_hold();
	steps_state_h = 0;
}

//OBSLUGA PRZERWANIA INT1
ISR(INT0_vect){
	s2_stop_flag = 1;
	silnik_hold();
	steps_state_v = 0;
}
