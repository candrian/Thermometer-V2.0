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

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "main.h"

#include "lcd.h"
#include "lcd.c"

#include "DHT.h"
#include "DHT.c"

#include "uart.h"
#include "uart.c"

/* 
Display modes:
 1)Celsius
 2)Farenhait
Backlight modes:
 1)Fast mode (press the button, light on, 2 seconds delay, light off)
 2)Light switched off
 3)Light switched on
*/

uint16_t in_temp=200;
uint8_t out_temp=0, humidity=0;

//Button Interrupt
ISR(INT0_vect){
    
    _delay_ms(500);                 //Wait some miliseconds to have a better look

    if (Backlight!=ON) {            //If backlight is off turn it on
        backlight_on();
    }
    
    _delay_ms(1000);                //Wait for 2 seconds. 
    _delay_ms(1000);                //1)Leave time for the user to see the temperature in fast mode
                                    //2)Delay to see button action

    if (!(PIND & _BV(BUTTON))) {    //If button is still pressed Led on Mode 
        backlight_toggle();
        _delay_ms(1000);            //Wait for the user to remove his finger from the button
        _delay_ms(1000);
        if (!(PIND & _BV(BUTTON))) {//If button is still pressed Display mode switching
            backlight_toggle();
            _delay_ms(1000);        //Wait for the user to remove his finger from the button
            
            if (mode==Farenhait) {          //Switch display mode
                mode=Celsius;
            }else{
                mode=Farenhait;
            }
            
        }
    }else{                          //Else turn the light off
        if (Backlight!=OFF) {
            backlight_off();
        }
    }

}

static void display_data(){
    
    in_temp+=adc_to_volt();
    in_temp/=2;
    out_temp=DHT_Read_Data(DHT_Temp);
    humidity=DHT_Read_Data(DHT_RH);
    
    if (mode==Celsius) {
        //Inside
        lcd_gotoxy(4,0);                    //Send cursor to 4th digit 1st line
        lcd_puts(int_to_float_char(in_temp)); //Print temperature in Celsius
        lcd_gotoxy(9,0);                    //Send cursor to 9th digit 1st line
        lcd_putc('C');                      //Print character C 
        
        //Sense temp
        lcd_gotoxy(11,1);
        lcd_puts(int_to_float_char(sense_temp(out_temp, humidity)));
        
        //Outside
        lcd_gotoxy(4,1);        
        lcd_puts(int_to_char(out_temp));
        lcd_gotoxy(9,1);
        lcd_putc('C');
        
        uart_puts("Inside  :");
        uart_puts(int_to_float_char(in_temp));
        uart_puts("C\r\n");
        uart_puts("Outside :");
        uart_puts(int_to_char(out_temp));
        uart_puts(".0C\r\n");
        uart_puts("Sense   :");
        uart_puts(int_to_float_char(sense_temp(out_temp, humidity)));
        uart_puts(".0C\r\n");
        
    }else{
        //Inside
        lcd_gotoxy(4,0);
        lcd_puts(int_to_char(to_fahrenheit(in_temp/10)));
        lcd_gotoxy(9,0);
        lcd_putc('F');
        
        //Sense temp
        lcd_gotoxy(11,1);
        lcd_puts(int_to_float_char(to_fahrenheit(sense_temp(out_temp, humidity))));
        
        //Outside
        lcd_gotoxy(4,1);
        lcd_puts(int_to_char(to_fahrenheit(out_temp)));
        lcd_gotoxy(9,1);
        lcd_putc('F');
        
        uart_puts("Inside  :");
        uart_puts(int_to_char(to_fahrenheit(in_temp)/10));
        uart_puts(".0F\r\n");
        uart_puts("Outside :");
        uart_puts(int_to_char(to_fahrenheit(out_temp)));
        uart_puts(".0F\r\n");
        uart_puts("Sense   :");
        uart_puts(int_to_float_char(to_fahrenheit(sense_temp(out_temp, humidity))));
        uart_puts(".0F\r\n");
        
    }
    
    lcd_gotoxy(13,0);
    lcd_puts(int_to_char(humidity));
    
    uart_puts("Humidity:");
    uart_puts(int_to_char(humidity));
    uart_puts("%\r\n\r\n");
}

static void adc_init(){
    DDRC    &=~ _BV(LM35);                              //Input for LM35
    ADMUX   =   _BV(REFS0);                             //AVCC with external capacitor at AREF pin
    ADMUX   |=  _BV(LM35_ADC);                          //Select ADC1 channel
    ADCSRA  =   _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);   //Set prescaler to 128 16MHz/128 = 125kHz
    ADCSRA  |=   _BV(ADEN);                              //Enable ADC
}

static void pwm_init(){
    
    TCCR1A  |= _BV(COM1A1);   //Clear OC1A on Compare Match, set OC1A at BOTTOM, (non-inverting mode)
    TCCR1A  |= _BV(WGM11);    //Fast PWM
    TCCR1B  |= _BV(WGM13) | _BV(WGM12);
    TCCR1B  |= _BV(CS11);     //Clk 8 (No prescaling)
    
	ICR1=255;                 //Counter Top Value
	Backlight=OFF;            //Dsiplay Brightness 0=min 255max
    
}

static void button_init(){
    //Configure Switch Button
    GIMSK   =   _BV(INT0);     //Enable external interrupt INT0 for button
    DDRD    &=~ _BV(BUTTON);   //Button pin as input
    PORTD   |=  _BV(BUTTON);   //Button internal pull-up enabled
}

void backlight_on(){
    
    uint8_t i;
    
    DDRB    |= _BV(PWM);        //PWM pin output
    Backlight=OFF;
    for (i=OFF;i<ON;i++){       //Fade in untill fully ON
        Backlight++;
        _delay_ms(FADE_RATE);
    }
    
}

void backlight_off(){
    
    uint8_t i;
    Backlight=ON;
    for (i=ON;i>OFF;i--){       //Fade out untill fully OFF
        Backlight--;
        _delay_ms(FADE_RATE);
    }
    
    DDRB    &=~ _BV(PWM);       //PWM pin input to turn backlight off!
}

void backlight_toggle(){
    
        Backlight=OFF;
        _delay_ms(50);
        Backlight=ON;
        _delay_ms(50);
        Backlight=OFF;
        _delay_ms(50);
        Backlight=ON;
        _delay_ms(100);

}

static uint16_t adc_read(){

    ADCSRA  |=  _BV(ADSC);          //Start single conversion
    while(ADCSRA & _BV(ADSC));      //Wait to be completed

    return (ADC);
}

static uint32_t adc_to_volt(){
    uint32_t result;
    //Voltage calculation V=(adc_value*Vref)/1024. 1024 is the max ADC value
    //The result of the formula is float. To avoid floats we multiply it
    //by 1000. By this we convert mVolts to Volts. So the formula becomes
    //V=((adc_value*Vref)/1024)*1000.    
    //L indicates that the value is long int
    result  = (adc_read() * 5L* 1000L)/1024L;
    return(result-15);  //15 correction value
}

static char int_to_float_char(uint16_t value){
    char buffer[100];
    //By converting Int to float in char conversion we spare time and storage
    if (value/10<10) {
        snprintf(buffer,100,"0%d.%d",value/10,value%10);
    }else{
        snprintf(buffer,100,"%d.%d",value/10,value%10);
    }
    return(buffer);
}

static char int_to_char(uint16_t value){
    char buffer[100];
    if (value<10) {
        snprintf(buffer,100,"0%d",value);
    }else{
        snprintf(buffer,100,"%d",value);
    }
    
    
    return(buffer);
}

static uint16_t to_fahrenheit(uint16_t temperature){

    //Farenhait=(Celsius*1.8) + 32
    //1.8 is a float value in order to avoid it i changed the formula to
    //1.8=9/5 32=160/5 So 
    //Farenhait=((Celsius*(9/5)) + (160/5))=((Celsius*9)+160)/5 
    temperature*=9;         //Normally is 1.8 18/10 takes less ROM space
    temperature+=160;       //Temperature in Fahrenheit
    temperature/=5;         //The division is made last in order not lose digits
    return(temperature);
}

uint16_t sense_temp(uint8_t T, uint8_t H){
    return((600 * T + 4 * T * H + 4000 - 40 * T)/100);
}

static void welcome_msg(){
    
    _delay_ms(1000);
    
    lcd_clrscr();       //Clear display and home cursor
    
    lcd_puts("Thermometer V2.1\n");
    lcd_puts(" Copyright 2011 ");

    backlight_on();

    _delay_ms(1000);
    _delay_ms(1000);
    
    lcd_clrscr();       //Clear display and home cursor
    
    lcd_puts("Andrianakis Har.\n");
    lcd_puts("www.candrian.gr ");
    
    _delay_ms(1000);
    _delay_ms(1000);
    
    lcd_clrscr();       //Clear display and home cursor

    lcd_gotoxy(0,0);    //Display In/Out and degree symbol
    lcd_puts("IN :    ");
    lcd_putc(DEGREE);

    lcd_gotoxy(10,1);
    lcd_puts("(  ");
    lcd_putc(DEGREE);
    lcd_puts(" )");
    
    lcd_gotoxy(0,1);
    lcd_puts("OUT:  .0");
    lcd_putc(DEGREE);
    
    lcd_gotoxy(11,0);
    lcd_puts("H:  %");

}

int main(void){
    uint8_t i=0;
    //Initialization
    button_init();          //Initialize Switch Button
    pwm_init();             //Initialize PWM
    adc_init();             //Initialize ADC
    uart_init();            //Initialize Uart
    lcd_init(LCD_DISP_ON);  //Initialize Display, Cursor off
               
    uart_puts("\r\nModule Powered On\r\n\r\n");
    
    welcome_msg();          //Display Welcome Messege

    sei();                  //Enable all interrupts!
    display_data();         //Display Data once
    while(1) {
        //Read Data every 30 Seconds
        for (i=0; i<=30; i++) {
            _delay_ms(1000);
        }
            display_data();
    }
}
