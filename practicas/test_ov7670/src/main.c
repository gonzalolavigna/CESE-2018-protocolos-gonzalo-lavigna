/*==================[inlcusiones]============================================*/

#include "sapi.h"
#include "ov7670IRQ.h"
#include "ov7670.h"


//Variable donde se guarda la cantidad de lineas recibidas, con esto nos referimos a sincronismo horizontal
extern volatile uint32_t lines_received;
//Por todas las lineas recibidas se cuentan cuantos pixel hay dentro de cada una
extern volatile uint32_t pixel_array_count [240];
//Se guarda la ultima linea para enviarla por puerto serie y verificar el patron
extern volatile uint8_t line[640];
//Variable para indicar que hubo un evento para sacar los datos almacenados
extern volatile bool_t event_frame_capture ;

/*==================[definiciones y macros]==================================*/

#define UART_PC        UART_USB

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
	uartInit(UART_USB,115200);
   // ---------- CONFIGURACIONES ------------------------------
	// Inicializar y configurar la plataforma
	//Inicializo frecuencia PCLK se la camara a 10 Mhz.
	uint32_t xClkFrequency = 10000000;
	uint8_t reg_value;
	uint8_t pll_divider_value = 0x10;
	uint32_t i;

	boardConfig();
	printf("Inicializando XCLK\r\n");
	//Se inicializa el clock hacia la camara
	OV7670initXClk(xClkFrequency);
	printf("XCLK Inicializado\r\n");
	printf("XCLK Configurado con un Duty Cycle del %d y una frecuencia de %ul\r\n",OV7670getXClkDutyCycle(),xClkFrequency);
	//Se inicializa el controlador de I2C de la EDU-CIAA a 100Khz
	OV7670initI2c();
	printf("INIT OV7670 REGS\r\n");
	//Valor por defualt de los registro de la camara
	OV7670Init();
	//Se realiza una configuracion para que se envien los datos por QVGA 320x240
	printf("CONFIG QVGA OV7670\r\n");
	OV7670ConfigQVGA();
	//Se inicializa para que el formato de los pixeles sea YUV422
	printf("CONFIG YUV 422\r\n");
	OV7670ConfigYUV422();


	//Se configura el preescaler en 16
	printf("CONFIG PLL PRESACLES con valor 0x%X\r\n",pll_divider_value);
	OV7670CofigPLLDivider(pll_divider_value);
	printf("CONFIG TEST PATTER SHIFTING 1 \r\n");
	OV7670ConfigTestPattern(SHIFTING_1); //Configuro valor del prescaler

	//Solo por motivo de debug se lee la primera configuracion de todos los registros.
	OV7670ReadAllRegs();

	//Se usan estos GPIO para indicar cuando llegan las IRQs y cuanto duran en atenderse
	gpioInit(T_COL0,GPIO_OUTPUT);
	gpioWrite(T_COL0,OFF);

	gpioInit(T_FIL2,GPIO_OUTPUT);
	gpioWrite(T_FIL2,OFF);

	gpioInit(T_FIL3,GPIO_OUTPUT);
	gpioWrite(T_FIL3,OFF);

	//Configuro los GPIOs asociados para adquirir los datos
	OV7670PixelDataInit();
	//Inicializo las interrupciones para la entrada de sincronismo vertical de la camara solo se configuran no se habilitan
	IRQInitVSync();
	//Inicializo las interrupciones para la entrada de sincronismo horizontal de la camara solo se configuran no se habilitan
	IRQInitHRef();
	//Inicializo las interrupcion para el clock correspondiente a los pixel solo se configuran no se habilitan
	IRQPixelClock();

	while (1){
		//Funcion de la SAPI para esperar el comando "s\r\n" por la consola y disparar la captura de un frame
		if(waitForReceiveStringOrTimeoutBlocking(UART_USB,"s\r\n",3,0xFFFFFFFFFFFFFFFF)== TRUE){
			uartWriteString(UART_USB,"Comando S recibido,esperando a completar un frame\r\n");
			//Se habilitan las interrupciones para obtener un frame
			IRQOV7670Enable();
			//Bloqueado hasta que se obtiene un frame
			while(	event_frame_capture == FALSE ){}
			//Una vez que se obtiene el frame se deshabilitan las interrupciones.
			IRQOV7670Disable();
			uartWriteString(UART_USB,"CANTIDAD DE LINEAS RECIBIDAS\r\n");
			//Se imprime la cantidad de pulsos de sincronismo horizontal recibidos
			printf("%d\r\n",lines_received);
			for(i=0;i<240;i++){
					//Se imprime cuantos clocks de pixel se recibieron dentro de la ventana de sincronismo horizontal
					printf("LINEA:%d CANTIDAD DE PIXELES:%d\r\n",i,pixel_array_count[i]);
			}
			for(i=0;i<640;i++){
					//Se imprime los pixel de la ultima linea recibida, aca dentro tiene que estar el patron de 1's que se va shifteando.
					printf("ULTIMA LINEA PIXRL:%d VALOR:%d\r\n",i,line[i]);
			}
			//Para evitar que se vuelva a entrar se pone en false a la espera de otro comando con "s\r\n" por parte del usuario.
			event_frame_capture= FALSE;
		}
   }
}
