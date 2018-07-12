/* Copyright 2016, Eric Pernia.
 * All rights reserved.
 *
 * This file is part sAPI library for microcontrollers.
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
 *
 */

/*
 * Date: 2016-04-26
 */

/*==================[inclusions]=============================================*/

#include "practica01.h"   // <= own header (optional)
#include "sapi.h"        // <= sAPI header
#include "string.h"

#include "ff.h"       // <= Biblioteca FAT FS

/*==================[macros and definitions]=================================*/
#define AD_CHANNELS 3
#define DECIMAL 10
#define RTC_BUFFER_LENGTH 128
#define UART_MESSAGE_LENGTH 128

#define FILENAME "log.txt"

typedef struct  {
	uint16_t muestra_CH1;
	uint16_t muestra_CH2;
	uint16_t muestra_CH3;
	uint8_t  message_to_display[128];
} muestrasAd_t;



/*==================[internal data declaration]==============================*/
static FATFS fs;           // <-- FatFs work area needed for each volume
static FIL fp;             // <-- File object needed for each open file

/*==================[internal functions declaration]=========================*/
void getAdMuestras (muestrasAd_t * structAdMuestras);
void rtcToString (rtc_t * rtc , uint8_t * string_output, uint8_t string_length);
/*==================[internal data definition]===============================*/


/*==================[external data definition]===============================*/
   /* Buffer */
   static char uartBuff[10];
   static uint8_t rtcBuff[RTC_BUFFER_LENGTH];
   static uint8_t messageToDisplay[UART_MESSAGE_LENGTH];
/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
   // FUNCION que se ejecuta cada vezque ocurre un Tick
   void diskTickHook( void *ptr );


/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.

 */
char* itoa(int value, char* result, int base) {
   // check that the base if valid
   if (base < 2 || base > 36) { *result = '\0'; return result; }

   char* ptr = result, *ptr1 = result, tmp_char;
   int tmp_value;

   do {
      tmp_value = value;
      value /= base;
      *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
   } while ( value );

   // Apply negative sign
   if (tmp_value < 0) *ptr++ = '-';
   *ptr-- = '\0';
   while(ptr1 < ptr) {
      tmp_char = *ptr;
      *ptr--= *ptr1;
      *ptr1++ = tmp_char;
   }
   return result;
}


/* Enviar fecha y hora en formato "DD/MM/YYYY, HH:MM:SS" */
void showDateAndTime( rtc_t * rtc ){
   /* Conversion de entero a ascii con base decimal */
   itoa( (int) (rtc->mday), (char*)uartBuff, 10 ); /* 10 significa decimal */
   /* Envio el dia */
   if( (rtc->mday)<10 )
      uartWriteByte( UART_USB, '0' );
   uartWriteString( UART_USB, uartBuff );
   uartWriteByte( UART_USB, '/' );

   /* Conversion de entero a ascii con base decimal */
   itoa( (int) (rtc->month), (char*)uartBuff, 10 ); /* 10 significa decimal */
   /* Envio el mes */
   if( (rtc->month)<10 )
      uartWriteByte( UART_USB, '0' );
   uartWriteString( UART_USB, uartBuff );
   uartWriteByte( UART_USB, '/' );

   /* Conversion de entero a ascii con base decimal */
   itoa( (int) (rtc->year), (char*)uartBuff, 10 ); /* 10 significa decimal */
   /* Envio el año */
   if( (rtc->year)<10 )
      uartWriteByte( UART_USB, '0' );
   uartWriteString( UART_USB, uartBuff );


   uartWriteString( UART_USB, ", ");


   /* Conversion de entero a ascii con base decimal */
   itoa( (int) (rtc->hour), (char*)uartBuff, 10 ); /* 10 significa decimal */
   /* Envio la hora */
   if( (rtc->hour)<10 )
      uartWriteByte( UART_USB, '0' );
   uartWriteString( UART_USB, uartBuff );
   uartWriteByte( UART_USB, ':' );

   /* Conversion de entero a ascii con base decimal */
   itoa( (int) (rtc->min), (char*)uartBuff, 10 ); /* 10 significa decimal */
   /* Envio los minutos */
  // uartBuff[2] = 0;    /* NULL */
   if( (rtc->min)<10 )
      uartWriteByte( UART_USB, '0' );
   uartWriteString( UART_USB, uartBuff );
   uartWriteByte( UART_USB, ':' );

   /* Conversion de entero a ascii con base decimal */
   itoa( (int) (rtc->sec), (char*)uartBuff, 10 ); /* 10 significa decimal */
   /* Envio los segundos */
   if( (rtc->sec)<10 )
      uartWriteByte( UART_USB, '0' );
   uartWriteString( UART_USB, uartBuff );


   /* Envio un 'enter' */
   uartWriteString( UART_USB, "\r\n");
}

/* FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE RESET. */
int main(void){

   /* ------------- INICIALIZACIONES ------------- */

   /* Inicializar la placa */
   boardConfig();

   spiConfig( SPI0 );

   // Inicializar el conteo de Ticks con resolucion de 10ms,
   // con tickHook diskTickHook
   tickConfig( 10 );
   tickCallbackSet( diskTickHook, NULL );

   /* Estructura RTC */
   rtc_t rtc;

   rtc.year = 2018;
   rtc.month = 7;
   rtc.mday = 4;
   rtc.wday = 1;
   rtc.hour = 18;
   rtc.min = 36;
   rtc.sec= 0;

   bool_t val = 0;
   uint8_t i = 0;

   /* Inicializar RTC */
   val = rtcConfig( &rtc );

   delay(2000); // El RTC tarda en setear la hora, por eso el delay

   /* Inicializar UART_USB a 115200 baudios */
   uartConfig( UART_USB, 115200 );

   adcConfig( ADC_ENABLE ); /* ADC */

   muestrasAd_t muestrasAD;

   /* Variables de delays no bloqueantes */
   delay_t delay1;
   /* Inicializar Retardo no bloqueante con tiempo en ms */
   delayConfig( &delay1, 1000 );

   UINT nbytes;

   if( f_mount( &fs, "", 0 ) != FR_OK ){
      // If this fails, it means that the function could
      // not register a file system object.
      // Check whether the SD card is correctly connected
   }

   /* ------------- REPETIR POR SIEMPRE ------------- */
   while(1) {

      /* delayRead retorna TRUE cuando se cumple el tiempo de retardo */
      if ( delayRead( &delay1 ) ){

    	 memset(messageToDisplay,0,UART_MESSAGE_LENGTH);
         /* Leer fecha y hora */
         val = rtcRead( &rtc );
         /* Mostrar fecha y hora en formato "DD/MM/YYYY, HH:MM:SS" */
         getAdMuestras(&muestrasAD);

         rtcToString(&rtc,rtcBuff,RTC_BUFFER_LENGTH);
         strncpy(messageToDisplay,muestrasAD.message_to_display,strlen(muestrasAD.message_to_display));
         strncat(messageToDisplay,rtcBuff,strlen(rtcBuff));
         uartWriteString( UART_USB, messageToDisplay );

         if( f_open( &fp, FILENAME, FA_WRITE | FA_OPEN_APPEND ) == FR_OK ){
            f_write( &fp, messageToDisplay, strlen(messageToDisplay), &nbytes );

            f_close(&fp);

            if( nbytes == strlen(messageToDisplay) ){
               // Turn ON LEDG if the write operation was successful
               gpioWrite( LEDG, ON );
            }
         } else{
            // Turn ON LEDR if the write operation was fail
            gpioWrite( LEDR, ON );
         }

      }

   }

   /* NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa no es llamado
      por ningun S.O. */
   return 0 ;
}

void getAdMuestras (muestrasAd_t * structAdMuestras){
	uint8_t string_buffer[10];

	memset(structAdMuestras->message_to_display,0,128);
	structAdMuestras->muestra_CH1 = adcRead(CH1);
	structAdMuestras->muestra_CH2 = adcRead(CH2);
	structAdMuestras->muestra_CH3 = adcRead(CH3);

	itoa(structAdMuestras->muestra_CH1,string_buffer,DECIMAL);
	strncpy(structAdMuestras->message_to_display,string_buffer,strlen(string_buffer));
	strncat(structAdMuestras->message_to_display,";",1);

	itoa(structAdMuestras->muestra_CH2,string_buffer,DECIMAL);
	strncat(structAdMuestras->message_to_display,string_buffer,strlen(string_buffer));
	strncat(structAdMuestras->message_to_display,";",1);

	itoa(structAdMuestras->muestra_CH3,string_buffer,DECIMAL);
	strncat(structAdMuestras->message_to_display,string_buffer,strlen(string_buffer));
	strncat(structAdMuestras->message_to_display,";",1);

}


void rtcToString (rtc_t * rtc , uint8_t * string_output, uint8_t string_length){
   uint8_t string_buffer[10];

   memset(string_output,0,string_length);

   itoa( (int) (rtc->year), (char*)string_buffer, DECIMAL ); /* 10 significa decimal */
   /* Envio el año */
   if ((rtc->year) < 1000)
	   strncat(string_output,"0",1);
   else if ((rtc->year) < 100)
	   strncat(string_output,"0",1);
   else if ((rtc->year) < 10)
	   strncat(string_output,"0",1);

   strncat(string_output,string_buffer,strlen(string_buffer));
   strncat(string_output,"/",1);

   itoa( (int) (rtc->month), (char*)string_buffer, DECIMAL ); /* 10 significa decimal */
   /* Envio el mes */
   if( (rtc->month)<10)
      strncat(string_output,"0",1);

   strncat(string_output,string_buffer,strlen(string_buffer));
   strncat(string_output,"/",1);

   itoa( (int) (rtc->mday), (char*)string_buffer, DECIMAL ); /* 10 significa decimal */
   /* Envio el dia */
   if( (rtc->mday)<10)
      strncat(string_output,"0",1);

   strncat(string_output,string_buffer,strlen(string_buffer));
   strncat(string_output,"_",1);


   itoa( (int) (rtc->hour), (char*)string_buffer, DECIMAL ); /* 10 significa decimal */
   /* Envio el hora */
   if( (rtc->hour)<10)
      strncat(string_output,"0",1);

   strncat(string_output,string_buffer,strlen(string_buffer));
   strncat(string_output,":",1);

   itoa( (int) (rtc->min), (char*)string_buffer, DECIMAL ); /* 10 significa decimal */
   /* Envio el minuto */
   if( (rtc->min)<10)
      strncat(string_output,"0",1);

   strncat(string_output,string_buffer,strlen(string_buffer));
   strncat(string_output,":",1);


   itoa( (int) (rtc->sec), (char*)string_buffer, DECIMAL ); /* 10 significa decimal */
   /* Envio el segundo */
   if( (rtc->sec)<10)
      strncat(string_output,"0",1);

   strncat(string_output,string_buffer,strlen(string_buffer));
   strncat(string_output,";\r\n",3);

}

// FUNCION que se ejecuta cada vezque ocurre un Tick
void diskTickHook( void *ptr ){
   disk_timerproc();   // Disk timer process
}

/*==================[end of file]============================================*/
