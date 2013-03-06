/*
 * pages.c
 *
 *  Created on: 26-02-2013
 *      Author: Robert
 */

#include "pages.h"


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

//style.css
uint16_t print_css(void)
{
	uint16_t plen;
	plen = http200okcss();
	plen = fill_tcp_data_p(buf, plen, PSTR("b{color: red;} html{margin-left: auto; margin-right: auto; text-align: center;}"));
	plen = fill_tcp_data_p(buf, plen, PSTR(".slV {-webkit-appearance: slider-vertical;}"));
	return(plen);
}


// prepare the webpage by writing the data to the tcp send buffer
uint16_t print_webpage(uint8_t *buf)
{
        uint16_t plen;
        char ox[200];
        char oy[200];
        plen = http200ok();


        plen=fill_tcp_data_p(buf, plen, PSTR("<link rel=stylesheet type=text/css href=style.css />"));
        plen=fill_tcp_data_p(buf, plen, PSTR("<script src=slider.js></script>"));
        plen=fill_tcp_data_p(buf, plen, PSTR("<b>Kamera sterowana protoko³em TCP/IP</b>"));

        //STEPPER + JS OX
        plen=fill_tcp_data_p(buf, plen, PSTR("<hr><form method=get/>"));
        plen=fill_tcp_data_p(buf, plen, PSTR("<input type=hidden name=sw value=2/>"));
        sprintf(ox, "Obrót w poziomie:<br> <input type=range name=ox min=0 max=100 step=5 value=%d onchange=\"showValue(this.value,'rangeH')\"/><br><span>Pozycja: </span><span id=rangeH>%d</span>", steps_state_h, steps_state_h);
        plen=fill_tcp_data(buf, plen, ox);
        plen=fill_tcp_data_p(buf, plen, PSTR("<br><input type=submit value=Start></form>"));

        //STEPPER + JS OY
        plen=fill_tcp_data_p(buf,plen,PSTR("<hr><form method=get/>"));
        plen=fill_tcp_data_p(buf,plen,PSTR("<input type=hidden name=sw value=2/>"));
        sprintf(oy, "\nObrót w pionie:<br> <input type=range class=\"slV\" name=oy min=\"0\" max=\"100\" step=\"5\" value=%d onchange=\"showValue(this.value,'rangeV')\"/><br><span>Pozycja: </span><span id=rangeV>%d</span>", steps_state_v, steps_state_v);
        plen=fill_tcp_data(buf, plen, oy);
        plen=fill_tcp_data_p(buf,plen,PSTR("<br><input type=submit value=\"Start\"></form>"));
        plen=fill_tcp_data_p(buf, plen, PSTR("<hr><p>Atmega32+enc28j60 Robert Mleczko 2013</p>"));
        return(plen);


/* NIE DZIALA
    	char webpage[850] = "\
<link rel=stylesheet type=text/css href=style.css />\
<script src=slider.js></script>\
<b>Kamera sterowana protoko³em TCP/IP</b>\
<hr><form method=get/>\
<input type=hidden name=sw value=2/>\
%s\
<br><input type=submit value=Start></form>\
<hr><form method=get/>\
<input type=hidden name=sw value=2/>\
%s\
<br><input type=submit value=\"Start\"></form>\
<hr><p>Atmega32+enc28j60 Robert Mleczko 2013</p>\
";

    	sprintf(ox, "Obrót w poziomie:<br> <input type=range name=ox min=0 max=100 step=5 value=%d onchange=\"showValue(this.value,'rangeH')\"/><br><span>Pozycja: </span><span id=rangeH>%d</span>", steps_state_h, steps_state_h);
    	sprintf(oy, "\nObrót w pionie:<br> <input type=range class=\"slV\" name=oy min=\"0\" max=\"100\" step=\"5\" value=%d onchange=\"showValue(this.value,'rangeV')\"/><br><span>Pozycja: </span><span id=rangeV>%d</span>", steps_state_v, steps_state_v);

    	char buffer[850];
    	sprintf(buffer, webpage, ox, oy);

    	plen = fill_tcp_data(buf, plen, webpage);
        return(plen);*/

}
