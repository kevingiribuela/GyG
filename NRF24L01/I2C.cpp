/*
 * I2C.cpp
 *
 * Created: 13/7/2023 00:58:34
 *  Author: Santiago
 */ 

#include "I2C.h"

volatile uint8_t status = TWI_NONE;

ISR(TWI_vect){
	
	status = (TWSR & TWI_NONE);
	
}

uint8_t TWI_Start(void){
	
	uint16_t i = 0;
	
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
	
	while(status != TWI_START){
		i++;	
		if(i >= TWI_TIMEOUT){
			return TWI_ERROR_START;
		}
		
	}
	
	return TWI_OK;
	
}

void TWI_Stop(void){
	
	TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN) | (1<<TWIE);
	
	return;
}


uint8_t TWI_RStart(void){
	
	uint16_t i = 0;
	
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
	
	while(status != TWI_RSTART){
		i++;
		if(i >= TWI_TIMEOUT){
			return TWI_ERROR_RSTART;
		}
		
	}
	
	return TWI_OK;
	
}

uint8_t TWI_addr_write_ack(void){
	
	uint16_t i = 0;
	
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
	
	while(status != TWIT_ADDR_ACK){
		i++;
		if(i >= TWI_TIMEOUT){
			return TWI_ERROR_ADDR_W;
		}
		
	}
	
	return TWI_OK;
	
}

uint8_t TWI_data_write_ack(void){
	
	uint16_t i = 0;
	
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
	
	while(status != TWIT_DATA_ACK){
		i++;
		if(i >= TWI_TIMEOUT){
			return TWI_ERROR_DATA_W;
		}
		
	}
	
	return TWI_OK;
	
}

uint8_t TWI_addr_read_ack(void){
	
	uint16_t i = 0;
	
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
	
	while(status != TWIR_ADDR_ACK){
		i++;
		if(i >= TWI_TIMEOUT){
			return TWI_ERROR_ADDR_R;
		}
		
	}
	
	return TWI_OK;
	
}

uint8_t TWI_data_read_ack(uint8_t ack){
	
	uint16_t i = 0;
	
	if(ack == TWI_ACK){
		
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
		
		while(status != TWIR_DATA_ACK){
			i++;
			if(i >= TWI_TIMEOUT){
				return TWI_ERROR_DATA_R;
			}
			
		}
		
	}
	else{
		
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
		
		while(status != TWIR_DATA_NACK){
			i++;
			if(i >= TWI_TIMEOUT){
				return TWI_ERROR_RSTART;
			}
			
		}
	}
	
	return TWI_OK;
	
}

uint8_t TWI_Read(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len_){
	
	uint16_t i = 0;
	uint8_t err = TWI_OK;
	
	err = TWI_Start();
	if(err != TWI_OK){
		TWI_Stop();
		return err;
	}
	
	TWDR = (addr<<1) | 0;
	
	err = TWI_addr_write_ack();
	if(err != TWI_OK){
		TWI_Stop();
		return err;
	}
	
	TWDR = reg;
	
	err = TWI_data_write_ack();
	if(err != TWI_OK){
		TWI_Stop();
		return err;
	}
	
	err = TWI_RStart();
	if(err != TWI_OK){
		TWI_Stop();
		return err;
	}
	
	TWDR = (addr<<1) | 1;
	
	err = TWI_addr_read_ack();
	if(err != TWI_OK){
		TWI_Stop();
		return err;
	}
	
	for(i=0;i<(len_ - 1);i++){
		
		err = TWI_data_read_ack(TWI_ACK);
		if(err != TWI_OK){
			TWI_Stop();
			return err;
		}
		data[i] = TWDR;
	}
	
	err = TWI_data_read_ack(TWI_NACK);
	if(err != TWI_OK){
		TWI_Stop();
		return err;
	}
	data[i] = TWDR;
	
	TWI_Stop();
	
	return TWI_OK;
}

uint8_t TWI_Write(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len_){
	
	uint16_t i = 0;
	uint8_t err = TWI_OK;
	
	err = TWI_Start();
	if(err != TWI_OK){
		TWI_Stop();
		return err;
	}
	
	TWDR = (addr<<1) | 0;
	
	err = TWI_addr_write_ack();
	if(err != TWI_OK){
		TWI_Stop();
		return err;
	}
	
	TWDR = reg;
	
	err = TWI_data_write_ack();
	if(err != TWI_OK){
		TWI_Stop();
		return err;
	}
	
	for(i=0;i<len_;i++){
		
		TWDR = data[i];
		
		err = TWI_data_write_ack();
		if(err != TWI_OK){
			TWI_Stop();
			return err;
		}
	}
	
	TWI_Stop();
	
	return TWI_OK;
}


void TWI_Init(uint32_t speed){
	
	uint32_t gen_t = 0;
	
	gen_t = (((F_CPU/speed) - 16) / 2) & 0xFF;
	SPCR |= (1<<MSTR);
	TWBR = (uint8_t) gen_t;
	TWCR = (1<<TWEN) | (1<<TWIE);
	PRR0 &= ~(1<<PRTWI);
	TWSR &= ~((1<<TWPS1) | (1<<TWPS0));
	TWI_DDR &= ~((1<<SDA_PIN) | (1<<SCL_PIN));
	TWI_PORT |= (1<<SDA_PIN) | (1<<SCL_PIN);
	
	return;
}


uint8_t TWI_Write_LCD(uint8_t addr, uint8_t *data, uint16_t len_){
	
	
	uint16_t i = 0;
	uint8_t err = TWI_OK;
	
	err = TWI_Start();
	if(err != TWI_OK){
		TWI_Stop();
		return err;
	}
	
	TWDR = (addr<<1) | 0;
	
	err = TWI_addr_write_ack();
	if(err != TWI_OK){
		TWI_Stop();
		return err;
	}
	
	for(i=0;i<len_;i++){
		
		TWDR = data[i];
		
		err = TWI_data_write_ack();
		if(err != TWI_OK){
			TWI_Stop();
			return err;
		}
	}
	
	TWI_Stop();
	
	return err;
	
}


uint8_t TWI_Write_LCD_2(uint8_t addr, uint8_t *data, uint16_t len_){
	
	
	uint16_t i = 0;
	uint8_t err = TWI_OK;
	
	err = TWI_Start();
	if(err != TWI_OK){
		TWI_Stop();
		return err;
	}
	
	TWDR = (addr<<1) | 0;
	
	err = TWI_addr_write_ack();
	if(err != TWI_OK){
		TWI_Stop();
		return err;
	}
	
	for(i=0;i<len_;i++){
		
		TWDR = data[i];
		
		err = TWI_data_write_ack();
		if(err != TWI_OK){
			TWI_Stop();
			return err;
		}
		
		_delay_us(100);
	}
	
	TWI_Stop();
	
	return err;
	
}