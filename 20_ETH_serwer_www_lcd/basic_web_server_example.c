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
 *
 *********************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "ip_arp_udp_tcp.h"
#include "enc28j60.h"
#include "util/delay.h"
#include "net.h"
#include "websrv_help_functions.h"

//LCD FILES
#include "lcd44780.h"


// ustalamy adres MAC
static uint8_t mymac[6] = {0x00,0x55,0x58,0x10,0x00,0x29};
// ustalamy adres IP urz¹dzenia
static uint8_t myip[4] = {192,168,0,110};

//pass to log in
static char password[]="mleko";

// server listen port for www
#define MYWWWPORT 80

#define BUFFER_SIZE 850
static uint8_t buf[BUFFER_SIZE+1];
static char gStrbuf[25];

// set output to VCC, LED off
#define LEDOFF PORTB|=(1<<PORTB1)
// set output to GND, LED on
#define LEDON PORTB&=~(1<<PORTB1)
// to test the state of the LED
#define LEDISOFF PORTB&(1<<PORTB1)

uint8_t verify_password(char *str)
{
        // the first characters of the received string are
        // a simple password/cookie:
        if (strncmp(password,str,strlen(password))==0){
                return(1);
        }
        return(0);
}

// analyse the url given
// return values: -1 invalid password
//                -2 no command given but password valid
//                -3 just refresh page
//                0 switch off
//                1 switch on
//                2 favicon.ico
//
//                The string passed to this function will look like this:
//                /password/?s=1 HTTP/1.....
//                /password/?s=0 HTTP/1.....
//                /password HTTP/1.....
int8_t analyse_get_url(char *str)
{
        uint8_t loop=15;
        uint8_t lcd_cmd=0;
        // the first slash:
        if (*str == '/'){
                str++;
        }else{
                return(-1);
        }
        if (strncmp("favicon.ico",str,11)==0){
                return(2);
        }
        // the password:
        if(verify_password(str)==0){
                return(-1);
        }
        // move forward to the first space or '/'
        while(loop){
                if(*str==' '){
                        // end of url and no slash after password:
                        return(-2);
                }
                if(*str=='/'){
                        // end of password
                        loop=0;
                        continue;
                }
                str++;
                loop--; // do not loop too long
        }
        // str is now something like password?sw=1 or just end of url
        if (find_key_val(str,gStrbuf,5,"sw")){
                if (gStrbuf[0]=='0'){
                        return(0);
                }
                if (gStrbuf[0]=='1'){
                        return(1);
                }
                //sw==5 uruchamia lcd
                if (gStrbuf[0]=='5'){
                	lcd_cmd=5;
                	//return(5);
                }
        }
        //if (find_key_val(str,gStrbuf,5,"cmd")){ //komenda cmd=5 uruchamia lcd
        //	if (gStrbuf[0]=='5'){
        //		lcd_cmd=5;
        //		return(5);
        //		}
        //}
        //wyswietlanie txt z www na lcd
        if (lcd_cmd==5){
        	lcd_cls();
        	//lcd_str("aaa");
        		if (find_key_val(str,gStrbuf,25,"l1")){
        			//lcd_str("aaa"); //test czy w ogole znajduje "l1"
        			urldecode(gStrbuf);
        	        lcd_str(gStrbuf);
        		}
        		if (find_key_val(str,gStrbuf,25,"l2")){
        			lcd_locate(1,0);
        	        urldecode(gStrbuf);
        	        lcd_str(gStrbuf);
        	    }
        }

        return(-3);
}

uint16_t http200ok(void)
{
        return(fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n")));
}

// answer HTTP/1.0 301 Moved Permanently\r\nLocation: .....\r\n\r\n
// to redirect
// type =0  : http://tuxgraphics.org/c.ico    favicon.ico file
// type =1  : /password/
uint16_t moved_perm(uint8_t *buf,uint8_t type)
{
        uint16_t plen;
        plen=fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 301 Moved Permanently\r\nLocation: "));
        if (type==1){
                plen=fill_tcp_data_p(buf,plen,PSTR("/"));
                plen=fill_tcp_data(buf,plen,password);
                plen=fill_tcp_data_p(buf,plen,PSTR("/"));
        }else{
                plen=fill_tcp_data_p(buf,plen,PSTR("http://tuxgraphics.org/c.ico"));
                //plen=fill_tcp_data_p(buf,plen,PSTR("http://tuxgraphics.org/ico/print.ico"));
        }
        plen=fill_tcp_data_p(buf,plen,PSTR("\r\n\r\nContent-Type: text/html\r\n\r\n"));
        plen=fill_tcp_data_p(buf,plen,PSTR("<h1>301 Moved Permanently</h1>\n"));
        return(plen);
}


// prepare the webpage by writing the data to the tcp send buffer
uint16_t print_webpage(uint8_t *buf,uint8_t on)
{
        uint16_t plen;
        plen=http200ok();
        plen=fill_tcp_data_p(buf,plen,PSTR("<h2>Eth remote switch</h2>\n<pre> "));
        //plen=fill_tcp_data_p(buf,plen,PSTR("<h2>printer switch</h2>\n<pre> "));
        if (on){
                plen=fill_tcp_data_p(buf,plen,PSTR(" <font color=#00FF00>ON</font>"));
                plen=fill_tcp_data_p(buf,plen,PSTR(" <a href=\"./?sw=0\">[switch off]</a>\n"));
				plen=fill_tcp_data_p(buf,plen,PSTR(" <a href=\"./?sw=0\">Moj_off</a>\n"));


        }else{
                plen=fill_tcp_data_p(buf,plen,PSTR("OFF"));
                plen=fill_tcp_data_p(buf,plen,PSTR(" <a href=\"./?sw=1\">[switch on]</a>\n"));
				plen=fill_tcp_data_p(buf,plen,PSTR(" <a href=\"./?sw=1\">Moj_on</a>\n"));
        }

        /*WRITING TO LCD*/
        plen=fill_tcp_data_p(buf,plen,PSTR("<hr><br><form METHOD=get action=\""));
                plen=fill_tcp_data_p(buf,plen,PSTR("\">\n<input type=hidden name=sw value=5>\n<input size=20 type=text name=l1>\n<br><input size=20 type=text name=l2>\n"));
                plen=fill_tcp_data_p(buf,plen,PSTR("\">\n<br><input type=submit value=\"write to LCD\"></form>\n"));


        plen=fill_tcp_data_p(buf,plen,PSTR("\n<a href=\".\">[refresh status]</a>\n"));
        plen=fill_tcp_data_p(buf,plen,PSTR("</pre><hr>tuxgraphics.org\n"));
        return(plen);
}

//LCD FLASH TAB
char PROGMEM tab1[] = {"FLASH"};

int main(void){

		/*LCD INIT SECTION*/
		DDRA |= (1<<PA0);
		PORTA |= (1<<PA0);

		lcd_init();

		lcd_str_P(tab1);				// napis z pamiêci FLASH
		lcd_locate(0,10);
		lcd_str_P( PSTR("Linia1") );	// napis z pamiêci FLASH
		lcd_locate(1,0);
		lcd_str_P(tab1);				// napis z pamiêci FLASH
		lcd_locate(1,10);
		lcd_str("Linia2");				// napis z pamiêci RAM
		/*END OF LCD INIT SECTION*/


        uint16_t plen;
        uint16_t dat_p;
        int8_t cmd;

        
        // set the clock speed to "no pre-scaler" (8MHz with internal osc or
        // full external speed)
        // set the clock prescaler. First write CLKPCE to enable setting
        // of clock the next four instructions.
        // Note that the CKDIV8 Fuse determines the initial
        // value of the CKKPS bits.
        //CLKPR=(1<<CLKPCE); // change enable
        //CLKPR=0; // "no pre-scaler"
        _delay_loop_1(0); // 60us

        /*initialize enc28j60*/
        enc28j60Init(mymac);
        //enc28j60clkout(2); // change clkout from 6.25MHz to 12.5MHz
        _delay_loop_1(0); // 60us
        
        /* Magjack leds configuration, see enc28j60 datasheet, page 11 */
        // LEDB=yellow LEDA=green
        //
        // 0x476 is PHLCON LEDA=links status, LEDB=receive/transmit
        // enc28j60PhyWrite(PHLCON,0b0000 0100 0111 01 10);
        enc28j60PhyWrite(PHLCON,0x476);
        
        // PD6 the the push button:
        DDRD&= ~(1<<PIND6);
        PORTD|=1<<PIND6; // internal pullup resistor on

        DDRB|= (1<<DDB1); // enable PB1, LED as output
        LEDOFF;
        // the transistor on PD7:
        DDRD|= (1<<DDD7);
        PORTD &= ~(1<<PORTD7);// transistor off

        //init the web server ethernet/ip layer:
        init_ip_arp_udp_tcp(mymac,myip,MYWWWPORT);

        while(1){

                // handle ping and wait for a tcp packet

                dat_p=packetloop_icmp_tcp(buf,enc28j60PacketReceive(BUFFER_SIZE, buf));

                if(dat_p==0){
                        // check if udp otherwise continue
                        continue;
                }
                if (strncmp("GET ",(char *)&(buf[dat_p]),4)!=0){
                        // head, post and other methods:
                        //
                        // for possible status codes see:
                        // http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
                        plen=http200ok();
                        plen=fill_tcp_data_p(buf,plen,PSTR("<h1>200 OK</h1>"));
                        goto SENDTCP;
                }
                if (strncmp("/ ",(char *)&(buf[dat_p+4]),2)==0){
                        plen=http200ok();
                        plen=fill_tcp_data_p(buf,plen,PSTR("<p>Usage: http://host_or_ip/password</p>\n"));
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
                        LEDON;
                        PORTD|= (1<<PORTD7);// transistor on
                }
                if (cmd==0){
                        LEDOFF;
                        PORTD &= ~(1<<PORTD7);// transistor off
                }
                if (cmd==2){
                        // favicon:
                        plen=moved_perm(buf,0);
                        goto SENDTCP;
                }
                if (cmd==-2){
                        // redirect to the right base url (e.g add a trailing slash):
                        plen=moved_perm(buf,1);
                        goto SENDTCP;
                }
                // if (cmd==-2) or any other value
                // just display the status:
                plen=print_webpage(buf,(PORTD & (1<<PORTD7)));
                //
SENDTCP:
                www_server_reply(buf,plen); // send data
        }
        return (0);
}


                // tcp port www end
                // -------------------------------
