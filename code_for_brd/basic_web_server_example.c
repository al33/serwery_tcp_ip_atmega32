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
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ip_arp_udp_tcp.h"
#include "enc28j60.h"
#include "websrv_help_functions.h"
#include "util/delay.h"
#include "net.h"
#include "step.h"
#include "led.h"

/*STEPPER VARIABLES*/
volatile uint8_t ms2_flag;
extern uint8_t steps_cmd;
extern uint8_t start_stepper;
extern uint8_t steps_state_h;
extern uint8_t steps_state_v;
extern uint8_t steps_received;
extern uint8_t steps_todo;
extern uint8_t right_dir;
extern uint8_t left_dir;
extern uint8_t up_dir;
extern uint8_t down_dir;
/*END OF STEPPER VARIABLES*/

volatile uint8_t s1_flag;	/* flaga tykni�cia timera co 1 sekund� */
volatile uint8_t sekundy;	/* licznik sekund 0-59 */
volatile uint16_t ms2_cnt;
volatile uint8_t s1_stop_flag = 0; //flaga wylaczenia silnika s1 po dotknieciu krancowki na INT1
volatile uint8_t s2_stop_flag = 0; //flaga wylaczenia silnika s2 po dotknieciu krancowki na INT0


// ustalamy adres MAC
static uint8_t mymac[6] = {0x00,0x55,0x58,0x10,0x00,0x29};
// ustalamy adres IP urz�dzenia
static uint8_t myip[4] = {192,168,1,110};

// server listen port for www
#define MYWWWPORT 80

#define BUFFER_SIZE 850
static uint8_t buf[BUFFER_SIZE+1];
static char gStrbuf[25];

//Inicjalizacja led i silnika
void led_step_init(void){
	uint8_t init_flag = 0;
	while(init_flag == 0){
		if(sekundy == 0){
			S1_LED_ON;
			S2_LED_ON;
			DATA_REC_LED_ON;
		}
		if(sekundy > 1)
			init_flag = 1;
		if(sekundy == 1){
			S2_LED_ON;
		}
		if(sekundy == 2){
			DATA_REC_LED_ON;
		}
		if(sekundy > 3){
			init_flag = 1;
		}
	}
	S1_LED_OFF;
	S2_LED_OFF;
	DATA_REC_LED_OFF;
	//Krecenie silnikiem s1 az do dotkniecia krancowki na INT1
	while(s1_stop_flag == 0){
		if(ms2_flag){
			kroki_prawo();
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

//ANALIZA URLA
int8_t analyse_get_url(char *str)
{

        //uint8_t loop=15;
        // the first slash:
        if (*str == '/'){
                str++;
        }else{
                return(-1);
        }

        if (strncmp("slider.js",str,9)==0){
                    return(10);
        }

        if (strncmp("style.css", str, 9)==0){
        			return(11);
        }


        // str is now end of ip adress
        if (find_key_val(str,gStrbuf,5,"sw")){
                if (gStrbuf[0]=='0'){
                        return(0);
                }
                if (gStrbuf[0]=='1'){
                        return(1);
                }
                if(gStrbuf[0]=='2'){
                	steps_cmd = 2;
                }
                if(gStrbuf[0]=='3'){
                	return(3);
                }
        }
        //STEP SETTINGS
        if (steps_cmd==2){
        	//Krecenie po osi OX
                	if(find_key_val(str, gStrbuf,5,"ox")){
                		steps_received = atoi(gStrbuf);
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
            //Krecenie po osi OY
                	if(find_key_val(str, gStrbuf,5,"oy")){
                		steps_received = atoi(gStrbuf);
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
                	return(2);
        }
        return(-3);
}
//KONIEC ANALIZY URLA

uint16_t http200ok(void)
{
        return(fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n")));
}

uint16_t http200okjs(void)
{
        return(fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 200 OK\r\nContent-Type: application/x-javascript\r\nPragma: no-cache\r\n\r\n")));
}

uint16_t http200okcss(void)
{
        return(fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/css\r\nPragma: no-cache\r\n\r\n")));
}

//slider.js
uint16_t print_js(void)
{
	uint16_t plen;
	plen = http200okjs();
	plen = fill_tcp_data_p(buf, plen, PSTR("function showValue(e,t){document.getElementById(t).innerHTML=e}"));
	return(plen);
}

uint16_t print_css(void)
{
	uint16_t plen;
	plen = http200okcss();
	plen = fill_tcp_data_p(buf, plen, PSTR("b{color: red;} html{margin-left: auto; margin-right: auto; text-align: center;}"));
	plen = fill_tcp_data_p(buf, plen, PSTR(".slV {-webkit-appearance: slider-vertical;}"));
	return(plen);
}


// prepare the webpage by writing the data to the tcp send buffer
uint16_t print_webpage(uint8_t *buf, uint8_t on)
{
        uint16_t plen;
        char ox[200];
        char oy[200];
        plen=http200ok();
        //plen=fill_tcp_data_p(buf,plen,PSTR("<pre>"));
        plen=fill_tcp_data_p(buf, plen, PSTR("<link rel=stylesheet type=text/css href=style.css />"));
        plen=fill_tcp_data_p(buf,plen,PSTR("<script src=slider.js></script>"));
        plen=fill_tcp_data_p(buf,plen,PSTR("<b>Kamera sterowana protoko�em TCP/IP</b>"));
        /*if(on){
               	   plen=fill_tcp_data_p(buf,plen,PSTR(" <font color=#00FF00>ON</font>"));
                   plen=fill_tcp_data_p(buf,plen,PSTR(" <a href=\"./?sw=0\">[switch off]</a>\n"));
              }
        else{
                   plen=fill_tcp_data_p(buf,plen,PSTR("OFF"));
                   plen=fill_tcp_data_p(buf,plen,PSTR(" <a href=\"./?sw=1\">[switch on]</a>\n"));
            }*/

        //STEPPER + JS OX
        plen=fill_tcp_data_p(buf,plen,PSTR("<hr><form method=get/>"));
        plen=fill_tcp_data_p(buf,plen,PSTR("<input type=hidden name=sw value=2/>"));
        sprintf(ox, "Obr�t w poziomie:<br> <input type=range name=ox min=0 max=100 step=5 value=%d onchange=\"showValue(this.value,'rangeH')\"/><br><span>Pozycja: </span><span id=rangeH>%d</span>", steps_state_h, steps_state_h);
        plen=fill_tcp_data(buf, plen, ox);
        plen=fill_tcp_data_p(buf,plen,PSTR("<br><input type=submit value=Start></form>"));

        //STEPPER + JS OY
        plen=fill_tcp_data_p(buf,plen,PSTR("<hr><form method=get/>"));
        plen=fill_tcp_data_p(buf,plen,PSTR("<input type=hidden name=sw value=2/>"));
        sprintf(oy, "\nObr�t w pionie:<br> <input type=range class=\"slV\" name=oy min=\"0\" max=\"100\" step=\"5\" value=%d onchange=\"showValue(this.value,'rangeV')\"/><br><span>Pozycja: </span><span id=rangeV>%d</span>", steps_state_v, steps_state_v);
        plen=fill_tcp_data(buf, plen, oy);
        plen=fill_tcp_data_p(buf,plen,PSTR("<br><input type=submit value=\"Start\"></form>"));
        plen=fill_tcp_data_p(buf, plen, PSTR("<hr><p>Atmega32+enc28j60 Robert Mleczko 2013</p>"));

        //plen=fill_tcp_data_p(buf,plen,PSTR("\n<a href=\".\">[refresh status]</a>\n"));
        //plen=fill_tcp_data_p(buf,plen,PSTR("</pre>\n"));

        return(plen);
}



int main(void){


		//ustawienie TIMER0 dla F_CPU=20MHz
		TCCR0 |= (1<<WGM01);				 //tryb CTC
		TCCR0 |= (1<<CS02)|(1<<CS00);		 //preskaler = 1024
		//OCR0 = 48;							//przepelnienie dla 400Hz IDEANE KROKI!
		OCR0 = 97;		//200Hz
		//OCR0 = 130;			//150Hz
		//OCR0 = 195;		//100Hz
		TIMSK |= (1<<OCIE0);				 //zezwolenie na przerwanie CompareMatch
		//przerwanie wykonywane z cz�stotliwo�ci� ok 2,5ms (400 razy na sekund�)

		//Ustawienie przerwa� na INT1, zbocze opadaj�ce
		MCUCR |= (1<<ISC11);
		GICR |= (1<<INT1);
		PORTD |= (1<<PD3);

		//Ustawienie przerwan na INT0, zbocze opadajace
		MCUCR |= (1<<ISC01);
		GICR |= (1<<INT0);
		PORTD |= (1<<PD2);

        uint16_t dat_p;
        int8_t cmd;
        uint16_t plen;

        silnik_stop();

        // Dioda LED na PD7:
        DDRD|= (1<<PD7);
        PORTD &= ~(1<<PORTD7);// Dioda OFF

        //Init diod LED dla silnikow
        DDRD |= S1_LED;
        DDRD |= S2_LED;
        S1_LED_OFF;
        S2_LED_OFF;

        //Init diod LED dla przesylu danych
        DDRD |= DATA_REC_LED;
        DATA_REC_LED_OFF;

        sei(); //odblokowanie przerwan

        //led_step_init(); //inicjalizacja silnika i diod

        //initialize the hardware driver for the enc28j60
        enc28j60Init(mymac);
        enc28j60PhyWrite(PHLCON,0x476);
        
        //init the ethernet/ip layer:
        init_ip_arp_udp_tcp(mymac,myip,MYWWWPORT);

        led_step_init(); //inicjalizacja silnika i diod

        while(1){

                // read packet, handle ping and wait for a tcp packet:
                dat_p=packetloop_icmp_tcp(buf,enc28j60PacketReceive(BUFFER_SIZE, buf));
                if(dat_p){
                	DATA_REC_LED_ON;
                }


//!!!!!!!!!!	REQUESTY DO SILNIKA PO TYM KOMENTARZU BO if(dat_p==0)
//PRZY DRUGIM PRZEBIEGU PETLI OMIJA CALEGO while() !!!!!!!!!!
                //led_step_init(); //tu nie dziala

                if(start_stepper && steps_todo)
               	{
                	if(left_dir){
                		S1_LED_ON;
                		if(ms2_flag){
                			kroki_lewo();
                			steps_todo --;
                			ms2_flag=0;
                		}
                	}
                	if(right_dir)
                	{
                		S1_LED_ON;
                		if(ms2_flag){
                			kroki_prawo();
                			steps_todo --;
                		    ms2_flag=0;
                		}
                	}
                	if(down_dir){
                		S2_LED_ON;
                		if(ms2_flag){
                			kroki_dol();
                			steps_todo --;
                			ms2_flag=0;
                		}
                	}
                	if(up_dir){
                		S2_LED_ON;
                		if(ms2_flag){
                			kroki_gora();
                			steps_todo --;
                			ms2_flag=0;
                		}
                	}
                }
                else{
                	silnik_hold();
					//silnik_stop();
					start_stepper = 0;
					S1_LED_OFF;
					S2_LED_OFF;
				}




//!!!!!!!!!!	REQUESTY DO SILNIKA PRZED TYM KOMENTARZEM BO if(dat_p==0)
//PRZY DRUGIM PRZEBIEGU PETLI OMIJA CALEGO while() !!!!!!!!!!

                /* dat_p will be unequal to zero if there is a valid 
                 * http get */
                if(dat_p==0){
                	DATA_REC_LED_OFF;
                	//led_step_init();
                        // no http request
                        continue;
                }
                // tcp port 80 begin
                if (strncmp("GET ",(char *)&(buf[dat_p]),4)!=0){
                        // head, post and other methods:
                		plen=http200ok();
                        plen=fill_tcp_data_p(buf,plen,PSTR("<h1>200 OK</h1>"));
                        goto SENDTCP;
                }
                // just one web page in the "root directory" of the web server
                if (strncmp("/ ",(char *)&(buf[dat_p+4]),2)==0){
                		plen=http200ok();
						plen=print_webpage(buf,(PORTD & (1<<PORTD7)));
                        goto SENDTCP;
                }
                cmd=analyse_get_url((char *)&(buf[dat_p+4]));
                                // for possible status codes see:
                                // http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
                                if (cmd==-1){
                                        plen=fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 401 Unauthorized\r\nContent-Type: text/html\r\n\r\n<h1>401 Unauthorized</h1>"));
                                        goto SENDTCP;
                                }
                                if (cmd==1){
                                        //LEDON;
                                        PORTD|= (1<<PORTD7);// transistor on
                                }
                                if (cmd==0){
                                        //LEDOFF;
                                        PORTD &= ~(1<<PORTD7);// transistor off
                                }
                                if (cmd==2){
                                       start_stepper = 1;
                                }
                                if (cmd==3)
                                {
                                	start_stepper = 0;
                                	silnik_stop();
                                }
                                if (cmd==10){
                                	plen=http200okjs();
                                	plen=print_js();
                                	goto SENDTCP;
                                }
                                if(cmd==11){
                                	plen=http200okcss();
                                	plen=print_css();
                                	goto SENDTCP;
                                }

                                // if (cmd==-2) or any other value
                                // just display the status:
                                plen=print_webpage(buf,(PORTD & (1<<PORTD7)));
SENDTCP:
				www_server_reply(buf,plen);
                // tcp port 80 end

        }
        return (0);
}




/* ================= PROCEDURA OBS�UGI PRZERWANIA � COMPARE MATCH */
/* pe�ni funkcj� timera programowego wyznaczaj�cego podstaw� czasu = 2,5ms */
ISR(TIMER0_COMP_vect){
		ms2_flag = 1;	/* ustawiamy flag� co 2,5ms */

		if(++ms2_cnt>399) {	/* gdy licznik ms > 499 (min�a 1 sekunda) */
				s1_flag=1;	/* ustaw flag� tykni�cia sekundy */
				sekundy++;	/* zwi�ksz licznik sekund */
				if(sekundy>59) sekundy=0; /* je�li ilo�� sekund > 59 - wyzeruj */
				ms2_cnt=0;	/* wyzeruj licznik setnych ms */
			}
	}

//OBSLUGA PRZERWANIA INT1
ISR(INT1_vect){
	s1_stop_flag = 1;
	silnik_hold();
	steps_todo = 0;
	steps_state_h = 0;
}

//OBSLUGA PRZERWANIA INT1
ISR(INT0_vect){
	s2_stop_flag = 1;
	silnik_hold();
	steps_todo = 0;
	steps_state_v = 0;
}