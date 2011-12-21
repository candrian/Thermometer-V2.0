/*
 Thermometer V2.0 
 (c) Created by Charalampos Andrianakis on 18/12/11.
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include <avr/io.h>
#include "uart.h"

void uart_init() {
    
    UBRRH = UBRR_9600>>8;                           // set baudrate counter
    UBRRL = UBRR_9600;
    
    UCSRB = _BV(RXEN)   | _BV(TXEN);                //Receiver Transmitter Enable
    UCSRC = _BV(URSEL)  | _BV(UCSZ0) | _BV(UCSZ1);  //Data 8-bit / 1-stop bit / Parity Disabled / Asynchronous mode
    
    DDRD |= _BV(PD1);   //TX Output
    DDRD &= ~_BV(PD0);  //RX Input
    
}

int uart_putchar(char c){
    loop_until_bit_is_set(UCSRA, UDRE);
    UDR = c;
    return 0;
}

void uart_puts(const char* str){
    while(*str)
        uart_putc(*str++);
}

void uart_putw_dec(uint16_t w){
    uint16_t num = 10000;
    uint8_t started = 0;
    
    while(num > 0){
        uint8_t b = w / num;
        if(b > 0 || started || num == 1){
            uart_putc('0' + b);
            started = 1;
        }
        w -= b * num;
        
        num /= 10;
    }
}
