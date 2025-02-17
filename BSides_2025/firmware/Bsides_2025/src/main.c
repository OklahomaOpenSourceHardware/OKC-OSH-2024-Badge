/*
 * Example for Timer 2 in "encoder" mode
 * 07-01-2023 frosty adapted example from tim2_pwm_remap
 *
 * Usage: 
 * Connect A/B pins of encoder to PC2/PC5
 * Spin the encoder and see the count increase/decrease on the debug monitor
 *
 * Points of interest:
 * 1. Port D pins and C0,C4, and C7 have a diode that clamps the pin to Vcc
 * 	and are not 5V tolerant when Vcc is 3V3
 * 2. Port C pins 1,2,3,5, and 6 are not clamped to Vcc and are should be
 * 	5V tolerant.
 * 2. Encoder mode uses CH1/CH2 so Timer2 must be used with a PARTIALREMAP1 so
 * 	CH1/CH2 pins are on 5V tolerant pins.
 * 3. If you don't require 5V tolerance you can use any of the REMAP options
 *
 * /

PC0        GPIO_Y4          T2CH3
PC1        SDA              TIM2_CH4
PC2        SCL              TIM2_CH1
PC3        GPIO_Y1          TIM2_CH2
PC4        GPIO_X1          TIM2_CH3
PC5        GPIO_X2          TIM2_CH2
PC6        GPIO_X3          TIM3_CH1
PC7        GPIO_X4          TIM3_CH2

PD0        GPIO_ROT_A       T1CH1N
PD1        DIO              T1CH3N
PD2        GPIO_ROT_B       T1CH1
PD3        GPIO_ROT_SW      T2CH2
PD4        GPIO_PHOTO_B     T2CH1ETR
PD5        GPIO_PHOTO_A     TIM4_CH4
PD6        GPIO_Y3          TIM3_CH1
PD7        GPIO_Y2          T2CH4  --note: requires disabling nRST in opt

PA1       SAO_GPIO1         TIM2_CH1
PA2       SAO_GPIO2         TIM2_CH2


Timer 2 pin mappings by AFIO->PCFR1
	00	AFIO_PCFR1_TIM2_REMAP_NOREMAP
		D4		T2CH1ETR
		D3		T2CH2
		C0		T2CH3
		D7		T2CH4  --note: requires disabling nRST in opt
	01	AFIO_PCFR1_TIM2_REMAP_PARTIALREMAP1
		C5		T2CH1ETR_
		C2		T2CH2_
		D2		T2CH3_
		C1		T2CH4_
	10	AFIO_PCFR1_TIM2_REMAP_PARTIALREMAP2
		C1		T2CH1ETR_
		D3		T2CH2
		C0		T2CH3
		D7		T2CH4  --note: requires disabling nRST in opt
	11	AFIO_PCFR1_TIM2_REMAP_FULLREMAP
		C1		T2CH1ETR_
		C7		T2CH2_
		D6		T2CH3_
		D5		T2CH4_
*/



#include "ch32fun.h"
//#include "lib_i2c.h" //needs https://github.com/ADBeta/CH32V003_lib_i2c/tree/main

#include "ch32v003_GPIO_branchless.h"
//#include "ch32v003fun.h" 

#include <stdio.h>


#define I2C_ADDR 0x68


uint32_t count;

int last = 0;
void handle_debug_input( int numbytes, uint8_t * data )
{
	last = data[0];
	count += numbytes;
}


/******************************************************************************************
 * initialize the charliplexed LED
 ******************************************************************************************/
void charliplex_led_init( void )
{
    	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC; //RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC;
/* not used
    // GPIO D6 Push-Pull
    GPIOD->CFGLR &= ~(0xf<<(4*6));
    GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*6);

    // GPIO D7 Push-Pull
    GPIOD->CFGLR &= ~(0xf<<(4*7));
    GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*7);

	// GPIO C0 Push-Pull
	GPIOC->CFGLR &= ~(0xf<<(4*0));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

    // GPIO C3 Push-Pull
    GPIOC->CFGLR &= ~(0xf<<(4*3));
    GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*3);
*/
    // GPIO C4 Push-Pull
    GPIOC->CFGLR &= ~(0xf<<(4*4));
    GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);
    // GPIO C5 Push-Pull
    GPIOC->CFGLR &= ~(0xf<<(4*5));
    GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*5);
    // GPIO C6 Push-Pull
    GPIOC->CFGLR &= ~(0xf<<(4*6));
    GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*6);
    // GPIO C7 Push-Pull
    GPIOC->CFGLR &= ~(0xf<<(4*7));
    GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*7);
}

/*
D1 = HH10
D2 = H1H0
D3 = 1HH0
D4 = HH01
D5 = H10H
D6 = 1H0H
D7 = H0H1
D8 = H01H
D9 = 10HH
D10= 0HH1
D11= 0H1H
D12= 01HH
*/

#define CHARLIE_X1(A, ...) A,
#define CHARLIE_X2(A, B, ...) B,
#define CHARLIE_X3(A, B, C, ...) C,
#define CHARLIE_X4(A, B, C, D, ...) D,
#define CHARLIE_COUNT(...) + 1

#define CHARLIEPLEX_LED(X)\
X(low, high, low, low)\
X(low, low, high, low)\
X(low, low, low, high)\
X(high, low, low, low)\
X(low, low, high, low)\
X(low, low, low, high)\
X(high, low, low, low)\
X(low, high, low, low)\
X(low, low, low, high)\
X(high, low, low, low)\
X(low, high, low, low)\
X(low, low, high, low)

//enum charlieplex_led_x1 { CHARLIEPLEX_LED(CHARLIE_X1) };
//enum charlieplex_led_x2 { CHARLIEPLEX_LED(CHARLIE_X2) };
//enum charlieplex_led_x3 { CHARLIEPLEX_LED(CHARLIE_X3) };
//enum charlieplex_led_x4 { CHARLIEPLEX_LED(CHARLIE_X4) };

//note, these are inverted from the above table(and need to be corrected)
#define CHARLIEPLEX_IO(Y)\
Y(GPIO_pinMode_I_floating, GPIO_pinMode_I_floating, GPIO_pinMode_O_pushPull, GPIO_pinMode_O_pushPull)\
Y(GPIO_pinMode_I_floating, GPIO_pinMode_O_pushPull, GPIO_pinMode_I_floating, GPIO_pinMode_O_pushPull)\
Y(GPIO_pinMode_O_pushPull, GPIO_pinMode_I_floating, GPIO_pinMode_I_floating, GPIO_pinMode_O_pushPull)\
Y(GPIO_pinMode_I_floating, GPIO_pinMode_I_floating, GPIO_pinMode_O_pushPull, GPIO_pinMode_O_pushPull)\
Y(GPIO_pinMode_I_floating, GPIO_pinMode_O_pushPull, GPIO_pinMode_O_pushPull, GPIO_pinMode_I_floating)\
Y(GPIO_pinMode_O_pushPull, GPIO_pinMode_I_floating, GPIO_pinMode_O_pushPull, GPIO_pinMode_I_floating)\
Y(GPIO_pinMode_I_floating, GPIO_pinMode_O_pushPull, GPIO_pinMode_I_floating, GPIO_pinMode_O_pushPull)\
Y(GPIO_pinMode_I_floating, GPIO_pinMode_O_pushPull, GPIO_pinMode_O_pushPull, GPIO_pinMode_I_floating)\
Y(GPIO_pinMode_O_pushPull, GPIO_pinMode_O_pushPull, GPIO_pinMode_I_floating, GPIO_pinMode_I_floating)\
Y(GPIO_pinMode_O_pushPull, GPIO_pinMode_I_floating, GPIO_pinMode_I_floating, GPIO_pinMode_O_pushPull)\
Y(GPIO_pinMode_O_pushPull, GPIO_pinMode_I_floating, GPIO_pinMode_O_pushPull, GPIO_pinMode_I_floating)\
Y(GPIO_pinMode_O_pushPull, GPIO_pinMode_O_pushPull, GPIO_pinMode_I_floating, GPIO_pinMode_I_floating) 

//enum charlieplex_io_x1 = { CHARLIEPLEX_IO(CHARLIE_X1) };
//enum charlieplex_io_x2 { CHARLIEPLEX_IO(CHARLIE_X2) };
//enum charlieplex_io_x3 { CHARLIEPLEX_IO(CHARLIE_X3) };
//enum charlieplex_io_x4 { CHARLIEPLEX_IO(CHARLIE_X4) };


#define  X(CHARLIE_X1) IO_X1 GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), IO_X1, GPIO_Speed_10MHz);
#undef   X
#define  Y(CHARLIE_X1) LED_X1 GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), LED_X1);
#undef   Y
#define  X(CHARLIE_X2) IO_X2 GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), IO_X2, GPIO_Speed_10MHz);
#undef   X
#define  Y(CHARLIE_X2) LED_X2 GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), LED_X2);
#undef   Y
#define  X(CHARLIE_X3) IO_X3 GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), IO_X3, GPIO_Speed_10MHz);
#undef   X
#define  Y(CHARLIE_X3) LED_X3 GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), LED_X3);
#undef   Y
#define  X(CHARLIE_X4) IO_X1 GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), IO_X4, GPIO_Speed_10MHz);
#undef   X
#define  Y(CHARLIE_X4) LED_X1 GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), LED_X4);
#undef Y      

//#define CHARLIE_X1_PINMODE(pin_i) GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), CHARLIE_X1, GPIO_Speed_10MHz);
//GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), IO_X1, GPIO_Speed_10MHz);

void test_led(int led)
{
    switch(led)
    {
        case 0:
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), high);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), low);
            break;
        case 1:
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), high);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), low);
            break;
        case 2:
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), high);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), low);
            break;
        case 3:
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), high);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), low);
            break;
        case 4:
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), high);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), low);
            break;
        case 5:
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), high);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), low);
            break;
        case 6:
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), high);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), low);
            break;
        case 7:
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), high);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), low);
            break;
        case 8:
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), high);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), low);
            break;
        case 9:
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), high);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), low);
            break;
        case 10:
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), high);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), low);
            break;
        case 11:
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), high);
            GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), low);
            break;
    }
}
            

//uses ch32v003_GPIO_branchless.h
/*


uint8_t charliplex_led_push_pull[12][4] = {
    { 0x00, 0x01, 0x00, 0x00 },
    { 0x00, 0x00, 0x01, 0x00 },
    { 0x00, 0x00, 0x00, 0x01 },
    { 0x01, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x01, 0x00 },
    { 0x00, 0x00, 0x00, 0x01 },
    { 0x01, 0x00, 0x00, 0x00 },
    { 0x00, 0x01, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x01 },
    { 0x01, 0x00, 0x00, 0x00 },
    { 0x00, 0x01, 0x00, 0x00 },
    { 0x00, 0x00, 0x01, 0x00 }
};
uint8_t charliplex_led_in_out[12][4] = {
    { 0x00, 0x00, 0x01, 0x01 },
    { 0x00, 0x01, 0x00, 0x01 },
    { 0x00, 0x01, 0x01, 0x00 },
    { 0x01, 0x01, 0x00, 0x00 },
    { 0x01, 0x00, 0x00, 0x01 },
    { 0x01, 0x00, 0x01, 0x00 },
    { 0x00, 0x01, 0x00, 0x01 },
    { 0x01, 0x00, 0x00, 0x01 },
    { 0x01, 0x01, 0x00, 0x00 },
    { 0x00, 0x01, 0x01, 0x00 },
    { 0x01, 0x00, 0x01, 0x00 },
    { 0x01, 0x01, 0x00, 0x00 }
};


void charliplex_led_test_GPIO(uint8_t x )
{
    GPIO_port_enable(GPIO_port_C);

    int i;
    for( i = 0; i < 12; i++ )
    {
        if(charliplex_led_in_out[x][i]) {
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, i), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
        }
        else {
            GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, i), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
        }
		GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, i), charliplex_led_push_pull[x][i]);



    }
    Delay_Ms( 100 );
}
*/
/*
void charliplex_led( uint8_t  data )
{
    if(data[0]) {
            // GPIO C4 Push-Pull
            GPIOC->CFGLR &= ~(0xf<<(4*4));
            GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);
    }
    else {
            // GPIO C4 Push-Pull
            GPIOC->CFGLR &= ~(0xf<<(4*4));
            GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_IN_FLOATING)<<(4*4);
    }
    if(data[1]) {

            // GPIO C5 Push-Pull
            GPIOC->CFGLR &= ~(0xf<<(4*5));
            GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*5);
    }
    else {
            // GPIO C5 Push-Pull
            GPIOC->CFGLR &= ~(0xf<<(4*5));
            GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_IN_FLOATING)<<(4*5);
    }
    if(data[2]) {
        // GPIO C6 Push-Pull
            GPIOC->CFGLR &= ~(0xf<<(4*6));
            GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*6);
    }
    else {
            // GPIO C6 Push-Pull
            GPIOC->CFGLR &= ~(0xf<<(4*6));
            GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_IN_FLOATING)<<(4*6);
    }
    if(data[3]) {
            // GPIO C7 Push-Pull
            GPIOC->CFGLR &= ~(0xf<<(4*7));
            GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*7);
    }
    else {
            // GPIO C7 Push-Pull
            GPIOC->CFGLR &= ~(0xf<<(4*7));
            GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_IN_FLOATING)<<(4*7);
    }
   
}
*/
//void charliplex_led( int led )
//{
   // charliplex_led_in_out( charlieplex_led_in_out[led] );

   // GPIOC->BSHR = 1<<(charliplex_led_push_pull[led] );
//}

/******************************************************************************************
 * initialize TIM2 for PWM
 ******************************************************************************************/
void t2encoder_init( void )
{
	// Enable GPIOC, TIM2, and AFIO *very important!*
	RCC->APB2PCENR |= RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC;
	RCC->APB1PCENR |= RCC_APB1Periph_TIM2;

	AFIO->PCFR1 |= AFIO_PCFR1_TIM2_REMAP_PARTIALREMAP1; //set partial remap mode 1

	// PC2 is T2CH1_, Input w/ Pullup/down
	GPIOC->CFGLR &= ~(0xf<<(4*2)); //clear old values
	GPIOC->CFGLR |= (GPIO_CNF_IN_PUPD)<<(4*2); //set new ones
	//1 = pull-up, 0 = pull-down
	GPIOC->OUTDR |= 1<<2;

	// PC5 is T2CH2_, Input w/ Pullup/down
	GPIOC->CFGLR &= ~(0xf<<(4*5)); //clear values
	GPIOC->CFGLR |= (GPIO_CNF_IN_PUPD)<<(4*5); //set new ones
	//1 = pull-up, 0 = pull-down
	GPIOC->OUTDR |= 1<<5;
	
	// Reset TIM2 to init all regs
	RCC->APB1PRSTR |= RCC_APB1Periph_TIM2;
	RCC->APB1PRSTR &= ~RCC_APB1Periph_TIM2;
	
	// set TIM2 clock prescaler If you want to reduce the resolution of the encoder
	//TIM2->PSC = 0x0000;

	// set a automatic reload if you want the counter to wrap earlier than 0xffff
	//TIM2->ATRLR = 0xffff;

	// SMCFGR: set encoder mode SMS=011b
	TIM2->SMCFGR |= TIM_EncoderMode_TI12;

	// initialize timer
	TIM2->SWEVGR |= TIM_UG;

	// set count to about mid-scale to avoid wrap-around
	TIM2->CNT = 0x8fff;

	// Enable TIM2
	TIM2->CTLR1 |= TIM_CEN;
};

/*****************************************************************************************
 * entry
 *****************************************************************************************/
int main()
{
	SystemInit();

    while( !DebugPrintfBufferFree() );


	Delay_Ms( 100 );
	t2encoder_init();

    GPIO_port_enable(GPIO_port_C);
	uint16_t initial_count = TIM2->CNT;
	uint16_t last_count = TIM2->CNT;
	while(1)
	{
        int i;
        for( i = 0; i < 12; i++ )
        {
            test_led(i);




           //GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), CHARLIE_X1(CHARLIEPLEX_IO), GPIO_Speed_10MHz);

          // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            
		    //GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), CHARLIEPLEX_LED(CHARLIE_X1) );
            //GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low );
            Delay_Ms( 100 );
        }
		uint16_t count = TIM2->CNT;
		if( count != last_count) {
			printf("Position relative=%ld absolute=%u delta=%ld\n",(int32_t)count - initial_count, count, (int32_t)count-last_count);
			last_count = count;
		}
		Delay_Ms(50);
	}
}