#include "ov7670IRQ.h"
#include "sapi.h"

volatile uint32_t href_count = 0;
volatile uint32_t lines_received = 0;
volatile uint32_t pixel_count = 0;
volatile uint32_t pixel_array_count [240];
volatile uint8_t line[640];

volatile bool_t event_frame_capture = FALSE;

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
//	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(4));
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(5));

	NVIC_ClearPendingIRQ( PIN_INT0_IRQn );
    NVIC_ClearPendingIRQ( PIN_INT1_IRQn );
	NVIC_ClearPendingIRQ( PIN_INT2_IRQn );
    NVIC_ClearPendingIRQ( PIN_INT3_IRQn );
//  NVIC_ClearPendingIRQ( PIN_INT4_IRQn );
	NVIC_ClearPendingIRQ( PIN_INT5_IRQn );

	NVIC_EnableIRQ( PIN_INT0_IRQn );
    NVIC_EnableIRQ( PIN_INT1_IRQn );
    NVIC_EnableIRQ( PIN_INT2_IRQn );
    NVIC_EnableIRQ( PIN_INT3_IRQn );
//	NVIC_EnableIRQ( PIN_INT4_IRQn );
	NVIC_EnableIRQ( PIN_INT5_IRQn );
}


void IRQOV7670Disable (void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(0));
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(1));
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(2));
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(3));
//	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(4));
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(5));

	NVIC_ClearPendingIRQ( PIN_INT0_IRQn );
    NVIC_ClearPendingIRQ( PIN_INT1_IRQn );
	NVIC_ClearPendingIRQ( PIN_INT2_IRQn );
    NVIC_ClearPendingIRQ( PIN_INT3_IRQn );
//  NVIC_ClearPendingIRQ( PIN_INT4_IRQn );
	NVIC_ClearPendingIRQ( PIN_INT5_IRQn );

	NVIC_DisableIRQ( PIN_INT0_IRQn );
	NVIC_DisableIRQ( PIN_INT1_IRQn );
	NVIC_DisableIRQ( PIN_INT2_IRQn );
	NVIC_DisableIRQ( PIN_INT3_IRQn );
//	NVIC_EnableIRQ( PIN_INT4_IRQn );
	NVIC_DisableIRQ( PIN_INT5_IRQn );
}

//VSYNC Falling Edge
void GPIO0_IRQHandler(void){
	static bool_t start_frame_grabber = FALSE;
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(0));
	gpioWrite(T_COL0,OFF);
	//Descarto primer frame luego de que empece ya que es probable que este incompleto
	lines_received = href_count;
	href_count = 0;
	if(start_frame_grabber == FALSE){
		start_frame_grabber = TRUE;
	}
	else {
		event_frame_capture = TRUE;
		start_frame_grabber = FALSE;
	}

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
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(4));
}

//Pixel Clock Rising Edge
void GPIO5_IRQHandler(void){
	gpioToggle(T_FIL3);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(5));
	if(gpioRead(ENET_MDC) == ON){
		pixel_count++;
		OV7670GetPixelData ();
	}
	gpioToggle(T_FIL3);

}
