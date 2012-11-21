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
 * MODYFIKACJE: Miros³aw Kardaœ --- ATmega32
 * MODYFIKACJE: Robert Mleczko --- Silniki krokowe
 *
 *
 *********************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>
#include "ip_arp_udp_tcp.h"
#include "enc28j60.h"
#include "util/delay.h"
#include "net.h"
#include "step.h"
#include "websrv_help_functions.h"

//LCD INCLUDE
#include "lcd44780.h"

//RS232 INCLUDE
#include "MKUART/mkuart.h"


/*STEPPER VARIABLES*/
#define STEPS 100
volatile uint8_t ms2_flag;
extern uint8_t steps_cmd;
extern uint8_t start_stepper;
extern uint8_t steps_state;
extern uint8_t steps_received;
extern uint8_t steps_todo;
extern uint8_t right_dir;
extern uint8_t left_dir;
extern uint8_t up_dir;
extern uint8_t down_dir;
/*END OF STEPPER VARIABLES*/

volatile uint8_t s1_flag;	/* flaga tykniêcia timera co 1 sekundê */
volatile uint8_t sekundy;	/* licznik sekund 0-59 */
volatile uint16_t ms2_cnt;


// ustalamy adres MAC
static uint8_t mymac[6] = {0x00,0x55,0x58,0x10,0x00,0x29};
// ustalamy adres IP urz¹dzenia
static uint8_t myip[4] = {192,168,0,110};

// server listen port for www
#define MYWWWPORT 80

#define BUFFER_SIZE 850
static uint8_t buf[BUFFER_SIZE+1];
static char gStrbuf[25];

//ANALIZA URLA
int8_t analyse_get_url(char *str)
{

    if (strncmp("slider.js",str,9)==0){
            return(10);
    }
        //uint8_t loop=15;
        // the first slash:
        if (*str == '/'){
                str++;
        }else{
                return(-1);
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
                		if(steps_state > steps_received){
                			left_dir = 1;
                			right_dir = 0;
                			steps_todo = (steps_state - steps_received);
                			steps_state = steps_received;
                		}
                		else if(steps_received > steps_state){
                			left_dir = 0;
                			right_dir = 1;
                		    steps_todo = (steps_received - steps_state);
                		    steps_state = steps_received;
                		}

                	}
            //Krecenie po osi OY
                	if(find_key_val(str, gStrbuf,5,"oy")){
                		steps_received = atoi(gStrbuf);
                		if(steps_state > steps_received){
                			down_dir = 1;
                			up_dir = 0;
                			steps_todo = (steps_state - steps_received);
                			steps_state = steps_received;
                		}
                		else if(steps_received > steps_state){
                			down_dir = 0;
                			up_dir = 1;
                			steps_todo = (steps_received - steps_state);
                			steps_state = steps_received;
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

//slider.js
uint16_t print_js(void)
{
	uint16_t plen;
	//plen = http200okjs();
	plen = fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 200 OK\r\nContent-Type: application/x-javascript\r\nPragma: no-cache\r\n\r\n"));
	plen = fill_tcp_data_p(buf, plen, PSTR("function showValue(e,t){document.getElementById(t).innerHTML=e}"));
	return(plen);
}


// prepare the webpage by writing the data to the tcp send buffer
uint16_t print_webpage(uint8_t *buf, uint8_t on)
{
        uint16_t plen;
        plen=http200ok();
        plen=fill_tcp_data_p(buf,plen,PSTR("<pre>"));
        plen=fill_tcp_data_p(buf,plen,PSTR("<font color='green' size='6'><b>Witaj !</b>\n</font>"));
        if(on){
               	   plen=fill_tcp_data_p(buf,plen,PSTR(" <font color=#00FF00>ON</font>"));
                   plen=fill_tcp_data_p(buf,plen,PSTR(" <a href=\"./?sw=0\">[switch off]</a>\n"));
              }
        else{
                   plen=fill_tcp_data_p(buf,plen,PSTR("OFF"));
                   plen=fill_tcp_data_p(buf,plen,PSTR(" <a href=\"./?sw=1\">[switch on]</a>\n"));
            }

        /*STEPPER
                   plen=fill_tcp_data_p(buf,plen,PSTR("<hr><br><form METHOD=get action=\""));
                   plen=fill_tcp_data_p(buf,plen,PSTR("\">\n<input type=hidden name=sw value=2>\nLEFT<input size=20 type=text name=ox>\n<br>"));
                   plen=fill_tcp_data_p(buf,plen,PSTR("\">\n<br><input type=submit value=\"MOVE LEFT\"></form>\n"));

                   plen=fill_tcp_data_p(buf,plen,PSTR("<hr><br><form METHOD=get action=\""));
                   plen=fill_tcp_data_p(buf,plen,PSTR("\">\n<input type=hidden name=sw value=2>\nRIGHT<input size=20 type=text name=oy>\n<br>"));
                   plen=fill_tcp_data_p(buf,plen,PSTR("\">\n<br><input type=submit value=\"MOVE RIGHT\"></form>\n"));
*/
        //STEPPER + JS OX
        plen=fill_tcp_data_p(buf,plen,PSTR("<hr><br><form method=get action=\""));
        plen=fill_tcp_data_p(buf,plen,PSTR("\"><input type=hidden name=sw value=2>"));
        plen=fill_tcp_data_p(buf,plen,PSTR("\">\nSTEPS Horizontal: <input type=range class=\"sliderH\" name=ox min=\"0\" max=\"100\" step=\"5\" value=0 onchange=\"showValue(this.value,'rangeH')\"/>"));
        plen=fill_tcp_data_p(buf,plen,PSTR("\"><span id=rangeH>0</span>"));
        plen=fill_tcp_data_p(buf,plen,PSTR("\"<script src=slider.js></script>"));
        //plen=fill_tcp_data_p(buf,plen,PSTR("function showValue(e,t){document.getElementById(t).innerHTML=e}"));
        plen=fill_tcp_data_p(buf,plen,PSTR("\">\n<br><input type=submit value=\"MOVE STEPPER OX\"></form>\n"));

        //STEPPER + JS OY
        /*plen=fill_tcp_data_p(buf,plen,PSTR("<hr><br><form method=get action=\""));
        plen=fill_tcp_data_p(buf,plen,PSTR("\"><input type=hidden name=sw value=2>"));
        plen=fill_tcp_data_p(buf,plen,PSTR("\">\nSTEPS Vertical: <input type=range class=\"sliderV\" name=oy min=\"0\" max=\"100\" step=\"5\" value=0 onchange=\"showValue(this.value, 'rangeV')\"/>"));
        plen=fill_tcp_data_p(buf,plen,PSTR("\"><span id=rangeV>0</span>"));
        plen=fill_tcp_data_p(buf,plen,PSTR("<script type=text/javascript>"));
        plen=fill_tcp_data_p(buf,plen,PSTR("function showValue(newValue, target){document.getElementById(target).innerHTML=newValue;}"));
        plen=fill_tcp_data_p(buf,plen,PSTR("</script>"));
        plen=fill_tcp_data_p(buf,plen,PSTR("\">\n<br><input type=submit value=\"MOVE STEPPER OY\"></form>\n"));
*/
        plen=fill_tcp_data_p(buf,plen,PSTR("\n<a href=\".\">[refresh status]</a>\n"));
        plen=fill_tcp_data_p(buf,plen,PSTR("</pre>\n"));
        return(plen);
}



int main(void){

	/*RS232 INIT SECTION*/
	USART_Init( __UBRR );

	/*LCD INIT SECTION*/
		DDRA |= (1<<PA0);
		PORTA |= (1<<PA0);
		lcd_init();
		lcd_str_P( PSTR("LCD INIT OK!") );

/*
 ustawienie TIMER0 dla F_CPU=16MHz
		TCCR0 |= (1<<WGM01);				 tryb CTC
		TCCR0 |= (1<<CS02)|(1<<CS00);		 preskaler = 1024
		OCR0 = 39;							//przepelnienie dla 400Hz IDEANE KROKI!
		TIMSK |= (1<<OCIE0);				 zezwolenie na przerwanie CompareMatch
 przerwanie wykonywane z czêstotliwoœci¹ ok 2,5ms (400 razy na sekundê)
*/
		//ustawienie TIMER0 dla F_CPU=20MHz
		TCCR0 |= (1<<WGM01);				 //tryb CTC
		TCCR0 |= (1<<CS02)|(1<<CS00);		 //preskaler = 1024
		OCR0 = 48;							//przepelnienie dla 400Hz IDEANE KROKI!
		TIMSK |= (1<<OCIE0);				 //zezwolenie na przerwanie CompareMatch
		//przerwanie wykonywane z czêstotliwoœci¹ ok 2,5ms (400 razy na sekundê)

        uint16_t dat_p;
        int8_t cmd;
        uint16_t plen;

        silnik_stop();

        // Dioda LED na PD7:
        DDRD|= (1<<DDD7);
        PORTD &= ~(1<<PORTD7);// Dioda OFF

        //initialize the hardware driver for the enc28j60
        enc28j60Init(mymac);
        enc28j60PhyWrite(PHLCON,0x476);
        
        //init the ethernet/ip layer:
        init_ip_arp_udp_tcp(mymac,myip,MYWWWPORT);

        sei();

        uart_puts("RS232 OK!");
        uart_putc('\r');
        uart_putc('\n');

        while(1){

        	if(s1_flag){
        		uart_putc('.');
        		s1_flag = 0;
        	}

        	//if(ms2_flag){				//krecenie motorkiem bez przerwy
        		//kroki_lewo();
        		//ms2_flag=0;
        	//}
                // read packet, handle ping and wait for a tcp packet:
                dat_p=packetloop_icmp_tcp(buf,enc28j60PacketReceive(BUFFER_SIZE, buf));

//!!!!!!!!!!	REQUESTY DO SILNIKA PO TYM KOMENTARZU BO if(dat_p==0)
//PRZY DRUGIM PRZEBIEGU PETLI OMIJA CALEGO while() !!!!!!!!!!


                if(start_stepper && steps_todo)
               	{
                	if(left_dir){
                		if(ms2_flag){
                			kroki_lewo();
                			steps_todo --;
                			ms2_flag=0;
                		}
                	}
                	if(right_dir)
                	{
                		if(ms2_flag){
                			kroki_prawo();
                			steps_todo --;
                		    ms2_flag=0;
                		}
                	}
                	if(down_dir){
                		if(ms2_flag){
                			kroki_dol();
                			steps_todo --;
                			ms2_flag=0;
                		}
                	}
                	if(up_dir){
                		if(ms2_flag){
                			kroki_gora();
                			steps_todo --;
                			ms2_flag=0;
                		}
                	}
                }
                else{
					silnik_stop();
					start_stepper = 0;
				}




//!!!!!!!!!!	REQUESTY DO SILNIKA PRZED TYM KOMENTARZEM BO if(dat_p==0)
//PRZY DRUGIM PRZEBIEGU PETLI OMIJA CALEGO while() !!!!!!!!!!

                /* dat_p will be unequal to zero if there is a valid 
                 * http get */
                if(dat_p==0){
                        // no http request
                        continue;
                }
                // tcp port 80 begin
                if (strncmp("GET ",(char *)&(buf[dat_p]),4)!=0){
                        // head, post and other methods:
                		//plen=http200okjs();
                		plen=http200ok();
                		//plen = fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 200 OK\r\nContent-Type: application/x-javascript\r\nPragma: no-cache\r\n\r\n"));
                		//plen=print_js();
                        plen=fill_tcp_data_p(buf,plen,PSTR("<h1>200 OK</h1>"));
                        goto SENDTCP;
                }
                // just one web page in the "root directory" of the web server
                if (strncmp("/ ",(char *)&(buf[dat_p+4]),2)==0){
                		plen=http200ok();
                		//plen=http200okjs();
                		//plen=print_js();
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
                                	//plen=fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 200 OK\r\nContent-Type: application/x-javascript\r\nPragma: no-cache\r\n\r\n"));
                                	plen=print_js();
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




/* ================= PROCEDURA OBS£UGI PRZERWANIA – COMPARE MATCH */
/* pe³ni funkcjê timera programowego wyznaczaj¹cego podstawê czasu = 2,5ms */
ISR(TIMER0_COMP_vect){
		ms2_flag = 1;	/* ustawiamy flagê co 2,5ms */

		if(++ms2_cnt>399) {	/* gdy licznik ms > 499 (minê³a 1 sekunda) */
				s1_flag=1;	/* ustaw flagê tykniêcia sekundy */
				sekundy++;	/* zwiêksz licznik sekund */
				if(sekundy>59) sekundy=0; /* jeœli iloœæ sekund > 59 - wyzeruj */
				ms2_cnt=0;	/* wyzeruj licznik setnych ms */
			}
	}

