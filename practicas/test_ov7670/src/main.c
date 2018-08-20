/*==================[inlcusiones]============================================*/

#include "sapi.h"
#include "ov7670IRQ.h"
#include "ov7670.h"


extern volatile uint32_t lines_received;
extern volatile uint32_t pixel_array_count [240];
extern volatile uint8_t line[640];
extern volatile bool_t event_frame_capture ;

/*==================[definiciones y macros]==================================*/

#define UART_PC        UART_USB

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
	uartInit(UART_USB,115200);
   // ---------- CONFIGURACIONES ------------------------------
	// Inicializar y configurar la plataforma
	uint32_t xClkFrequency = 10000000;
	uint8_t reg_value;
	uint8_t pll_divider_value = 0x10;
	uint32_t i;

	boardConfig();
	printf("Inicializando XCLK\r\n");
	OV7670initXClk(xClkFrequency);
	printf("XCLK Inicializado\r\n");
	printf("XCLK Configurado con un Duty Cycle del %d y una frecuencia de %ul\r\n",OV7670getXClkDutyCycle(),xClkFrequency);
	OV7670initI2c();
	printf("INIT OV7670 REGS\r\n");
	OV7670Init();
	printf("CONFIG QVGA OV7670\r\n");
	OV7670ConfigQVGA();
	printf("CONFIG YUV 422\r\n");
	OV7670ConfigYUV422();


	printf("CONFIG PLL PRESACLES con valor 0x%X\r\n",pll_divider_value);
	OV7670CofigPLLDivider(pll_divider_value);
	printf("CONFIG TEST PATTER SHIFTING 1 \r\n");
	OV7670ConfigTestPattern(SHIFTING_1); //Configuro valor del prescaler

	OV7670ReadAllRegs();

	//Only for debug, only for bypassing the IRQs
	gpioInit(T_COL0,GPIO_OUTPUT);
	gpioWrite(T_COL0,OFF);

	gpioInit(T_FIL2,GPIO_OUTPUT);
	gpioWrite(T_FIL2,OFF);

	gpioInit(T_FIL3,GPIO_OUTPUT);
	gpioWrite(T_FIL3,OFF);

	OV7670PixelDataInit();
	IRQInitVSync();
	IRQInitHRef();
	IRQPixelClock();

	while (1){
		//Max Timeout
		if(waitForReceiveStringOrTimeoutBlocking(UART_USB,"s\r\n",3,0xFFFFFFFFFFFFFFFF)== TRUE){
			uartWriteString(UART_USB,"Comando S recibido,esperando a completar un frame\r\n");
			IRQOV7670Enable();
			//Bloqueado hasta que se obtiene un frame
			while(	event_frame_capture == FALSE ){}
			IRQOV7670Disable();
			uartWriteString(UART_USB,"CANTIDAD DE LINEAS RECIBIDAS\r\n");
			printf("%d\r\n",lines_received);
			for(i=0;i<240;i++){
					printf("LINEA:%d CANTIDAD DE PIXELES:%d\r\n",i,pixel_array_count[i]);
			}
			for(i=0;i<640;i++){
					printf("ULTIMA LINEA PIXRL:%d VALOR:%d\r\n",i,line[i]);
			}

			event_frame_capture= FALSE;
		}
   }
}
