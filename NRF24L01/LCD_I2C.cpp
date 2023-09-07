/*
 * LCD_I2C.cpp
 *
 * Created: 16/7/2023 16:00:24
 *  Author: Santiago
 */ 

#include "LCD_I2C.h"

LCD::LCD(void){	
}

LCD::LCD(uint8_t address_){
	
	this->address_ = address_;
	this->back_light = 0;
}

void LCD::BL_on_off(uint8_t back_light){
	
	if(back_light == OFF){
		this->back_light = 0;
	}
	else{
		this->back_light = 1;
	}
	return;
}

void LCD::lcd_cmd(uint8_t cmd_,uint8_t INST_DATA,uint8_t nibbles_){
	
	uint8_t inst_;
	
	if(INST_DATA == LCD_CMD){
		if(nibbles_ == 1){
			inst_ = (0<<RS_bit_pos) | (0<<RW_bit_pos) | (1<<E_bit_pos) | ((this->back_light)<<BL_bit_pos) | ((cmd_ & 0x0F) << D4_bit_pos);
			TWI_Write_LCD(this->address_,&inst_,1);
			_delay_us(500);
			inst_ = (0<<RS_bit_pos) | (0<<RW_bit_pos) | (0<<E_bit_pos) | ((this->back_light)<<BL_bit_pos) | ((cmd_ & 0x0F) << D4_bit_pos);
			TWI_Write_LCD(this->address_,&inst_,1);
			_delay_us(500);
		}
		else
		{	
			//Nibble más significativo
			inst_ = (0<<RS_bit_pos) | (0<<RW_bit_pos) | (1<<E_bit_pos) | ((this->back_light)<<BL_bit_pos) | (((cmd_ & 0xF0)>>4)<<D4_bit_pos);
			TWI_Write_LCD(this->address_,&inst_,1);
			_delay_us(500);
			inst_ = (0<<RS_bit_pos) | (0<<RW_bit_pos) | (0<<E_bit_pos) | ((this->back_light)<<BL_bit_pos) | (((cmd_ & 0xF0)>>4)<<D4_bit_pos);
			TWI_Write_LCD(this->address_,&inst_,1);
			_delay_us(500);
			//Nibble menos significativo
			inst_ = (0<<RS_bit_pos) | (0<<RW_bit_pos) | (1<<E_bit_pos) | ((this->back_light)<<BL_bit_pos) | ((cmd_ & 0x0F) << D4_bit_pos);
			TWI_Write_LCD(this->address_,&inst_,1);
			_delay_us(500);
			inst_ = (0<<RS_bit_pos) | (0<<RW_bit_pos) | (0<<E_bit_pos) | ((this->back_light)<<BL_bit_pos) | ((cmd_ & 0x0F) << D4_bit_pos);
			TWI_Write_LCD(this->address_,&inst_,1);
			_delay_us(500);
		}
	}
	else{
		//Nibble más significativo
		inst_ = (1<<RS_bit_pos) | (0<<RW_bit_pos) | (1<<E_bit_pos) | ((this->back_light)<<BL_bit_pos) | (((cmd_ & 0xF0)>>4)<<D4_bit_pos);
		TWI_Write_LCD(this->address_,&inst_,1);
		_delay_us(500);
		inst_ = (1<<RS_bit_pos) | (0<<RW_bit_pos) | (0<<E_bit_pos) | ((this->back_light)<<BL_bit_pos) | (((cmd_ & 0xF0)>>4)<<D4_bit_pos);
		TWI_Write_LCD(this->address_,&inst_,1);
		_delay_us(500);
		//Nibble menos significativo
		inst_ = (1<<RS_bit_pos) | (0<<RW_bit_pos) | (1<<E_bit_pos) | ((this->back_light)<<BL_bit_pos) | ((cmd_ & 0x0F) << D4_bit_pos);
		TWI_Write_LCD(this->address_,&inst_,1);
		_delay_us(500);
		inst_ = (1<<RS_bit_pos) | (0<<RW_bit_pos) | (0<<E_bit_pos) | ((this->back_light)<<BL_bit_pos) | ((cmd_ & 0x0F) << D4_bit_pos);
		TWI_Write_LCD(this->address_,&inst_,1);
		_delay_us(500);
	}
	return;
}

void LCD::lcd_Init(void){
	
	uint8_t cmd;
	
	cli();
	TWI_Init(100000);
	sei();
	_delay_ms(100);
	
	/////////////////////////////////////////////////////////////////////////////////
	cmd = 0x03 & 0x0F;
	lcd_cmd(cmd,LCD_CMD,1);
	_delay_ms(5);
	/////////////////////////////////////////////////////////////////////////////////
	lcd_cmd(cmd,LCD_CMD,1);
	_delay_us(150);
	/////////////////////////////////////////////////////////////////////////////////
	lcd_cmd(cmd,LCD_CMD,1);
	_delay_us(150);
	/////////////////////////////////////////////////////////////////////////////////
	cmd = 0x02 & 0x0F;
	lcd_cmd(cmd,LCD_CMD,1);
	_delay_us(150);
	/////////////////////////////////////////////////////////////////////////////////
	lcd_cmd(0x2C,LCD_CMD,2);
	lcd_cmd(0x08,LCD_CMD,2);
	lcd_cmd(0x01,LCD_CMD,2);
	lcd_cmd(0x06,LCD_CMD,2);
	lcd_cmd(0x0C,LCD_CMD,2);
	
	return;
	
}

void LCD::lcd_Clear(void){
	
	lcd_cmd(0x01,LCD_CMD,2);
	
	return;
}

void LCD::lcd_write_char(char car_){
	
	lcd_cmd(car_,LCD_DATA,2);
	
	return;
}

void LCD::lcd_write_string(char *cadena){
	
	for(uint8_t i = 0;cadena[i] != '\0';i++){
	
		lcd_write_char(cadena[i]);
		
	}
	
	return;
	
}

void LCD::lcd_Set_Cursor(uint8_t fila,uint8_t columna){
	
	uint8_t inst_;
	
	inst_ = 0x80 | (fila*0x40);
	inst_ += columna;
	lcd_cmd(inst_,LCD_CMD,2);
	
	return;
	
}

void LCD::lcd_home(void){
	
	lcd_Set_Cursor(0,0);
	
	return;
}

