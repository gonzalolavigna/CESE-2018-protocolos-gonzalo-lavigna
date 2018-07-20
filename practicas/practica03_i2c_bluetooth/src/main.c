/* Copyright 2018, Eric Pernia
 * All rights reserved.
 *
 * This file is part of sAPI Library.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
 
/*==================[inlcusiones]============================================*/

#include "sapi.h"        // <= Biblioteca sAPI
#include <string.h>

/*==================[definiciones y macros]==================================*/

#define UART_PC        UART_USB
#define UART_BLUETOOTH UART_232

typedef struct {
	uint8_t acce_string[100];
	uint8_t gyro_string[100];
	uint8_t magn_string[100];
	uint8_t temp_string[100];
	bool_t acce_new_string;
	bool_t gyro_new_string;
	bool_t magn_new_string;
	bool_t temp_new_string;

} string_mpu_9250_t;

/*==================[definiciones de datos internos]=========================*/
MPU9250_address_t addr = MPU9250_ADDRESS_0;
string_mpu_9250_t mpu_9250_data;

/*==================[definiciones de datos externos]=========================*/

/*==================[declaraciones de funciones internas]====================*/
void init_mpu_9250_data (void);

/*==================[declaraciones de funciones externas]====================*/

bool_t hm10bleTest( int32_t uart );
void hm10blePrintATCommands( int32_t uart );

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
   // ---------- CONFIGURACIONES ------------------------------

   // Inicializar y configurar la plataforma
   boardConfig();

   // Inicializar UART_USB para conectar a la PC
   uartConfig( UART_PC, 115200 );
   uartWriteString( UART_PC, "UART_PC configurada.\r\n" );

   // Inicializar UART_232 para conectar al modulo bluetooth
   uartConfig( UART_BLUETOOTH, 9600 );
   uartWriteString( UART_PC, "UART_BLUETOOTH para modulo Bluetooth configurada.\r\n" );
   
   uint8_t data = 0;
   
   uartWriteString( UART_PC, "Testeto si el modulo esta conectado enviando: AT\r\n" );
   if( hm10bleTest( UART_BLUETOOTH ) ){
      uartWriteString( UART_PC, "Modulo conectado correctamente.\r\n" );
   }  
   printf("Inicializando IMU MPU9250...\r\n" );
   int8_t status;
   status = mpu9250Init( addr );

   if( status < 0 ){
      printf( "IMU MPU9250 no inicializado, chequee las conexiones:\r\n\r\n" );
      printf( "MPU9250 ---- EDU-CIAA-NXP\r\n\r\n" );
      printf( "    VCC ---- 3.3V\r\n" );
      printf( "    GND ---- GND\r\n" );
      printf( "    SCL ---- SCL\r\n" );
      printf( "    SDA ---- SDA\r\n" );
      printf( "    AD0 ---- GND\r\n\r\n" );
      printf( "Se detiene el programa.\r\n" );
      while(1);
   }   
   init_mpu_9250_data();
   printf("IMU MPU9250 inicializado correctamente.\r\n\r\n" );
   
   delay_t delay_1_seg;
   delayInit(&delay_1_seg,1000);
   delayRead(&delay_1_seg);

   gpioInit(GPIO0,GPIO_OUTPUT);
   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE ) {

      // Si leo un dato de una UART lo envio a al otra (bridge)
      if( uartReadByte( UART_PC, &data ) ) {
         uartWriteByte( UART_BLUETOOTH, data );
      }     
      // Si presiono TEC1 imprime la lista de comandos AT
      if( !gpioRead( TEC1 ) ) {
         hm10blePrintATCommands( UART_BLUETOOTH );
      }

      if( uartReadByte( UART_BLUETOOTH, &data ) ) {
         if( data == 'h' ) {
            gpioWrite( LEDB, ON );
         }
         if( data == 'l' ) {
            gpioWrite( LEDB, OFF );
         }
         uartWriteByte( UART_PC, data );
      }


      gpioWrite(GPIO0,ON);
      if(mpu_9250_data.acce_new_string == TRUE){
         printf("%s",mpu_9250_data.acce_string);
         uartWriteString(UART_BLUETOOTH,mpu_9250_data.acce_string);
         mpu_9250_data.acce_new_string = FALSE;
      }
      if(mpu_9250_data.gyro_new_string == TRUE){
         printf("%s",mpu_9250_data.gyro_string);
         uartWriteString(UART_BLUETOOTH,mpu_9250_data.gyro_string);
         mpu_9250_data.gyro_new_string = FALSE;
      }
      if(mpu_9250_data.magn_new_string == TRUE){
         printf("%s",mpu_9250_data.magn_string);
         uartWriteString(UART_BLUETOOTH,mpu_9250_data.magn_string);
         mpu_9250_data.magn_new_string = FALSE;
      }
      if(mpu_9250_data.temp_new_string == TRUE){
         printf("%s",mpu_9250_data.temp_string);
         uartWriteString(UART_BLUETOOTH,mpu_9250_data.temp_string);
         mpu_9250_data.temp_new_string = FALSE;
      }
      gpioWrite(GPIO0,OFF);

      if(delayRead(&delay_1_seg)==1){

    	  mpu9250Read();
         // Imprimir resultados
         /*
         printf( "Giroscopo:      (%f, %f, %f)   [rad/s]\r\n",
                   mpu9250GetGyroX_rads(),
                   mpu9250GetGyroY_rads(),
                   mpu9250GetGyroZ_rads()
                 );
         */

         sprintf(mpu_9250_data.acce_string,"Giroscopo:      (%f, %f, %f)   [rad/s]\r\n",
        		 mpu9250GetGyroX_rads(),
				 mpu9250GetGyroY_rads(),
				 mpu9250GetGyroZ_rads()
				 );         
         mpu_9250_data.acce_new_string = TRUE;

         sprintf(mpu_9250_data.gyro_string, "Acelerometro:   (%f, %f, %f)   [m/s2]\r\n",
                   mpu9250GetAccelX_mss(),
                   mpu9250GetAccelY_mss(),
                   mpu9250GetAccelZ_mss()
                 );
         mpu_9250_data.gyro_new_string = TRUE;

         sprintf(mpu_9250_data.magn_string, "Magnetometro:   (%f, %f, %f)   [uT]\r\n",
                   mpu9250GetMagX_uT(),
                   mpu9250GetMagY_uT(),
                   mpu9250GetMagZ_uT()
                 );
         mpu_9250_data.magn_new_string = TRUE;

         sprintf(mpu_9250_data.temp_string, "Temperatura:    %f   [C]\r\n\r\n",
                   mpu9250GetTemperature_C()
                 );
         mpu_9250_data.temp_new_string = TRUE;

         delayRead(&delay_1_seg);

      }

   }

   // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
   // directamenteno sobre un microcontroladore y no es llamado por ningun
   // Sistema Operativo, como en el caso de un programa para PC.
   return 0;
}

/*==================[definiciones de funciones internas]=====================*/

/*==================[definiciones de funciones externas]=====================*/

bool_t hm10bleTest( int32_t uart )
{
   uartWriteString( uart, "AT\r\n" );
   return waitForReceiveStringOrTimeoutBlocking( uart, 
                                                 "OK\r\n", strlen("OK\r\n"),
                                                 50 );
}

void hm10blePrintATCommands( int32_t uart )
{
   delay(500);
   uartWriteString( uart, "AT+HELP\r\n" );
}


void init_mpu_9250_data (void){
	mpu_9250_data.acce_new_string = FALSE;
	mpu_9250_data.gyro_new_string = FALSE;
	mpu_9250_data.magn_new_string = FALSE;
	mpu_9250_data.temp_new_string = FALSE;
	bzero(mpu_9250_data.acce_string,100);
	bzero(mpu_9250_data.gyro_string,100);
	bzero(mpu_9250_data.magn_string,100);
	bzero(mpu_9250_data.temp_string,100);
}
/*==================[fin del archivo]========================================*/
