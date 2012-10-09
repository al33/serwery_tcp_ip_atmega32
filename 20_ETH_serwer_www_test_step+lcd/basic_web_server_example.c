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


/*STEPPER VARIABLES*/
#define STEPS 100
volatile uint8_t ms2_flag;
extern uint8_t steps_cmd;
extern uint8_t start_stepper;
extern int8_t steps_state;
extern uint8_t steps_received;
extern int8_t steps_todo;
extern int8_t oversteps;
extern uint8_t right_dir;
extern uint8_t left_dir;
/*END OF STEPPER VARIABLES*/


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
                	if(find_key_val(str, gStrbuf,5,"ox")){
                		steps_received = atoi(gStrbuf);
                		steps_state += steps_received;
                		left_dir = 1;
                		right_dir = 0;
                		lcd_locate(0,0);
                		lcd_str_P( PSTR("rcv:") );
                		lcd_int(steps_received);
                		lcd_locate(0,8);
                		lcd_str_P( PSTR("sta:") );
                		lcd_int(steps_state);
                		if(steps_state > STEPS){
                			oversteps = steps_state % STEPS;
                			lcd_locate(1,0);
                			lcd_str_P( PSTR("over:") );
                			lcd_int(oversteps);
                			steps_state = STEPS - 1;
                			steps_todo = steps_received - oversteps;
                			lcd_locate(1,8);
                			lcd_str_P( PSTR("todo:") );
                			lcd_int(steps_todo);
                		}
                		else{
                			steps_todo = steps_received;
                		}
                	}
                	if(find_key_val(str, gStrbuf,5,"oy")){
                		steps_received = atoi(gStrbuf);
                		steps_state -= steps_received;
                		right_dir = 1;
                		left_dir = 0;
                  		if(steps_state < 0){
                  			oversteps = (steps_state*-1) % STEPS;
                  			steps_state = 0;
                  			steps_todo = steps_received - oversteps;
                  		}
                      	else{
                      		steps_todo = steps_received;
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

// prepare the webpage by writing the data to the tcp send buffer
uint16_t print_webpage(uint8_t *buf, uint8_t on)
{
        uint16_t plen;
        plen=http200ok();
        plen=fill_tcp_data_p(buf,plen,PSTR("<pre>"));
        plen=fill_tcp_data_p(buf,plen,PSTR("<font color='green' size='6'><b>Witaj !</b>\n</font>"));
        plen=fill_tcp_data_p(buf,plen,PSTR("<font color='blue'><i>twój serwer www dzia³a znakomicie</i>\n\n</font>"));
        if(on){
               	   plen=fill_tcp_data_p(buf,plen,PSTR(" <font color=#00FF00>ON</font>"));
                   plen=fill_tcp_data_p(buf,plen,PSTR(" <a href=\"./?sw=0\">[switch off]</a>\n"));
                   plen=fill_tcp_data_p(buf,plen,PSTR(" <a href=\"./?sw=0\">Moj_off</a>\n"));
              }
        else{
                   plen=fill_tcp_data_p(buf,plen,PSTR("OFF"));
                   plen=fill_tcp_data_p(buf,plen,PSTR(" <a href=\"./?sw=1\">[switch on]</a>\n"));
                   plen=fill_tcp_data_p(buf,plen,PSTR(" <a href=\"./?sw=1\">Moj_on</a>\n"));
            }

        /*STEPPER*/
                   plen=fill_tcp_data_p(buf,plen,PSTR("<hr><br><form METHOD=get action=\""));
                   plen=fill_tcp_data_p(buf,plen,PSTR("\">\n<input type=hidden name=sw value=2>\nLEFT<input size=20 type=text name=ox>\n<br>"));
                   plen=fill_tcp_data_p(buf,plen,PSTR("\">\n<br><input type=submit value=\"MOVE LEFT\"></form>\n"));

                   plen=fill_tcp_data_p(buf,plen,PSTR("<hr><br><form METHOD=get action=\""));
                   plen=fill_tcp_data_p(buf,plen,PSTR("\">\n<input type=hidden name=sw value=2>\nRIGHT<input size=20 type=text name=oy>\n<br>"));
                   plen=fill_tcp_data_p(buf,plen,PSTR("\">\n<br><input type=submit value=\"MOVE RIGHT\"></form>\n"));

                   plen=fill_tcp_data_p(buf,plen,PSTR(" <a href=\"./?sw=3\">TURN OFF STEPPER</a>\n"));


        plen=fill_tcp_data_p(buf,plen,PSTR("\n<a href=\".\">[refresh status]</a>\n"));
        plen=fill_tcp_data_p(buf,plen,PSTR("</pre>\n"));
        return(plen);
}



int main(void){

	/*LCD INIT SECTION*/
		DDRA |= (1<<PA0);
		PORTA |= (1<<PA0);
		lcd_init();
		lcd_str_P( PSTR("LCD INIT OK!") );

/* ustawienie TIMER0 dla F_CPU=16MHz */
		TCCR0 |= (1<<WGM01);				/* tryb CTC */
		TCCR0 |= (1<<CS02)|(1<<CS00);		/* preskaler = 1024 */
		OCR0 = 39;							//przepelnienie dla 400Hz IDEANE KROKI!
		TIMSK |= (1<<OCIE0);				/* zezwolenie na przerwanie CompareMatch */
/* przerwanie wykonywane z czêstotliwoœci¹ ok 2,5ms (400 razy na sekundê) */

		sei();
        uint16_t dat_p;
        uint8_t cmd;
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

        while(1){
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
                }
                else {
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
	}

