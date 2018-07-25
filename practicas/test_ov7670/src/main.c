/*==================[inlcusiones]============================================*/

#include "sapi.h"
#include "ov7670_regs.h"
#include "ov7670.h"

/*==================[definiciones y macros]==================================*/

#define UART_PC        UART_USB

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
   // ---------- CONFIGURACIONES ------------------------------
   // Inicializar y configurar la plataforma
	uint32_t xClkFrequency = 24000000;

   boardConfig();
   printf("Inicializando XCLK\r\n");
   OV7670initXClk(xClkFrequency);
   printf("XCLK Inicializado\r\n");
   printf("XCLK Configurado con un Duty Cycle del %d y una frecuencia de %ul\r\n",OV7670getXClkDutyCycle(),xClkFrequency);
   i2cInit(I2C0,100000);
   uint8_t reg_value;
   while (1){
	   OV7670readRegDebug(REG_PID,&reg_value);
	   printf("OV7670-I2C-PID REG=0X%X 0d%d\r\n\n",reg_value,reg_value);
	   reg_value = 0xFF;
	   delay(1000);
   }

}
