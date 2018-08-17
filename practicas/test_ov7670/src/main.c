/*==================[inlcusiones]============================================*/

#include "sapi.h"
#include "ov7670_regs.h"
#include "ov7670.h"

#define RISING_EDGE 0
#define FALLING_EDGE 1

uint32_t href_count = 0;
uint32_t pixel_count = 0;
uint32_t pixel_array_count [240];
uint8_t line[640];

void IRQInitVSync (void);
void IRQInitHRef (void);
void IRQPixelClock (void);
void IRQOV7670Enable (void);
void OV7670PixelDataInit (void);
void OV7670GetPixelData (void);

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


	boardConfig();
	printf("Inicializando XCLK\r\n");
	OV7670initXClk(xClkFrequency);
	printf("XCLK Inicializado\r\n");
	printf("XCLK Configurado con un Duty Cycle del %d y una frecuencia de %ul\r\n",OV7670getXClkDutyCycle(),xClkFrequency);
	OV7670initI2c();
	printf("INIT OV7670\r\n");
	OV7670Init();
	printf("CONFIG QVGA OV7670\r\n");
	OV7670ConfigQVGA();
	printf("CONFIG YUV 422\r\n");
	OV7670ConfigYUV422();
	/*
	OV7670readReg(REG_CLKRC,&reg_value);
	OV7670writeReg(REG_CLKRC,(reg_value & 0b10000000) | 0b00000111);
   	OV7670readReg(REG_CLKRC,&reg_value);
	printf("OV7670-I2C-CLKRC REG=0X%X 0d%d\r\n",reg_value,reg_value);
	*/
	OV7670writeReg(0x11, 0x1F); //PReescale en maximo valor 0x1F
	printf("CONFIG TEST PATTER SHIFTING 1 \r\n");
	OV7670ConfigTestPattern(SHIFTING_1); //Configuro valor del prescaler

	OV7670ReadAllRegs();

	//Only for debug, only for bypassing VSYN
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
	IRQOV7670Enable();

   while (1){
	   /*
	   OV7670readReg(REG_CLKRC,&reg_value);
	   printf("OV7670-I2C-CLKRC REG=0X%X 0d%d\r\n",reg_value,reg_value);
	   OV7670readReg(DBLV,&reg_value);
	   printf("OV7670-I2C-PLL REG=0X%X 0d%d\r\n",reg_value,reg_value);
	   OV7670readReg(REG_COM7,&reg_value);
	   printf("OV7670-I2C-REG COM 7 REG=0X%X 0d%d\r\n",reg_value,reg_value);
	   OV7670readReg(REG_BLUE,&reg_value);
	   printf("OV7670-I2C-REG BLUE GAIN=0X%X 0d%d\r\n",reg_value,reg_value);
	   OV7670readReg(REG_COM10,&reg_value);
	   printf("OV7670-I2C-REG COM10=0X%X 0d%d\r\n\n",reg_value,reg_value);
	   delay(1000);
	   */
   }

}

void OV7670PixelDataInit (void){
	gpioInit(GPIO1,GPIO_INPUT);
	gpioInit(GPIO2,GPIO_INPUT);
	gpioInit(GPIO3,GPIO_INPUT);
	gpioInit(GPIO4,GPIO_INPUT);
	gpioInit(GPIO5,GPIO_INPUT);
	gpioInit(GPIO6,GPIO_INPUT);
	gpioInit(GPIO7,GPIO_INPUT);
	gpioInit(GPIO8,GPIO_INPUT);
}

void OV7670GetPixelData (void){
	uint8_t temp=0;
	//temp= Chip_GPIO_ReadValue(LPC_GPIO_PORT,6);
	temp=gpioRead(GPIO1);
	temp=(temp | gpioRead(GPIO2)<<1);
	temp=(temp | gpioRead(GPIO3)<<2);
	temp=(temp | gpioRead(GPIO4)<<3);
	temp=(temp | gpioRead(GPIO5)<<4);
	temp=(temp | gpioRead(GPIO6)<<5);
	temp=(temp | gpioRead(GPIO7)<<6);
	temp=(temp | gpioRead(GPIO8)<<7);
	line[pixel_count] = temp;
}

void IRQInitVSync (void){
		//ENET TXEN Enable IRQ Falling Edged
		gpioInit(ENET_TXEN,GPIO_INPUT);

		Chip_SCU_GPIOIntPinSel(0,0,1);
		Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(0));
		Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH(0));
		Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT,PININTCH(0));
		//ENET TXEN Enable IRQ Rising  Edged
		Chip_SCU_GPIOIntPinSel(1,0,1);
		Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(1));
		Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH(1));
		Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT,PININTCH(1));
}

void IRQInitHRef (void){
		//ENET MDC Enable IRQ Falling Edged
		gpioInit(ENET_MDC,GPIO_INPUT);

		Chip_SCU_GPIOIntPinSel(2,3,15);
		Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(2));
		Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH(2));
		Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT,PININTCH(2));
		//ENET MDC Enable IRQ Rising  Edged
		Chip_SCU_GPIOIntPinSel(3,3,15);
		Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(3));
		Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH(3));
		Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT,PININTCH(3));
}


void IRQPixelClock (void){

	//ENET_TXD1 Enable IRQ Falling  Edged
	gpioInit(ENET_TXD1,GPIO_INPUT);

	Chip_SCU_GPIOIntPinSel(4,0,15);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(4));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH(4));
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT,PININTCH(4));
	//ENET_TXD1 Enable IRQ Rising  Edged
	Chip_SCU_GPIOIntPinSel(5,0,15);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(5));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH(5));
	Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT,PININTCH(5));
}


void IRQOV7670Enable (void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(0));
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(1));
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(2));
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(3));
	//Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(4));
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(5));

	NVIC_ClearPendingIRQ( PIN_INT0_IRQn );
    NVIC_ClearPendingIRQ( PIN_INT1_IRQn );
	NVIC_ClearPendingIRQ( PIN_INT2_IRQn );
    NVIC_ClearPendingIRQ( PIN_INT3_IRQn );
    //NVIC_ClearPendingIRQ( PIN_INT4_IRQn );
	NVIC_ClearPendingIRQ( PIN_INT5_IRQn );

	NVIC_EnableIRQ( PIN_INT0_IRQn );
    NVIC_EnableIRQ( PIN_INT1_IRQn );
    NVIC_EnableIRQ( PIN_INT2_IRQn );
    NVIC_EnableIRQ( PIN_INT3_IRQn );
	//NVIC_EnableIRQ( PIN_INT4_IRQn );
	NVIC_EnableIRQ( PIN_INT5_IRQn );
}


void IRQOV7670Disable (void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(0));
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(1));
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(2));
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(3));
	//Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(4));
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(5));

	NVIC_ClearPendingIRQ( PIN_INT0_IRQn );
    NVIC_ClearPendingIRQ( PIN_INT1_IRQn );
	NVIC_ClearPendingIRQ( PIN_INT2_IRQn );
    NVIC_ClearPendingIRQ( PIN_INT3_IRQn );
    //NVIC_ClearPendingIRQ( PIN_INT4_IRQn );
	NVIC_ClearPendingIRQ( PIN_INT5_IRQn );

	NVIC_DisableIRQ( PIN_INT0_IRQn );
	NVIC_DisableIRQ( PIN_INT1_IRQn );
	NVIC_DisableIRQ( PIN_INT2_IRQn );
	NVIC_DisableIRQ( PIN_INT3_IRQn );
	//NVIC_EnableIRQ( PIN_INT4_IRQn );
	NVIC_DisableIRQ( PIN_INT5_IRQn );
}


//VSYNC Falling Edge
void GPIO0_IRQHandler(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(0));
	gpioWrite(T_COL0,OFF);
	uartWriteString(UART_USB,"VSYNC PULSE RECEIVED\r\n");
	printf("%d\r\n",href_count);
	href_count = 0;
	uartWriteString(UART_USB,"LAST PIXEL COUNT RECEIVED\r\n");
	printf("%d\r\n",pixel_array_count[239]);
}

//VSYNC Rising Edge
void GPIO1_IRQHandler(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(1));
	gpioWrite(T_COL0,ON);
}


//HREF Falling Edge
void GPIO2_IRQHandler(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(2));
	pixel_array_count[href_count]= pixel_count;
	pixel_count = 0;
	href_count++;
	gpioWrite(T_FIL2,OFF);
}

//HREF Rising Edge
void GPIO3_IRQHandler(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(3));

	gpioWrite(T_FIL2,ON);
}


//Pixel Clock Falling Edge
void GPIO4_IRQHandler(void){
	uartWriteString(UART_USB,"ERROR");

	//No falling edge with Pixel Clock Implemented
	/*
	gpioToggle(T_FIL3);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(4));
	if(gpioRead(ENET_MDC) == ON){
		//Get Pixel

	}
	gpioToggle(T_FIL3);
	*/
}

//Pixel Clock Rising Edge
void GPIO5_IRQHandler(void){
	gpioToggle(T_FIL3);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(5));
	if(gpioRead(ENET_MDC) == ON){
		pixel_count++;
		OV7670GetPixelData ();
		//gpioWrite(T_FIL3,ON);
	}
	gpioToggle(T_FIL3);

}
