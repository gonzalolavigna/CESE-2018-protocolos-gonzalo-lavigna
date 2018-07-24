/*==================[inlcusiones]============================================*/

#include "sapi.h"        // <= Biblioteca sAPI
#include "sapi_sct.h"
#include <string.h>

/*==================[definiciones y macros]==================================*/

#define UART_PC        UART_USB

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
   // ---------- CONFIGURACIONES ------------------------------

   // Inicializar y configurar la plataforma
   boardConfig();
   printf("Inicializando Prueba SCT\r\n");
   Sct_Init(24000000);
   Sct_EnablePwmFor(CTOUT6);//CT OUT 6
   Sct_SetDutyCycle(CTOUT6,128);
   i2cInit(I2C0,100000);

   uint8_t _buffer[21];
   _buffer[0] = 30;
   uint8_t sub_address = 0x0a;

   delay(10000);
   while(1){
	   delay(2000);
	   if(i2cRead(I2C0,0x21,&sub_address,1,TRUE,_buffer,0,TRUE) == 1)
		   printf("OK\r\n");
	   else printf("NOK\r\n");
	   //delay(2000);
	   if(i2cRead(I2C0,0x21,&sub_address,0,TRUE,_buffer,16,TRUE) == 1)
		   printf("OK\r\n");
	   else printf("NOK\r\n");
	   printf("DUTY CYCLE:%d\r\n",Sct_GetDutyCycle(CTOUT6));
	   printf("Registro %d Valor %d %d %d %d\r\n",sub_address,_buffer[0],_buffer[1],_buffer[2],_buffer[3]);
   }

}
