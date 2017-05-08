#include <stdlib.h>
#include <math.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#include "cpu_speed.h"
#include "lcd.h"
#include "graphics.h"
#include "sprite.h"

#define LED0 	PB2
#define LED1 	PB3
#define LEDLCD	PC7
#define SW1  	PF6
#define SW2  	PF5

#define NUM_BUTTONS 6
#define BTN_DPAD_LEFT 0
#define BTN_DPAD_RIGHT 1
#define BTN_DPAD_UP 2
#define BTN_DPAD_DOWN 3
#define BTN_LEFT 4
#define BTN_RIGHT 5

#define BTN_STATE_UP 0
#define BTN_STATE_DOWN 1

volatile unsigned char btn_hists[NUM_BUTTONS];
volatile unsigned char btn_states[NUM_BUTTONS];

void initialiseHardware();
void initialiseGame();
void debounce();

void process();
void draw();

void process() {
}

void drawBackground() {
	// Border
	draw_line(0, 0, 0, LCD_Y - 1); 					// LEFT BORDER
	draw_line(0, 0, LCD_X - 1, 0); 					// TOP BORDER
	draw_line(0, LCD_Y - 1, LCD_X, LCD_Y - 1); 		// BOTTOM BORDER
	draw_line(LCD_X - 1, 0, LCD_X - 1, LCD_Y - 1); 	// RIGHT BORDER

	// Status Display
	
	// Dummy Variables TODO
	int lives = 10;
	int score = 0;
	int secondsTotal = 0;
	int minutes = secondsTotal / 60;
	int seconds = secondsTotal % 60;

	// buffer to convert ints to strings
	char buff[20];

	sprintf(buff, "%d", lives);
	draw_string(1, 1, buff);
	sprintf(buff, "%d", score);
	draw_string(17, 1, buff);
	sprintf(buff, "%02d:%02d", minutes, seconds);
	draw_string(LCD_X - 28, 1, buff);

	draw_line(0, 8, LCD_X - 1, 8);
}

void draw() {
	drawBackground();
}

int main(void) {
	// Set the CPU speed to 8MHz (you must also be compiling at 8MHz)
	set_clock_speed(CPU_8MHz);

	initialiseHardware();

	initialiseGame();
	
	while(1) {
		clear_screen();
		process();
		draw();
		show_screen();
		_delay_ms(500);
	}

	return 0;
}

void initialiseHardware() {
	// Get our data directions set up as we please (everything is an output unless specified...)
	DDRF &= ~((1 << PF5) | (1 << PF6));
    DDRB &= ~((1 << PB1) | (1 << PB7) | (1 << PB0));
    DDRD &= ~((1 << PD0) | (1 << PD1));

    DDRB |= ((1 << PB2) | (1 << PB3));  

	// Initialise the LCD screen
	lcd_init(LCD_DEFAULT_CONTRAST);
	clear_screen();
	show_screen();

	// Set Timer0 to CTC mode
    TCCR0A |= (1<<WGM01);
    // Prescaling to 256
    TCCR0B |= (1<<CS02);
    // Set inturrupt on compare to match for 250
    TIMSK0 |= (1<<OCIE0A);
    // Set compare to value of 250
    OCR0A = 250;

	sei();
	
}

void initialiseGame() {
	
	draw_string((LCD_X - 12*5) / 2, 0, "Alien Attack");
	draw_string((LCD_X - 14*5) / 2, 8, "Brandon Janson");
	draw_string((LCD_X - 8*5) / 2, 16, "N9494006");
	draw_string((LCD_X - 14*5) / 2, 24, "Press a button");
	draw_string((LCD_X - 14*5) / 2, 32, "to continue...");

	show_screen();

	while(btn_states[BTN_LEFT] == BTN_STATE_UP && btn_states[BTN_RIGHT] == BTN_STATE_UP);
	_delay_ms(100);

	char buff[2];
	for(int i = 3; i > 0; i--) {
		clear_screen();
		sprintf(buff, "%d", i);
		draw_string((LCD_X - 3) / 2, (LCD_Y - 8) / 2, buff);
		show_screen();
		_delay_ms(300);
	}
	
}

ISR(TIMER0_COMPA_vect) {
    for(int i = 0; i < NUM_BUTTONS; i++) {
        btn_hists[i] = btn_hists[i] << 1;
        switch(i) {
            case BTN_DPAD_LEFT:
                btn_hists[i] |= ((PINB >> PB1) & 1);
            break;
            case BTN_DPAD_RIGHT:
                btn_hists[i] |= ((PIND >> PD0) & 1);
            break;
            case BTN_DPAD_UP:
                btn_hists[i] |= ((PIND >> PD1) & 1);
            break;
            case BTN_DPAD_DOWN:
                btn_hists[i] |= ((PINB >> PB7) & 1);
            break;
            case BTN_LEFT:
                btn_hists[i] |= ((PINF >> PF6) & 1);
            break;
            case BTN_RIGHT:
                btn_hists[i] |= ((PINF >> PF5) & 1);
            break;
        }
        if(btn_hists[i] == 0b11111111 && btn_states[i] == BTN_STATE_UP) {
            btn_states[i] = BTN_STATE_DOWN;
        } else if (btn_hists[i] == 0 && btn_states[i] == BTN_STATE_DOWN){
            btn_states[i] = BTN_STATE_UP;
        }
    }
}