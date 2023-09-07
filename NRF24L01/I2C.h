/*
 * I2C.h
 *
 * Created: 13/7/2023 00:58:26
 *  Author: Santiago
 */ 

#pragma once

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//This defines indicates the port and pins of the I2C.
#define TWI_DDR	DDRD
#define TWI_PORT PORTD
#define SDA_PIN PIND1
#define SCL_PIN PIND0

#define TWI_TIMEOUT 1600

#define TWI_ACK			0x00
#define TWI_NACK		0x01

#define TWI_START		0x08
#define TWI_RSTART		0x10
#define TWIT_ADDR_ACK	0x18
#define TWIT_ADDR_NACK	0x20
#define TWIT_DATA_ACK	0x28
#define TWIT_DATA_NACK	0x30

#define TWIR_ADDR_ACK	0x40
#define TWIR_ADDR_NACK	0x48
#define TWIR_DATA_ACK	0x50
#define TWIR_DATA_NACK	0x58

#define TWI_ERROR		0x38
#define TWI_NONE		0xF8

enum{
	TWI_OK,
	TWI_ERROR_START,
	TWI_ERROR_RSTART,
	TWI_ERROR_ADDR_W,
	TWI_ERROR_DATA_W,
	TWI_ERROR_ADDR_R,
	TWI_ERROR_DATA_R
	};
	
void TWI_Init(uint32_t speed);
uint8_t TWI_Write(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len_);
uint8_t TWI_Write_LCD(uint8_t addr, uint8_t *data, uint16_t len_);
uint8_t TWI_Write_LCD_2(uint8_t addr, uint8_t *data, uint16_t len_);
uint8_t TWI_Read(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len_);
uint8_t TWI_data_read_ack(uint8_t ack);
uint8_t TWI_addr_read_ack(void);
uint8_t TWI_data_write_ack(void);
uint8_t TWI_addr_write_ack(void);
uint8_t TWI_RStart(void);
void TWI_Stop(void);
uint8_t TWI_Start(void);


//P7 con D7 -> P4 con D4
//P0 con RS
//P1 con RW
//P2 con E
// Estructura del Byte: {D7,D6,D5,D4,X,E,RW,RS}