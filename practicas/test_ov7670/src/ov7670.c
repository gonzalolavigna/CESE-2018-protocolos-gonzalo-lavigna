#include "ov7670_regs.h"
#include "ov7670.h"
#include "sapi.h"        // <= Biblioteca sAPI
#include "sapi_datatypes.h"
#include "sapi_peripheral_map.h"
#include "sapi_sct.h"

static uint16_t OV7670i2cHardwareRead( i2cMap_t  i2cNumber,
                               uint8_t  i2cSlaveAddress,
                               uint8_t* dataToReadBuffer,
                               uint16_t dataToReadBufferSize,
                               bool_t   sendWriteStop,
                               uint8_t* receiveDataBuffer,
                               uint16_t receiveDataBufferSize,
                               bool_t   sendReadStop );

static uint16_t OV7670i2cHardwareWrite( i2cMap_t  i2cNumber,
                                uint8_t  i2cSlaveAddress,
                                uint8_t* transmitDataBuffer,
                                uint16_t transmitDataBufferSize,
                                bool_t   sendWriteStop );


static uint16_t OV7670i2cRead( i2cMap_t  i2cNumber,
                uint8_t  i2cSlaveAddress,
                uint8_t* dataToReadBuffer,
                uint16_t dataToReadBufferSize,
                bool_t   sendWriteStop,
                uint8_t* receiveDataBuffer,
                uint16_t receiveDataBufferSize,
                bool_t   sendReadStop );

static uint16_t OV7670i2cWrite( i2cMap_t  i2cNumber,
                 uint8_t  i2cSlaveAddress,
                 uint8_t* transmitDataBuffer,
                 uint16_t transmitDataBufferSize,
                 bool_t   sendWriteStop );



void OV7670initXClk(uint32_t frequency){
//Habilita el clock del pixel a traves del GPIO 02 o sea CTOU6
	Sct_Init(frequency);
   	Sct_EnablePwmFor(CTOUT6);//CT OUT 6
   	Sct_SetDutyCycle(CTOUT6,128);
//Espero 10 segundos a que inicialice la camara
   	delay(10000);
}

uint8_t OV7670getXClkDutyCycle(void){
	return Sct_GetDutyCycle(CTOUT6);	
}

bool_t OV7670readReg(uint8_t reg_address, uint8_t * reg_value){
	if(OV7670i2cRead(I2C0,camAddr,&reg_address,1,TRUE,reg_value,0,TRUE)== 0){
		return FALSE;
	}
	if(OV7670i2cRead(I2C0,camAddr,&reg_address,0,TRUE,reg_value,1,TRUE) == 0){
		return FALSE;
	}
	return TRUE;
}

uint16_t OV7670readRegDebug(uint8_t reg_address, uint8_t * reg_value){
	/*uint16_t retval;
	printf("Leyendo Registro:0x%X / 0d%d\r\n",reg_address,reg_address);
	retval = OV7670i2cRead(I2C0,camAddr,&reg_address,1,TRUE,reg_value,0,TRUE);
	printf("Valor de retorno primera transaccion:0x%X 0d%d\r\n",retval,retval);
	OV7670i2cRead(I2C0,camAddr,&reg_address,0,TRUE,reg_value,1,TRUE);
	printf("Valor de retorno segunda transaccion:0x%X 0d%d\r\n",retval,retval);
	return retval;
	*/
	uint32_t bytes_written = 0;
	uint32_t bytes_readed = 0;
	do {
		bytes_written = Chip_I2C_MasterSend(I2C0,0x21,&reg_address,1);
		if(bytes_written == 1){
			bytes_readed= Chip_I2C_MasterRead(I2C0,0x21,reg_value,1);
		}
		printf("CODE MAS:0x%X 0d%d\r\n",Chip_I2CM_GetCurState(I2C0),Chip_I2CM_GetCurState(I2C0));
	}while(bytes_written != 1 && bytes_readed != 1);
	return 1;
}




uint16_t OV7670i2cRead( i2cMap_t  i2cNumber,
                uint8_t  i2cSlaveAddress,
                uint8_t* dataToReadBuffer,
                uint16_t dataToReadBufferSize,
                bool_t   sendWriteStop,
                uint8_t* receiveDataBuffer,
                uint16_t receiveDataBufferSize,
                bool_t   sendReadStop )
{

   bool_t retVal = FALSE;

   if( i2cNumber != I2C0 ) {
      return FALSE;
   }

   retVal = OV7670i2cHardwareRead( i2cNumber,
                             i2cSlaveAddress,
                             dataToReadBuffer,
                             dataToReadBufferSize,
                             sendWriteStop,
                             receiveDataBuffer,
                             receiveDataBufferSize,
                             sendReadStop );

   return retVal;
}
uint16_t OV7670i2cWrite( i2cMap_t  i2cNumber,
                 uint8_t  i2cSlaveAddress,
                 uint8_t* transmitDataBuffer,
                 uint16_t transmitDataBufferSize,
                 bool_t   sendWriteStop )
{

   bool_t retVal = FALSE;

   if( i2cNumber != I2C0 ) {
      return FALSE;
   }
   retVal = OV7670i2cHardwareWrite( i2cNumber,
                              i2cSlaveAddress,
                              transmitDataBuffer,
                              transmitDataBufferSize,
                              sendWriteStop );
   return retVal;
}
static uint16_t OV7670i2cHardwareRead( i2cMap_t  i2cNumber,
                               uint8_t  i2cSlaveAddress,
                               uint8_t* dataToReadBuffer,
                               uint16_t dataToReadBufferSize,
                               bool_t   sendWriteStop,
                               uint8_t* receiveDataBuffer,
                               uint16_t receiveDataBufferSize,
                               bool_t   sendReadStop )
{

   //TODO: ver i2cData.options si se puede poner la condicion opcional de stop

   I2CM_XFER_T i2cData;

   i2cData.slaveAddr = i2cSlaveAddress;
   i2cData.options   = 0;
   i2cData.status    = 0;
   i2cData.txBuff    = dataToReadBuffer;
   i2cData.txSz      = dataToReadBufferSize;
   i2cData.rxBuff    = receiveDataBuffer;
   i2cData.rxSz      = receiveDataBufferSize;

   if( Chip_I2CM_XferBlocking( LPC_I2C0, &i2cData ) == 0 ) {
      return FALSE;
   }

   return i2cData.status;
}
static uint16_t OV7670i2cHardwareWrite( i2cMap_t  i2cNumber,
                                uint8_t  i2cSlaveAddress,
                                uint8_t* transmitDataBuffer,
                                uint16_t transmitDataBufferSize,
                                bool_t   sendWriteStop )
{

   //TODO: ver i2cData.options si se puede poner la condicion opcional de stop

   I2CM_XFER_T i2cData;

   if( i2cNumber != I2C0 ) {
      return FALSE;
   }

   // Prepare the i2cData register
   i2cData.slaveAddr = i2cSlaveAddress;
   i2cData.options   = 0;
   i2cData.status    = 0;
   i2cData.txBuff    = transmitDataBuffer;
   i2cData.txSz      = transmitDataBufferSize;
   i2cData.rxBuff    = 0;
   i2cData.rxSz      = 0;

   /* Send the i2c data */
   if( Chip_I2CM_XferBlocking( LPC_I2C0, &i2cData ) == 0 ) {
      return FALSE;
   }

   /* *** TEST I2C Response ***

   Chip_I2CM_XferBlocking( LPC_I2C0, &i2cData );

   if( i2cData.status == I2CM_STATUS_OK){
      while(1){
         gpioWrite( LEDB, ON );
         delay(100);
         gpioWrite( LEDB, OFF );
         delay(100);
      }
   }

   *** END - TEST I2C Response *** */

   return i2cData.status;
}
