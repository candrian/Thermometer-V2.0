/*
 DHT-11 Library
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
#include "DHT.h"

uint8_t DHT_Read_Data(uint8_t select){

    //data[5] is 8byte table where data come from DHT are stored
    //laststate holds laststate value
    //counter is used to count microSeconds
    uint8_t data[5], laststate = 0, counter = 0, j = 0, i = 0;
    
    //Clear array
    data[0] = data[1] = data[2] = data[3] = data[4] = 0;
    
    cli();                              //Disable interrupts
    
    //Prepare the bus
    DDR(DHT_PORT)   |= _BV(DHT_PIN);    //Set pin Output
    DHT_PORT        |= _BV(DHT_PIN);    //Pin High
    _delay_ms(250);                     //Wait for 250mS
    
    //Send Request Signal
    DHT_PORT        &=~_BV(DHT_PIN);    //Pin Low
    _delay_ms(20);                      //20ms Low 
    
    DHT_PORT        |= _BV(DHT_PIN);    //Pin High
    _delay_us(40);                      //40us High
    
    //Set pin Input to read Bus
    DDR(DHT_PORT)   &=~_BV(DHT_PIN);    //Set pin Input

    laststate=DHT_Read_Pin;             //Read Pin value
    
    //Repeat for each Transistions
    for (i=0; i<MAXTIMINGS; i++) {
        //While state is the same count microseconds
        while (laststate==DHT_Read_Pin) {
            _delay_us(1);
            counter++;
        }
        
        //laststate==_BV(DHT_PIN) checks if laststate was High
        //ignore the first 2 transitions which are the DHT Response
        if (laststate==_BV(DHT_PIN) && (i > 2)) {
            //Save bits in segments of bytes
            //Shift data[] value 1 position left
            //Example. 01010100 if we shift it left one time it will be
            //10101000
            data[j/8]<<=1;
            if (counter >= 40) {    //If it was high for more than 40uS
                data[j/8]|=1;       //it means it is bit '1' so make a logic
            }                       //OR with the value (save it)
            j++;                    //making an OR by 1 to this value 10101000
        }                           //we will have the resault 10101001
                                    //1 in 8-bit binary is 00000001
        //j/8 changes table record every 8 bits which means a byte has been saved
        //so change to next record. 0/8=0 1/8=0 ... 7/8=0 8/8=1 ... 15/8=1 16/8=2
        laststate=DHT_Read_Pin;     //save current state
        counter=0;                  //reset counter
        
    }
    sei();                          //Enable interrupts
    
    //Check if data received are correct by checking the CheckSum
    if (data[0] + data[1] + data[2] + data[3] == data[4]) {
        if (select==DHT_Temp) {     //Return the value has been choosen
            return(data[2]);
        }else if(select==DHT_RH){
            return(data[0]);
        }
    }else{
        uart_puts("\r\nCheck Sum Error");
    }
 
    
}
