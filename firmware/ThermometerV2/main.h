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

//Backlight PWM Values
#define Backlight   OCR1A   //PWM value register
#define ON          255     //Value to turn on PWM
#define OFF         0       //Value to turn off PWM
#define FADE_RATE   15      //Fade rate in miliseconds

//LM35 Pins
#define LM35        PC1
#define LM35_ADC    MUX0

//PWM Pins
#define PWM         PB1
#define BUTTON      PD2

//ASCII
#define DEGREE      223     //Degree Ascii code for LCD

//Display Modes:
#define Celsius     0
#define Farenhait   1

uint8_t mode=0;

//Initialize ADC
static void adc_init();
//Initialize PWM
static void pwm_init();
//Initialize Buttons I/O
static void button_init();
//Display Data
static void display_data();
//Turn backlight on by fadding
void backlight_on();
//Turn backlight off by fadding
void backlight_off();
//Toggle backlight indicating mode change
void backlight_toggle(); 
//Read ADC
static uint16_t adc_read();
//Convert ADC to Voltage
static uint32_t adc_to_volt();
//Convert int to float Char to display to LCD
static char int_to_float_char(uint16_t value);
//Convert int to Char to display to LCD
static char int_to_char(uint16_t value);
//Convert Celsius to Fahrenheit
static uint16_t to_fahrenheit(uint16_t temperature);
//Calculate the sense themperature
uint16_t sense_temp(uint8_t T, uint8_t H);
//Welcome Message
static void welcome_msg();