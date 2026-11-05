//openocd -f interface/jlink.cfg -c "transport select swd" -f target/cx32l003.cfg

/*
********************************************************************************
*	STANDARD LIBRARIES
********************************************************************************
*/
#include	<stdlib.h>
#include	<string.h>
#include	<stdbool.h>
#include	<unistd.h>

/*
********************************************************************************
*	INCLUDE FILES
********************************************************************************
*/

#include "cx32l003.h"
#include "cx32l003_rcc.h"
#include "cx32l003_gpio.h"
#include "cx32l003_uart.h"

/*
********************************************************************************
*	LOCAL CONSTANTS
********************************************************************************
*/

//----------------------------PIN-----------------------------------------------

// PA1 UART0_RXD
// PA2 UART0_TXD
// PA3 ?

// PB5 IO_OUTPUT NRST
#define SET_NRST()	GPIO_SetBits( GPIOB, GPIO_Pin_5 )
#define RST_NRST()	GPIO_ResetBits( GPIOB, GPIO_Pin_5 )

// PB4 IO_OUTPUT NSS
#define SET_NSS()	GPIO_SetBits( GPIOB, GPIO_Pin_4 )
#define RST_NSS()	GPIO_ResetBits( GPIOB, GPIO_Pin_4 )

// PC3 IO_INPUT DIO1
#define GET_DIO1()	GPIO_ReadInputDataBit( GPIOC, GPIO_Pin_3 )

// PC4 IO_OUTPUT LED GREEN
#define SET_LED_GREEN()	GPIO_SetBits( GPIOC, GPIO_Pin_4 )
#define RST_LED_GREEN()	GPIO_ResetBits( GPIOC, GPIO_Pin_4 )

// PC5 IO_OUTPUT SCK
#define SET_SCK()	GPIO_SetBits( GPIOC, GPIO_Pin_5 )
#define RST_SCK()	GPIO_ResetBits( GPIOC, GPIO_Pin_5 )

// PC6 IO_OUTPUT RFSW
#define SET_RFSW()	GPIO_SetBits( GPIOC, GPIO_Pin_6 )
#define RST_RFSW()	GPIO_ResetBits( GPIOC, GPIO_Pin_6 )

// PC7 SWDIO
// PD1 SWDCLK

// PD2 IO_INPUT BUSY
#define GET_BUSY()	GPIO_ReadInputDataBit( GPIOD, GPIO_Pin_2 )

// PD3 IO_OUTPUT LED RED
#define SET_LED_RED()	GPIO_SetBits( GPIOD, GPIO_Pin_3 )
#define RST_LED_RED()	GPIO_ResetBits( GPIOD, GPIO_Pin_3 )

// PD4 IO_INPUT KEY
#define GET_KEY()	GPIO_ReadInputDataBit( GPIOD, GPIO_Pin_4 )

// PD5 IO_INPUT MISO
#define GET_MISO()	GPIO_ReadInputDataBit( GPIOD, GPIO_Pin_5 )

// PD6 IO_OUTPUT MOSI
#define SET_MOSI()	GPIO_SetBits( GPIOD, GPIO_Pin_6 )
#define RST_MOSI()	GPIO_ResetBits( GPIOD, GPIO_Pin_6 )

//------------------------------------------------------------------------------
#define UARTx		UART0
#define TIMx		TIM10
#define BAUDRATE	115200

#define trim_24M    (*((volatile uint16_t*)(0x180000C0))&0x0fff);
#define trim_4M     (*((volatile uint16_t*)(0x180000C8))&0x0fff);

#define SYSCLK			Fpclk //24000000
#define TICKS_PER_SEC	100

//------------------------------------------------------------------------------

/*
********************************************************************************
*	LOCAL DATA TYPES
********************************************************************************
*/

/*
********************************************************************************
*	LOCAL GLOBAL VARIABLES
********************************************************************************
*/

static uint32_t TickCnt;
static uint32_t UserTickCnt;

/*
********************************************************************************
*	LOCAL FUNCTION PROTOTYPES
********************************************************************************
*/

/*
********************************************************************************
*	Sys_Tick_ISR
********************************************************************************
*/
static void Sys_Tick_ISR (void) {

	//--------------------------------------------------------------------------
	TickCnt++;
	UserTickCnt++;
	//--------------------------------------------------------------------------
	if( GET_BUSY() )
		RST_LED_RED();
	else
		SET_LED_RED();
	//--------------------------------------------------------------------------
}

/*
********************************************************************************
*	GetTickCnt
********************************************************************************
*/
static uint32_t GetTickCnt(void) {

	return TickCnt;
}

/*
********************************************************************************
*	ClockInit
********************************************************************************
*/
static void ClockInit(void) {

	//--------------------------------------------------------------------------
	RCC->REGLOCK = RCC_REGLOCKKEY;
	RCC->HIRCCR = 0x5a690000 + trim_24M;
	RCC->CLKCON =  0x5a690205;
	RCC->REGLOCK = RCC_RESGLOCKKEY;
	//--------------------------------------------------------------------------
	SysTick_Config( SYSCLK / TICKS_PER_SEC );
    //--------------------------------------------------------------------------
}

/*
********************************************************************************
*	GPIOInit
********************************************************************************
*/
static void GPIOInit(void) {

 GPIO_InitTypeDef GPIO_InitStruct;

	//--------------------------------------------------------------------------
	RCC->AHBCLKEN |=	(	RCC_AHBPeriph_GPIOAEN   |
							RCC_AHBPeriph_GPIOBEN   |
							RCC_AHBPeriph_GPIOCEN   |
							RCC_AHBPeriph_GPIODEN );
	//--------------------------------------------------------------------------
	// PA1 UART0_RXD
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1; // RX
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init( GPIOA,&GPIO_InitStruct );
    GPIO_PinAFConfig( GPIOA, GPIO_PinSource1, GPIO_AF_UART0_RX_PA1 );
	//--------------------------------------------------------------------------
	// PA2 UART0_TXD
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2; // TX
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init(GPIOA,&GPIO_InitStruct);
    GPIO_PinAFConfig( GPIOA, GPIO_PinSource2, GPIO_AF_UART0_TX_PA2 );
    //--------------------------------------------------------------------------
	// PA3 ?
    //GPIO_SetBits( GPIOA, GPIO_Pin_3 );
    //GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    //GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    //GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
    //GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    //GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    //GPIO_Init( GPIOA, &GPIO_InitStruct );
    //--------------------------------------------------------------------------
	// PB5 IO_OUTPUT NRST x
    GPIO_SetBits( GPIOB, GPIO_Pin_5 );
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init( GPIOB, &GPIO_InitStruct );
    //--------------------------------------------------------------------------
	// PB4 IO_OUTPUT NSS x
    GPIO_SetBits( GPIOB, GPIO_Pin_4 );
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init( GPIOB, &GPIO_InitStruct );
    //--------------------------------------------------------------------------
	// PC3 IO_INPUT DIO1 x
    //GPIO_ResetBits( GPIOC, GPIO_Pin_3 );
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init( GPIOC, &GPIO_InitStruct );
    //--------------------------------------------------------------------------
	// PC4 IO_OUTPUT LED GREEN x
    GPIO_SetBits( GPIOC, GPIO_Pin_4 );
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init( GPIOC, &GPIO_InitStruct );
    //--------------------------------------------------------------------------
	// PC5 IO_OUTPUT SCK x
    GPIO_ResetBits( GPIOC, GPIO_Pin_5 );
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init( GPIOC, &GPIO_InitStruct );
    //--------------------------------------------------------------------------
	// PC6 IO_OUTPUT RFSW x
    GPIO_SetBits( GPIOC, GPIO_Pin_6 );
    //GPIO_ResetBits( GPIOC, GPIO_Pin_6 );
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init( GPIOC, &GPIO_InitStruct );
    //--------------------------------------------------------------------------
	// PC7 SWDIO
	// PD1 SWDCLK
    //--------------------------------------------------------------------------
	// PD2 IO_INPUT BUSY x
    GPIO_ResetBits( GPIOD, GPIO_Pin_2 );
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init( GPIOD, &GPIO_InitStruct );
    //--------------------------------------------------------------------------
	// PD3 IO_OUTPUT LED RED x
    GPIO_SetBits( GPIOD, GPIO_Pin_3 );
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init( GPIOD, &GPIO_InitStruct );
    //--------------------------------------------------------------------------
	// PD4 IO_INPUT KEY x
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init( GPIOD, &GPIO_InitStruct );
    //--------------------------------------------------------------------------
	// PD5 IO_INPUT MISO x
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init( GPIOD, &GPIO_InitStruct );
    //--------------------------------------------------------------------------
	// PD6 IO_OUTPUT MOSI x
    GPIO_SetBits( GPIOD, GPIO_Pin_6 );
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init( GPIOD, &GPIO_InitStruct );
    //--------------------------------------------------------------------------
}

/*
********************************************************************************
*	UARTInit
********************************************************************************
*/
static void UARTInit(void) {

 UART_InitTypeDef UART_InitStructure;

    //--------------------------------------------------------------------------
	RCC->APBCLKEN |= RCC_APBPeriph_UART0CKEN;
	//--------------------------------------------------------------------------
    UART_InitStructure.UART_BaudRate = BAUDRATE;
    UART_InitStructure.UART_BaudRateDbaud_Enable = ENABLE;
    UART_InitStructure.UART_BaudRateTimer_Selected = DISABLE;
    UART_InitStructure.UART_Mode = UART_MODE1;
    UART_Init( UARTx, TIMx, &UART_InitStructure);
    //--------------------------------------------------------------------------
    UART_ITConfig( UARTx, UART_RIEN_EABLE, ENABLE );
    UART_ITConfig( UARTx, UART_TIEN_EABLE, ENABLE );
    //--------------------------------------------------------------------------
    UART_Cmd( UARTx, UART_RXEN_EABLE, ENABLE );
    //--------------------------------------------------------------------------
}

/*
********************************************************************************
*	SPI_BYTE
********************************************************************************
*/
static uint8_t SPI_BYTE( uint8_t Byte ) {

 uint8_t BIT_Ctr;

	//--------------------------------------------------------------------------
	for( BIT_Ctr = 0; BIT_Ctr < 8; BIT_Ctr++ ) {
		//----------------------------------------------------------------------
		if( Byte & 0x80 )
			SET_MOSI();
		else
			RST_MOSI();
		//----------------------------------------------------------------------
		SET_SCK();
		//----------------------------------------------------------------------
		Byte = ( Byte << 1 );
		//----------------------------------------------------------------------
		if( GET_MISO() != 0 )
			Byte |= 0x01;
		//----------------------------------------------------------------------
		RST_SCK();
		//----------------------------------------------------------------------
	}
	//--------------------------------------------------------------------------
	return Byte;
}

/*
********************************************************************************
*	LLCC68Init
********************************************************************************
*/
static void LLCC68Init(void) {

 uint32_t	Pause;

	//--------------------------------------------------------------------------
	Pause = GetTickCnt() + TICKS_PER_SEC; // 1000 ms
	while( Pause > GetTickCnt() );
	//--------------------------------------------------------------------------
	RST_NRST();
	Pause = GetTickCnt() + ( TICKS_PER_SEC / 10 ); // 100 ms
	while( Pause > GetTickCnt() );
	//--------------------------------------------------------------------------
	SET_NRST();
	Pause = GetTickCnt() + ( TICKS_PER_SEC / 10 ); // 100 ms
	while( Pause > GetTickCnt() );
	//--------------------------------------------------------------------------
}


/*
********************************************************************************
*	WaitOnBusy
********************************************************************************
*/
void WaitOnBusy(void) {

 uint32_t Pause;

	//--------------------------------------------------------------------------
	Pause = GetTickCnt() + ( TICKS_PER_SEC / 10 ); // 100 ms
	//--------------------------------------------------------------------------
	do {
		//----------------------------------------------------------------------
		if( GET_BUSY() == 0 )
			return;
		//----------------------------------------------------------------------
	} while( Pause > GetTickCnt() );
	//--------------------------------------------------------------------------
	// Wake up
	//--------------------------------------------------------------------------
	RST_NSS();
	//--------------------------------------------------------------------------
	Pause = GetTickCnt() + ( TICKS_PER_SEC / 10 ); // 100 ms
	//--------------------------------------------------------------------------
	while( Pause > GetTickCnt() );
	//--------------------------------------------------------------------------
	SET_NSS();
	//--------------------------------------------------------------------------
	Pause = GetTickCnt() + ( TICKS_PER_SEC ); // 1000 ms
	//--------------------------------------------------------------------------
	do {
		//----------------------------------------------------------------------
		if( GET_BUSY() == 0 )
			return;
		//----------------------------------------------------------------------
	} while( Pause > GetTickCnt() );
	//--------------------------------------------------------------------------
}

/*
********************************************************************************
*	main
********************************************************************************
*/
int main(void) {

    //--------------------------------------------------------------------------
    ClockInit();
    GPIOInit();
    UARTInit();
    LLCC68Init();
    //--------------------------------------------------------------------------
    uint8_t	Index = 0;
    uint8_t	DataLen = 0;
    uint8_t	DataIndex = 0;
    //--------------------------------------------------------------------------
    while(1) {
    	//----------------------------------------------------------------------
    	if( UART_GetITStatus( UARTx, UART_ISR_RI ) != 0 ) {
    		//------------------------------------------------------------------
    	 uint8_t Byte = UART_ReceiveData( UARTx );
    		UART_ClearITBit( UARTx, UART_ISR_RI );
    		//------------------------------------------------------------------
    		if( Index == 0 ) {
    			//--------------------------------------------------------------
    			if( Byte == 0x55 ) {
    				//----------------------------------------------------------
        			Index++;
        			UserTickCnt = 0;
        			DataIndex = 0;
        			//----------------------------------------------------------
    			}
    			//--------------------------------------------------------------
    		} else if( Index == 1 ) {
				//--------------------------------------------------------------
				if( Byte == 0xFF ) // Command restart (x55 xFF)
					NVIC_SystemReset();
				//--------------------------------------------------------------
				WaitOnBusy();
				//--------------------------------------------------------------
				RST_NSS();
				//--------------------------------------------------------------
				RST_LED_GREEN();
				//--------------------------------------------------------------
				UART_SendData( UARTx, SPI_BYTE( Byte ) );
				while( UART_GetITStatus( UARTx, UART_ISR_TI ) != SET );
				//--------------------------------------------------------------
    			Index++;
    			//--------------------------------------------------------------
    		} else if( Index == 2 ) {
    			//--------------------------------------------------------------
    			DataLen = Byte;
    			Index++;
    			//--------------------------------------------------------------
    		} else if( DataIndex < DataLen ) {
				//--------------------------------------------------------------
				UART_SendData( UARTx, SPI_BYTE( Byte ) );
				while( UART_GetITStatus( UARTx, UART_ISR_TI ) != SET );
				//--------------------------------------------------------------
				DataIndex++;
				//--------------------------------------------------------------
			} else if(	( DataIndex == DataLen ) &&
    		    		( Byte == 0xAA ) ) {
    			//--------------------------------------------------------------
    			Index = 0;
    			//--------------------------------------------------------------
				SET_NSS();
				//--------------------------------------------------------------
    			SET_LED_GREEN();
    			//--------------------------------------------------------------
    		} else
    			Index = 0;
    		//------------------------------------------------------------------
    	}
    	//----------------------------------------------------------------------
    	if( ( Index != 0 ) &&
    		( UserTickCnt > ( TICKS_PER_SEC * 2 ) ) ) { // 2000 msec
			//------------------------------------------------------------------
			Index = 0;
			//------------------------------------------------------------------
			SET_NSS();
			//------------------------------------------------------------------
			SET_LED_GREEN();
			//------------------------------------------------------------------
		}
    	//----------------------------------------------------------------------
    }
    //--------------------------------------------------------------------------
	return 0;
}

/*
********************************************************************************
*	Reset_Handler
********************************************************************************
*/
extern void _stack(void);
extern uint32_t _etext;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _ebss;
extern uint32_t _sbss;

void Reset_Handler(void) {

 uint32_t *pulSrc, *pulDest;

 	//--------------------------------------------------------------------------
	pulSrc = &_etext;
	pulDest = &_sdata;
	while( pulDest < &_edata )
		*(pulDest++) = *(pulSrc++);
	//--------------------------------------------------------------------------
	pulDest = &_sbss;
	while( pulDest < &_ebss )
		*(pulDest++) = 0;
	//--------------------------------------------------------------------------
	main();
	//--------------------------------------------------------------------------
	NVIC_SystemReset();
	//--------------------------------------------------------------------------
    return;
}

/*
********************************************************************************
*	INTERRUPT VECTOR TABLE
********************************************************************************
*/
void App_NMI_ISR(void) {

}

/*
********************************************************************************
*	EXCEPTIONT VECTOR TABLE
********************************************************************************
*/
void App_Fault_ISR(void) {

	//--------------------------------------------------------------------------
	NVIC_SystemReset();
	//--------------------------------------------------------------------------
}

/*
********************************************************************************
*	EXCEPTIONT VECTOR TABLE
********************************************************************************
*/
void App_MemFault_ISR(void) {

	//--------------------------------------------------------------------------
	NVIC_SystemReset();
	//--------------------------------------------------------------------------
}

/*
********************************************************************************
*	EXCEPTIONT VECTOR TABLE
********************************************************************************
*/
void App_BusFault_ISR(void) {

	//--------------------------------------------------------------------------
	NVIC_SystemReset();
	//--------------------------------------------------------------------------
}

/*
********************************************************************************
*	EXCEPTIONT VECTOR TABLE
********************************************************************************
*/
void App_UsageFault_ISR(void) {

	//--------------------------------------------------------------------------
	NVIC_SystemReset();
	//--------------------------------------------------------------------------
}

/*
********************************************************************************
*	INTERRUPT VECTOR TABLE
********************************************************************************
*/
void App_Spurious_ISR(void) {

}

/*
********************************************************************************
*	EXCEPTION / INTERRUPT VECTOR TABLE
********************************************************************************
*/

//__attribute__ ((__used__))
__attribute__ ((section(".isr_vector")))
void (* const __vector_table[])(void) = {
	&_stack, // SP start value
	Reset_Handler, // PC start value
	App_NMI_ISR,
	App_Fault_ISR,
	App_MemFault_ISR,
	App_BusFault_ISR,
	App_UsageFault_ISR,
	App_Spurious_ISR, // Reserved
	App_Spurious_ISR, // Reserved
	App_Spurious_ISR, // Reserved
	App_Spurious_ISR, // Reserved
	App_Spurious_ISR, // SVCall Handler
	App_Spurious_ISR, // Debug Monitor Handler
	App_Spurious_ISR, // Reserved
	App_Spurious_ISR, // PendSV Handler
	Sys_Tick_ISR, // SysTick Handler
	App_Spurious_ISR, // GPIOA_IRQn                  = 0
	App_Spurious_ISR, // GPIOB_IRQn                  = 1
	App_Spurious_ISR, // GPIOC_IRQn                  = 2
	App_Spurious_ISR, // GPIOD_IRQn                  = 3
	App_Spurious_ISR, // Reserved
	App_Spurious_ISR, // Reserved
	App_Spurious_ISR, // UART0_IRQn                  = 6
	App_Spurious_ISR, // UART1_IRQn                  = 7
	App_Spurious_ISR, // LPUART_IRQn                 = 8
	App_Spurious_ISR, // Reserved
	App_Spurious_ISR, // SPI0COMB_IRQn               = 10
	App_Spurious_ISR, // SPI_DUMMYCOMB_IRQn          = 11
	App_Spurious_ISR, // I2C0_IRQn                   = 12
	App_Spurious_ISR, // I2C_DUMMY_IRQn              = 13
	App_Spurious_ISR, // TIMER10_IRQn                 = 14
	App_Spurious_ISR, // TIMER11_IRQn                 = 15
	App_Spurious_ISR, // LPTIMER_IRQn                = 16
	App_Spurious_ISR, // Reserved
	App_Spurious_ISR, // PWM_IRQn                    = 18
	App_Spurious_ISR, // ADVTIM_IRQn                = 19
	App_Spurious_ISR, // Reserved
	App_Spurious_ISR, // PCA_IRQn                    = 21
	App_Spurious_ISR, // WWDT_IRQn                   = 22
	App_Spurious_ISR, // IWDT_IRQn                   = 23
	App_Spurious_ISR, // ADC0_IRQn                   = 24
	App_Spurious_ISR, // LVD_IRQn                    = 25
	App_Spurious_ISR, // VC_IRQn                     = 26
	App_Spurious_ISR, // Reserved
	App_Spurious_ISR, // AWK_IRQn                    = 28
	App_Spurious_ISR, // ONEWIRE_IRQn                = 29
	App_Spurious_ISR, // RTC_MATCH0_IRQn             = 30
	App_Spurious_ISR // CLKTRIM_IRQn                = 31
};

/*
********************************************************************************
*	END
********************************************************************************
*/

