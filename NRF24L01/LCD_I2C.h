/*
 * LCD_I2C.h
 *
 * Created: 16/7/2023 16:00:37
 *  Author: Santiago
 */ 

#pragma once

#include "defines.h"
#include "I2C.h"

#ifndef F_CPU
#define F_CPU 16000000
#endif

/*-------------------------------------------------------------------------*/
#ifndef LCD_ADDRESS
#define LCD_ADDRESS 0x27
#endif
/*-------------------------------------------------------------------------*/

//{D7,D6,D5,D4,X,E,RW,RS}
#define D7_bit_pos 7
#define D6_bit_pos 6
#define D5_bit_pos 5
#define D4_bit_pos 4
#define BL_bit_pos 3
#define E_bit_pos  2
#define RW_bit_pos 1
#define RS_bit_pos 0

#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>
#include <stdbool.h>

enum {
	LCD_CMD,
	LCD_DATA
	};
	
enum {
	OFF,
	ON
	};

class LCD{
	
	private:
	
	uint8_t address_;
	uint8_t error_;
	uint8_t back_light;
	
	
	public:
	
	LCD(void);
	
	LCD(uint8_t);
	
	void BL_on_off(uint8_t);
	//			 cmd
	void lcd_cmd(uint8_t,uint8_t,uint8_t);
	
	//void lcd_cmd_2(uint8_t,uint8_t,uint8_t);
	
	//
	void lcd_Clear(void);
	
	//					 fila	columna
	void lcd_Set_Cursor(uint8_t,uint8_t);
	
	//
	void lcd_home(void);
	
	//
	void lcd_backlight_ON(void);
	
	//
	void lcd_Init(void);
	
	//			  caracter
	void lcd_write_char(char);
	
	//						cadena
	void lcd_write_string(char *);
	
	//void write_string(char *);
	
	//
	void lcd_shift_right(void);
	
	//
	void lcd_shift_left(void);
	
	uint8_t get_error(void);
	
};

typedef class LCD display;