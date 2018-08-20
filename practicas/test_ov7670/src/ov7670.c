#include "ov7670IRQ.h"
#include "ov7670.h"
#include "sapi.h"        // <= Biblioteca sAPI
#include "sapi_datatypes.h"
#include "sapi_peripheral_map.h"
#include "sapi_sct.h"

//Info From
//https://github.com/spotify/linux/blob/master/drivers/media/video/ov7670.c
regval_list ov7670_default_regs[] = {
	{ REG_COM7, COM7_RESET },
/*
 * Clock scale: 3 = 15fps
 *              2 = 20fps
 *              1 = 30fps
 */
	{ REG_CLKRC, 0x1 },	/* OV: clock scale (30 fps) */
	{ REG_TSLB,  0x04 },	/* OV */
	{ REG_COM7, 0 },	/* VGA */
	/*
	 * Set the hardware window.  These values from OV don't entirely
	 * make sense - hstop is less than hstart.  But they work...
	 */
	{ REG_HSTART, 0x13 },	{ REG_HSTOP, 0x01 },
	{ REG_HREF, 0xb6 },	{ REG_VSTART, 0x02 },
	{ REG_VSTOP, 0x7a },	{ REG_VREF, 0x0a },

	{ REG_COM3, 0 },	{ REG_COM14, 0 },
	/* Mystery scaling numbers */
	{ 0x70, 0x3a },		{ 0x71, 0x35 },
	{ 0x72, 0x11 },		{ 0x73, 0xf0 },
	{ 0xa2, 0x02 },		{ REG_COM10, 0x0 },

	/* Gamma curve values */
	{ 0x7a, 0x20 },		{ 0x7b, 0x10 },
	{ 0x7c, 0x1e },		{ 0x7d, 0x35 },
	{ 0x7e, 0x5a },		{ 0x7f, 0x69 },
	{ 0x80, 0x76 },		{ 0x81, 0x80 },
	{ 0x82, 0x88 },		{ 0x83, 0x8f },
	{ 0x84, 0x96 },		{ 0x85, 0xa3 },
	{ 0x86, 0xaf },		{ 0x87, 0xc4 },
	{ 0x88, 0xd7 },		{ 0x89, 0xe8 },

	/* AGC and AEC parameters.  Note we start by disabling those features,
	   then turn them only after tweaking the values. */
	{ REG_COM8, COM8_FASTAEC | COM8_AECSTEP | COM8_BFILT },
	{ REG_GAIN, 0 },	{ REG_AECH, 0 },
	{ REG_COM4, 0x40 }, /* magic reserved bit */
	{ REG_COM9, 0x18 }, /* 4x gain + magic rsvd bit */
	{ REG_BD50MAX, 0x05 },	{ REG_BD60MAX, 0x07 },
	{ REG_AEW, 0x95 },	{ REG_AEB, 0x33 },
	{ REG_VPT, 0xe3 },	{ REG_HAECC1, 0x78 },
	{ REG_HAECC2, 0x68 },	{ 0xa1, 0x03 }, /* magic */
	{ REG_HAECC3, 0xd8 },	{ REG_HAECC4, 0xd8 },
	{ REG_HAECC5, 0xf0 },	{ REG_HAECC6, 0x90 },
	{ REG_HAECC7, 0x94 },
	{ REG_COM8, COM8_FASTAEC|COM8_AECSTEP|COM8_BFILT|COM8_AGC|COM8_AEC },

	/* Almost all of these are magic "reserved" values.  */
	{ REG_COM5, 0x61 },	{ REG_COM6, 0x4b },
	{ 0x16, 0x02 },		{ REG_MVFP, 0x07 },
	{ 0x21, 0x02 },		{ 0x22, 0x91 },
	{ 0x29, 0x07 },		{ 0x33, 0x0b },
	{ 0x35, 0x0b },		{ 0x37, 0x1d },
	{ 0x38, 0x71 },		{ 0x39, 0x2a },
	{ REG_COM12, 0x78 },	{ 0x4d, 0x40 },
	{ 0x4e, 0x20 },		{ REG_GFIX, 0 },
	{ 0x6b, 0x4a },		{ 0x74, 0x10 },
	{ 0x8d, 0x4f },		{ 0x8e, 0 },
	{ 0x8f, 0 },		{ 0x90, 0 },
	{ 0x91, 0 },		{ 0x96, 0 },
	{ 0x9a, 0 },		{ 0xb0, 0x84 },
	{ 0xb1, 0x0c },		{ 0xb2, 0x0e },
	{ 0xb3, 0x82 },		{ 0xb8, 0x0a },

	/* More reserved magic, some of which tweaks white balance */
	{ 0x43, 0x0a },		{ 0x44, 0xf0 },
	{ 0x45, 0x34 },		{ 0x46, 0x58 },
	{ 0x47, 0x28 },		{ 0x48, 0x3a },
	{ 0x59, 0x88 },		{ 0x5a, 0x88 },
	{ 0x5b, 0x44 },		{ 0x5c, 0x67 },
	{ 0x5d, 0x49 },		{ 0x5e, 0x0e },
	{ 0x6c, 0x0a },		{ 0x6d, 0x55 },
	{ 0x6e, 0x11 },		{ 0x6f, 0x9f }, /* "9e for advance AWB" */
	{ 0x6a, 0x40 },		{ REG_BLUE, 0x40 },
	{ REG_RED, 0x60 },
	{ REG_COM8, COM8_FASTAEC|COM8_AECSTEP|COM8_BFILT|COM8_AGC|COM8_AEC|COM8_AWB },

	/* Matrix coefficients */
	{ 0x4f, 0x80 },		{ 0x50, 0x80 },
	{ 0x51, 0 },		{ 0x52, 0x22 },
	{ 0x53, 0x5e },		{ 0x54, 0x80 },
	{ 0x58, 0x9e },

	{ REG_COM16, COM16_AWBGAIN },	{ REG_EDGE, 0 },
	{ 0x75, 0x05 },		{ 0x76, 0xe1 },
	{ 0x4c, 0 },		{ 0x77, 0x01 },
	{ REG_COM13, 0xc3 },	{ 0x4b, 0x09 },
	{ 0xc9, 0x60 },		{ REG_COM16, 0x38 },
	{ 0x56, 0x40 },

	{ 0x34, 0x11 },		{ REG_COM11, COM11_EXP|COM11_HZAUTO },
	{ 0xa4, 0x88 },		{ 0x96, 0 },
	{ 0x97, 0x30 },		{ 0x98, 0x20 },
	{ 0x99, 0x30 },		{ 0x9a, 0x84 },
	{ 0x9b, 0x29 },		{ 0x9c, 0x03 },
	{ 0x9d, 0x4c },		{ 0x9e, 0x3f },
	{ 0x78, 0x04 },

	/* Extra-weird stuff.  Some sort of multiplexor register */
	{ 0x79, 0x01 },		{ 0xc8, 0xf0 },
	{ 0x79, 0x0f },		{ 0xc8, 0x00 },
	{ 0x79, 0x10 },		{ 0xc8, 0x7e },
	{ 0x79, 0x0a },		{ 0xc8, 0x80 },
	{ 0x79, 0x0b },		{ 0xc8, 0x01 },
	{ 0x79, 0x0c },		{ 0xc8, 0x0f },
	{ 0x79, 0x0d },		{ 0xc8, 0x20 },
	{ 0x79, 0x09 },		{ 0xc8, 0x80 },
	{ 0x79, 0x02 },		{ 0xc8, 0xc0 },
	{ 0x79, 0x03 },		{ 0xc8, 0x40 },
	{ 0x79, 0x05 },		{ 0xc8, 0x30 },
	{ 0x79, 0x26 },

	{ 0xff, 0xff },	/* END MARKER */
};


//Se habilita el clock hacia la camara que tiene que estar entre 10Mhz y 48Mhz hacia la camara
//El clock sale por el PIN CT OUT 9 o sea el SPIO MOSI de la EDU CIAA
void OV7670initXClk(uint32_t frequency){
	Sct_Init(frequency);
   	Sct_EnablePwmFor(CTOUT9);//CT OUT 9 //SPI MOSI en EDU-CIAA
   	Sct_SetDutyCycle(CTOUT9,128); //Configuro duty cycle al 50%
   	//Espero 1 segundo a que inicialice la camara solo para tener cuidado
   	delay(1000);
}
//Se obtiene el duty cycle configurado
uint8_t OV7670getXClkDutyCycle(void){
	return Sct_GetDutyCycle(CTOUT9);
}
//Se inicializa el I2C a 100Khz se probo a 400 Khz y no funciono.
void OV7670initI2c(void){
	   i2cInit(I2C0,100000);
}

//Funcion para leer los registros de la camara. Funcion bloqueante-
//El valor leido se devuelve por referencia.
//Una lectura hacia la camara esta formada por dos transferencia una de write indicando el registro que se quiere leer
//Otra transferencia de read con el address de la camara. Con lo cual son dos transferencia I2C.
//Tener en mente tambien
//Como pueden haber ciertas transferencias que fallen se hace la transaccion hasta que tanto los dos ciclos finalizaron correctamente.
//Cualquier ciclo que no finalice con OK se repite hasta finalizar.
//TODO: Mejorar valor de retorno por el momento no se considero necesario.


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
//Se recibe el address y el dato y se hace una sola tranaccion para finalizar la escritura.
//Como pueden haber ciertas transferencias que fallen se hace la transaccion hasta que tanto los dos ciclos finalizaron correctamente.
//Cualquier ciclo que no finalice con OK se repite hasta finalizar.
//TODO: Mejorar valor de retorno por el momento no se considero necesario.

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

//Solo para debug se leen todos los registros de la camara.
//TODO:Se deberian agregar a los registros un string asociados donde se tenga una descripcion del mismo.
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


//Funcion para escribir a la camara con un arreglo de estructuras que contienen address y datos.
//Funcion realiza un chequeo en el cual se fija si el dato escrito coincide.

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

void OV7670CofigPLLDivider (uint8_t value){
	OV7670writeReg(REG_CLKRC, value);
}

