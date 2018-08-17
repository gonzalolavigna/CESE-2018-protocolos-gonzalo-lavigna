#include "ov7670_regs.h"
#include "ov7670.h"
#include "sapi.h"        // <= Biblioteca sAPI
#include "sapi_datatypes.h"
#include "sapi_peripheral_map.h"
#include "sapi_sct.h"


void OV7670initXClk(uint32_t frequency){
//Habilita el clock del pixel a traves del GPIO 02 o sea CTOU6
	Sct_Init(frequency);
   	Sct_EnablePwmFor(CTOUT9);//CT OUT 9 //SPI MOSI en EDU-CIAA
   	Sct_SetDutyCycle(CTOUT9,128);
//Espero 10 segundos a que inicialice la camara
   	delay(1000);
}

uint8_t OV7670getXClkDutyCycle(void){
	return Sct_GetDutyCycle(CTOUT9);
}

void OV7670initI2c(void){
	   i2cInit(I2C0,100000);
}

bool_t OV7670readReg(uint8_t reg_address, uint8_t * reg_value){
	uint16_t i2c_transfer_status = I2CM_STATUS_BUSY ;

	while(i2c_transfer_status != I2CM_STATUS_OK){
		if ((i2c_transfer_status = glavignaI2cRead(I2C0,camAddr,&reg_address,1,TRUE,reg_value,0,TRUE)) != I2CM_STATUS_OK){
			continue;
		}
		if ((i2c_transfer_status = glavignaI2cRead(I2C0,camAddr,&reg_address,0,TRUE,reg_value,1,TRUE)) != I2CM_STATUS_OK){
					continue;
		}
	}
	return TRUE;
}

bool_t OV7670writeReg (uint8_t reg_address, uint8_t  reg_value){
	bool_t flag;
	uint16_t i2c_transfer_status = I2CM_STATUS_BUSY ;
	uint8_t buffer[2];
	buffer[0] = reg_address;
	buffer[1] = reg_value;

	while(i2c_transfer_status != I2CM_STATUS_OK){
		if ((i2c_transfer_status = glavignaI2cWrite(I2C0,camAddr,buffer,2,TRUE) ) != I2CM_STATUS_OK){
			continue;
		}
	}
	return TRUE;
}

//Only for Debug
bool_t OV7670ReadAllRegs(void){
	bool_t flag;
	uint8_t reg_address = 0;
	uint8_t reg_value;
	for( reg_address = 0;reg_address < MAX_REG_ADDRES;reg_address++){
		if ((flag = OV7670readReg(reg_address,&reg_value)) ==FALSE){
			printf("ERROR IN READING OV7670 REGS CHECK CONNECTIONS\r\n");
			break;
		}
		printf("ADDDRES:0x%X VALUE:0x%X\r\n",reg_address,reg_value);
	}
	return flag;
}


bool_t OV7670WriteArray(regval_list *vals){
	bool_t flag;
	uint8_t check_value;
	//SEARCH FOR CONFIGURATION FINISH
	while (vals->reg_address != 0XFF || vals->reg_value != 0xFF){
		OV7670writeReg(vals->reg_address,vals->reg_value);
		OV7670readReg(vals->reg_address,&check_value);
		if(check_value != vals->reg_value){
			printf("REGISTER DOESNT MATCH: 0x%X EXPECTED VALUE 0x%X RETURN VALUE 0x%X\r\n",vals->reg_address,vals->reg_value,check_value);
		}
		vals++;
	}
	return flag = TRUE;
}
//Data from https://hkalasua.wordpress.com/2017/09/11/ov7670-arduino-sd/
void OV7670Init(){
  //Reset All Register Values
	OV7670writeReg(0x12,0x80);
	delay(100);
	OV7670writeReg(0x3A, 0x04); //TSLB

	OV7670writeReg(0x13, 0xC0); //COM8
	OV7670writeReg(0x00, 0x00); //GAIN
	OV7670writeReg(0x10, 0x00); //AECH
	OV7670writeReg(0x0D, 0x40); //COM4
	OV7670writeReg(0x14, 0x18); //COM9
	OV7670writeReg(0x24, 0x95); //AEW
	OV7670writeReg(0x25, 0x33); //AEB
	OV7670writeReg(0x13, 0xC5); //COM8
	OV7670writeReg(0x6A, 0x40); //GGAIN
	OV7670writeReg(0x01, 0x40); //BLUE
	OV7670writeReg(0x02, 0x60); //RED
	OV7670writeReg(0x13, 0xC7); //COM8
	OV7670writeReg(0x41, 0x08); //COM16
	OV7670writeReg(0x15, 0x30); //COM10 - PCLK does not toggle on HBLANK and inver PCLK
	OV7670writeReg(0x09, 0x03); //COM2 -  Output drive capability in x4
  }

void OV7670ConfigQVGA(){
	OV7670writeReg(0x0C, 0x04);//COM3 - Enable Scaling
	OV7670writeReg(0x3E, 0x19);//COM14
	OV7670writeReg(0x72, 0x11);//
	OV7670writeReg(0x73, 0xF1);//
	OV7670writeReg(0x17, 0x16);//HSTART
	OV7670writeReg(0x18, 0x04);//HSTOP
	OV7670writeReg(0x32, 0xA4);//HREF
	OV7670writeReg(0x19, 0x02);//VSTART
	OV7670writeReg(0x1A, 0x7A);//VSTOP
	OV7670writeReg(0x03, 0x0A);//VREF
  }

void OV7670ConfigYUV422(){
	OV7670writeReg(0x12, 0x00);//COM7
	OV7670writeReg(0x8C, 0x00);//RGB444
	OV7670writeReg(0x04, 0x00);//COM1
	OV7670writeReg(0x40, 0xC0);//COM15
	OV7670writeReg(0x14, 0x1A);//COM9
	OV7670writeReg(0x3D, 0x40);//COM13
  }

bool_t OV7670ConfigTestPattern (ov7670_test_pattern_t test_pattern){
	uint8_t reg_value;
	bool_t flag;
	switch (test_pattern){
		case (NO_PATTERN):
			OV7670readReg(SCALING_XSC,&reg_value);
			OV7670writeReg(SCALING_XSC,(reg_value & TEST_PATTERN_MASK) | 0b00000000);
			OV7670readReg(SCALING_YSC,&reg_value);
			OV7670writeReg(SCALING_YSC,(reg_value & TEST_PATTERN_MASK) | 0b00000000);
			flag= TRUE;
			break;
		case (SHIFTING_1):
			OV7670readReg(SCALING_XSC,&reg_value);
			OV7670writeReg(SCALING_XSC,(reg_value & TEST_PATTERN_MASK) | 0b10000000);
			OV7670readReg(SCALING_YSC,&reg_value);
			OV7670writeReg(SCALING_YSC,(reg_value & TEST_PATTERN_MASK) | 0b00000000);
			flag= TRUE;
			break;
		case (BAR_COLOCAR_BAR):
			OV7670readReg(SCALING_XSC,&reg_value);
			OV7670writeReg(SCALING_XSC,(reg_value & TEST_PATTERN_MASK) | 0b00000000);
			OV7670readReg(SCALING_YSC,&reg_value);
			OV7670writeReg(SCALING_YSC,(reg_value & TEST_PATTERN_MASK) | 0b10000000);
			flag= TRUE;
			break;
		case (FADE_TO_GRAY):
			OV7670readReg(SCALING_XSC,&reg_value);
			OV7670writeReg(SCALING_XSC,(reg_value & TEST_PATTERN_MASK) | 0b10000000);
			OV7670readReg(SCALING_YSC,&reg_value);
			OV7670writeReg(SCALING_YSC,(reg_value & TEST_PATTERN_MASK) | 0b10000000);
			flag= TRUE;
			break;
	}
	return flag;
}

