////////////////////////////////////////////////////////////////////////////////
// 
// AVR Current Meter
//
// (c) 2014 Charles Franklin
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// This program reads three analog values, scales them, then prints them to
// an LCD
//
////////////////////////////////////////////////////////////////////////////////


#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#define LCDPort	PORTD	// The port for the LCD control pins
#define LCDrs 	PD6		// The rs pin
#define LCDen	PD5		// The en pin

#define pin_set(port, pin) port |= 1 << (pin);		// Sets pin on port
#define pin_clr(port, pin) port &= ~(1 << (pin));	// Clears pin on port

#define delay_ms _delay_ms

#define WAIT 	500		// Time between measurements
#define SCALE	5		// Maximum input voltage by which to scale

void lcdInit(void);
void lcdSendByte(uint8_t reg, uint8_t byte);
void lcdSendNib(uint8_t nib);
void lcdPut(char c);
void lcdPuts(char *s);
void lcdClear(void);

unsigned int adcRead(int pin);

int main(void) {
	PORTD = 0;		// Clear port
	DDRD  |= 0xE0;	// Set 3 bits to output
	DDRB  |= 0xF0;	// Set upper nibble high
	ADCSRA = (1 << ADEN) | (0x06);	// Enable ADC with prescale 64
	ADMUX |= (1 << REFS0);			// Use internal 1.1V Reference
	DIDR0 = 0x38;					// Disable digital on Ain 5-3
	lcdInit();
	lcdPuts("Hello World!");
	while(1) {
		lcdClear();
		PORTD |= 0x80;	// Turn on LED
		int i;
		for (i = 3; i < 6; i++) {
			// Loop through each pin
			double val = adcRead(i);				// Read value
			char str_val[6];
			dtostrf(val*SCALE/1023, 5, 2, str_val);	// Convert to string
			if (i == 5) {
				lcdSendByte(0, 0xC0);				// Second Row
			}
			lcdPut(i + 0x30);						// Convert to ascii
			lcdPuts(":");
			lcdPuts(str_val);
			lcdPut(' ');
		}
		
		delay_ms(WAIT);
		PORTD &= 0x7F;	// Turn off LED
		delay_ms(WAIT);
	}
	return 0;
}

void lcdInit(void) {
	pin_clr(LCDPort, LCDrs);
	pin_clr(LCDPort, LCDen);
	delay_ms(15);	
	lcdSendNib(0x30);	// Reset 1
	delay_ms(5);
	lcdSendNib(0x30);	// Reset 2
	delay_ms(1);
	lcdSendNib(0x30);	// Reset 3
	delay_ms(5);
	lcdSendNib(0x20);		// 4-bit mode
	lcdSendByte(0, 0x28);	// Two lines
	lcdSendByte(0, 0x0C);	// On / Cursor off / Blink off
	lcdClear();
	delay_ms(1);
}

void lcdSendNib(uint8_t nib) {
	PORTB |= nib & 0xF0;
	PORTB &= nib | 0x0F;
	pin_set(LCDPort, LCDen);
	delay_ms(1);
	pin_clr(LCDPort, LCDen);
	delay_ms(1);
}

void lcdSendByte(uint8_t reg, uint8_t byte) {
	if (reg) {
		pin_set(LCDPort, LCDrs);
	}
	else {
		pin_clr(LCDPort, LCDrs);
	}
	lcdSendNib(byte);
	lcdSendNib(byte << 4);
}

void lcdPut(char c) {
	lcdSendByte(1, c);
}

void lcdPuts(char *s) {
	char *c;
	for (c = (char*)s; *c != '\0'; c++) {
		lcdSendByte(1, *c);
	}
}

void lcdClear(void) {
	lcdSendByte(0, 0x01);
}

unsigned int adcRead(int pin) {
	ADMUX &= 0xF0; 			// Clear input selection
	ADMUX |= pin & 0x0F;	// Select new pin
	ADCSRA |= 1 << ADSC;	// Start conversion
	delay_ms(1);
	return ADC;
}
