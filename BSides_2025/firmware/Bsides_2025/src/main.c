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

//#include "ch32v003_GPIO_branchless.h"
//#include "ch32v003fun.h" 

#include <stdio.h>


#define I2C_ADDR 0x68

#define MAX_ENCODER_COUNT 0x00ff

uint32_t count;
uint32_t ROT_A;
uint32_t ROT_B;
uint32_t ROT_SW;
uint16_t last_led;

uint8_t encoderClicked = 0;
int encoderClicks = 0;

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

//#define CHARLIE_X1_PINMODE(pin_i) GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), CHARLIE_X1, GPIO_Speed_10MHz);
//GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), IO_X1, GPIO_Speed_10MHz);

void charlie_led(int led)
{
    if(led != last_led){
        funDigitalWrite( PC7, FUN_LOW );
        funDigitalWrite( PC6, FUN_LOW );
        funDigitalWrite( PC5, FUN_LOW );
        funDigitalWrite( PC4, FUN_LOW );
        last_led = led;
    }
    switch(led)
    {
        case 0:
            funPinMode( PC7, GPIO_CFGLR_IN_FLOAT ); // GPIO_CFGLR_IN_FLOAT or GPIO_CFGLR_OUT_2Mhz_PP
            funPinMode( PC6, GPIO_CFGLR_IN_FLOAT );
            funPinMode( PC5, GPIO_CFGLR_OUT_2Mhz_PP );
            funPinMode( PC4, GPIO_CFGLR_OUT_2Mhz_PP );

        
            ///GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            //GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            //GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            //GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
          //  GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), low);
           // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), low);
            //GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), high);
            //GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low);
            funDigitalWrite( PC5, FUN_HIGH );
            funDigitalWrite( PC4, FUN_LOW );
            break;
        case 1:
            funPinMode( PC7, GPIO_CFGLR_IN_FLOAT ); // GPIO_CFGLR_IN_FLOAT or GPIO_CFGLR_OUT_2Mhz_PP
            funPinMode( PC6, GPIO_CFGLR_OUT_2Mhz_PP );
            funPinMode( PC5, GPIO_CFGLR_IN_FLOAT );
            funPinMode( PC4, GPIO_CFGLR_OUT_2Mhz_PP );
            //GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            //GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            //GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            //GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            //GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low);
            //GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
            //GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), high);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), low);
            funDigitalWrite( PC6, FUN_HIGH );
            funDigitalWrite( PC4, FUN_LOW );
            break;
        case 2:
            funPinMode( PC7, GPIO_CFGLR_OUT_2Mhz_PP ); // GPIO_CFGLR_IN_FLOAT or GPIO_CFGLR_OUT_2Mhz_PP
            funPinMode( PC6, GPIO_CFGLR_IN_FLOAT );
            funPinMode( PC5, GPIO_CFGLR_IN_FLOAT );
            funPinMode( PC4, GPIO_CFGLR_OUT_2Mhz_PP );
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), low);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), high);
            funDigitalWrite( PC7, FUN_HIGH );
            funDigitalWrite( PC4, FUN_LOW );
            break;
        case 3:
            funPinMode( PC7, GPIO_CFGLR_IN_FLOAT ); // GPIO_CFGLR_IN_FLOAT or GPIO_CFGLR_OUT_2Mhz_PP
            funPinMode( PC6, GPIO_CFGLR_IN_FLOAT );
            funPinMode( PC5, GPIO_CFGLR_OUT_2Mhz_PP );
            funPinMode( PC4, GPIO_CFGLR_OUT_2Mhz_PP );
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), high);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), low);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), low);
            funDigitalWrite( PC5, FUN_LOW );
            funDigitalWrite( PC4, FUN_HIGH );
            break;
        case 4:
            funPinMode( PC7, GPIO_CFGLR_IN_FLOAT ); // GPIO_CFGLR_IN_FLOAT or GPIO_CFGLR_OUT_2Mhz_PP
            funPinMode( PC6, GPIO_CFGLR_OUT_2Mhz_PP );
            funPinMode( PC5, GPIO_CFGLR_OUT_2Mhz_PP );
            funPinMode( PC4, GPIO_CFGLR_IN_FLOAT );
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), high);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), low);
            funDigitalWrite( PC6, FUN_HIGH );
            funDigitalWrite( PC5, FUN_LOW );
            break;
        case 5:
            funPinMode( PC7, GPIO_CFGLR_OUT_2Mhz_PP ); // GPIO_CFGLR_IN_FLOAT or GPIO_CFGLR_OUT_2Mhz_PP
            funPinMode( PC6, GPIO_CFGLR_IN_FLOAT );
            funPinMode( PC5, GPIO_CFGLR_OUT_2Mhz_PP );
            funPinMode( PC4, GPIO_CFGLR_IN_FLOAT );
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), low);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), high);
            funDigitalWrite( PC7, FUN_HIGH );
            funDigitalWrite( PC5, FUN_LOW );
            break;
        case 6:
            funPinMode( PC7, GPIO_CFGLR_IN_FLOAT ); // GPIO_CFGLR_IN_FLOAT or GPIO_CFGLR_OUT_2Mhz_PP
            funPinMode( PC6, GPIO_CFGLR_OUT_2Mhz_PP );
            funPinMode( PC5, GPIO_CFGLR_IN_FLOAT );
            funPinMode( PC4, GPIO_CFGLR_OUT_2Mhz_PP );
        //     GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
        //     GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
        //     GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
        //     GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
        //     GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), high);
        //     GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
        //     GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), low);
        //     GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), low);
            funDigitalWrite( PC6, FUN_LOW );
            funDigitalWrite( PC4, FUN_HIGH );
            break;
        case 7:
            funPinMode( PC7, GPIO_CFGLR_IN_FLOAT ); // GPIO_CFGLR_IN_FLOAT or GPIO_CFGLR_OUT_2Mhz_PP
            funPinMode( PC6, GPIO_CFGLR_OUT_2Mhz_PP);
            funPinMode( PC5, GPIO_CFGLR_OUT_2Mhz_PP );
            funPinMode( PC4, GPIO_CFGLR_IN_FLOAT );
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), low);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), high);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), low);
            funDigitalWrite( PC5, FUN_HIGH );
            funDigitalWrite( PC6, FUN_LOW );
            break;
        case 8:
            funPinMode( PC7, GPIO_CFGLR_OUT_2Mhz_PP ); // GPIO_CFGLR_IN_FLOAT or GPIO_CFGLR_OUT_2Mhz_PP
            funPinMode( PC6, GPIO_CFGLR_OUT_2Mhz_PP );
            funPinMode( PC5, GPIO_CFGLR_IN_FLOAT );
            funPinMode( PC4, GPIO_CFGLR_IN_FLOAT );
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), high);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), low);
            funDigitalWrite( PC6, FUN_LOW );
            funDigitalWrite( PC7, FUN_HIGH );
            break;
        case 9:
            funPinMode( PC7, GPIO_CFGLR_OUT_2Mhz_PP ); // GPIO_CFGLR_IN_FLOAT or GPIO_CFGLR_OUT_2Mhz_PP
            funPinMode( PC6, GPIO_CFGLR_IN_FLOAT );
            funPinMode( PC5, GPIO_CFGLR_IN_FLOAT );
            funPinMode( PC4, GPIO_CFGLR_OUT_2Mhz_PP );
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), low);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), high);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), low);
            funDigitalWrite( PC4, FUN_HIGH );
            funDigitalWrite( PC7, FUN_LOW );
            break;
        case 10:
            funPinMode( PC7, GPIO_CFGLR_OUT_2Mhz_PP ); // GPIO_CFGLR_IN_FLOAT or GPIO_CFGLR_OUT_2Mhz_PP
            funPinMode( PC6, GPIO_CFGLR_IN_FLOAT );
            funPinMode( PC5, GPIO_CFGLR_OUT_2Mhz_PP );
            funPinMode( PC4, GPIO_CFGLR_IN_FLOAT );
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), low);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), high);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), low);
            funDigitalWrite( PC5, FUN_HIGH );
            funDigitalWrite( PC7, FUN_LOW );
            break;
        case 11:
            funPinMode( PC7, GPIO_CFGLR_OUT_2Mhz_PP ); // GPIO_CFGLR_IN_FLOAT or GPIO_CFGLR_OUT_2Mhz_PP
            funPinMode( PC6, GPIO_CFGLR_OUT_2Mhz_PP );
            funPinMode( PC5, GPIO_CFGLR_IN_FLOAT );
            funPinMode( PC4, GPIO_CFGLR_IN_FLOAT );
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 7), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 5), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 6), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 5), low);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 6), high);
            // GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 7), low);
            funDigitalWrite( PC6, FUN_HIGH );
            funDigitalWrite( PC7, FUN_LOW );
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


void EXTI7_0_IRQHandler( void ) __attribute__((interrupt));
void EXTI7_0_IRQHandler( void )
{
    // if( EXTI->INTFR & EXTI_Line0 )
    // {
    //     ROT_A++;
    //     EXTI->INTFR = EXTI_Line0;
    // }

    if ( EXTI->INTFR & EXTI_Line0 )
    {
        encoderClicked = 1;

        if (funDigitalRead(PD2) == 1)
        {
            encoderClicks++;
        }
        else
        {
            encoderClicks--;
        }
        
        EXTI->INTFR = EXTI_Line0;
    }
     if( EXTI->INTFR & EXTI_Line2 )
     {
         ROT_B++;
         EXTI->INTFR = EXTI_Line2;
     }
     if( EXTI->INTFR & EXTI_Line3 )
     {
         ROT_SW++;
         EXTI->INTFR = EXTI_Line3;
     }
}

/******************************************************************************************
 * initialize TIM2 for PWM
 ******************************************************************************************/
void encoder_init( void )
{
    
    funPinMode( PD0, GPIO_CFGLR_IN_FLOAT );
    funPinMode( PD2, GPIO_CFGLR_IN_FLOAT );
    funPinMode( PD3, GPIO_CFGLR_IN_FLOAT );

	asm volatile(
        #if __GNUC__ > 10
                ".option arch, +zicsr\n"
        #endif
                 "addi t1, x0, 3\n"
                "csrrw x0, 0x804, t1\n"
                 : : :  "t1" );
    // Configure the IO as an interrupt.
	AFIO->EXTICR = AFIO_EXTICR_EXTI0_PD | AFIO_EXTICR_EXTI2_PD| AFIO_EXTICR_EXTI3_PD;   //AFIO_EXTICR_EXTI3_PD;
	EXTI->INTENR = EXTI_INTENR_MR0|EXTI_INTENR_MR2|EXTI_INTENR_MR3; // Enable EXT3
	EXTI->RTENR = EXTI_FTENR_TR0|EXTI_FTENR_TR2|EXTI_FTENR_TR3;// Falling edge trigger
    NVIC_EnableIRQ( EXTI7_0_IRQn );
	// Enable GPIOD, TIM1, and AFIO *very important!*
	//RCC->APB2PCENR |= RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOD | RCC_APB2Periph_TIM1;
	//RCC->APB1PCENR |= ;

	//AFIO->PCFR1 |= AFIO_PCFR1_TIM2_REMAP_PARTIALREMAP1; //set partial remap mode 1

	// PD0 is T1CH1N, Input floating
	//GPIOD->CFGLR &= ~(0xf<<(4*0)); //clear old values
	//GPIOD->CFGLR |= (GPIO_CNF_IN_FLOATING)<<(4*0); //set new ones
	//1 = pull-up, 0 = pull-down
	//GPIOD->OUTDR |= 1<<0;

	// PD2 is T2CH1N, Input floating
	//GPIOD->CFGLR &= ~(0xf<<(4*2)); //clear values
	//GPIOD->CFGLR |= (GPIO_CNF_IN_FLOATING)<<(4*2); //set new ones
	//1 = pull-up, 0 = pull-down
	//GPIOD->OUTDR |= 1<<2;
	
	// Reset TIM2 to init all regs
	//RCC->APB1PRSTR |= RCC_APB1Periph_TIM2;
	//RCC->APB1PRSTR &= ~RCC_APB1Periph_TIM2;
	
	// set TIM2 clock prescaler If you want to reduce the resolution of the encoder
	//TIM2->PSC = 0x0000;

	// set a automatic reload if you want the counter to wrap earlier than 0xffff
	//TIM2->ATRLR = 0xffff;

	// SMCFGR: set encoder mode SMS=011b
	//TIM2->SMCFGR |= TIM_EncoderMode_TI12;

	// initialize timer
	//TIM2->SWEVGR |= TIM_UG;

	// set count to about mid-scale to avoid wrap-around
	//TIM2->CNT = 0x8fff;

	// Enable TIM2
	//TIM2->CTLR1 |= TIM_CEN;
};

int check_rst();
/*****************************************************************************************
 * entry
 *****************************************************************************************/
int main()
{
	//SystemInit();
    check_rst();
   	// Enable GPIOs
	funGpioInitAll();
    while( !DebugPrintfBufferFree() );

    printf("hello world");
	Delay_Ms( 100 );
	encoder_init();
    int i;
    for( i = 0; i < 12; i++ )
    {
        charlie_led(i%12);




       //GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), CHARLIE_X1(CHARLIEPLEX_IO), GPIO_Speed_10MHz);

      // GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
        
        //GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), CHARLIEPLEX_LED(CHARLIE_X1) );
        //GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low );
        Delay_Ms( 100 );
    }
    //GPIO_port_enable(GPIO_port_C);
	//uint16_t initial_count = TIM2->CNT;
	uint16_t last_count = 0;
	while(1)
	{

            charlie_led(encoderClicks%12);



            Delay_Ms( 100 );

        uint16_t count = ROT_A-ROT_B;
		//uint16_t count = TIM2->CNT;
		if( count != last_count) {
			printf("Position relative=%ld absolute=%ld delta=%ld\n",(int32_t) count, (int32_t)ROT_A+ROT_B, (int32_t)count-last_count);
			last_count = count;
		}
        if(encoderClicked == 1)
        {
            encoderClicked = 0; 
            printf("Encoder value: %d\r\n", encoderClicks);
        }
		Delay_Ms(50);
	}
}

/* This shows how to use the option bytes.  I.e. how do you disable NRST?
   WARNING Portions of this code are under the following copyright.
*/
/********************************** (C) COPYRIGHT  *******************************
 * File Name          : ch32v00x_flash.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/08/08
 * Description        : This file provides all the FLASH firmware functions.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *****
 */
 int FLASH_WaitForLastOperation(uint32_t Timeout);
 
 int check_rst()
 {

    /* Notes from flash document:
    * @param   OB_IWDG - Selects the IWDG mode
    *            OB_IWDG_SW - Software IWDG selected
    *            OB_IWDG_HW - Hardware IWDG selected
    *          OB_STOP - Reset event when entering STOP mode.
    *            OB_STOP_NoRST - No reset generated when entering in STOP
    *            OB_STOP_RST - Reset generated when entering in STOP
    *          OB_STDBY - Reset event when entering Standby mode.
    *            OB_STDBY_NoRST - No reset generated when entering in STANDBY
    *            OB_STDBY_RST - Reset generated when entering in STANDBY
    *          OB_RST - Selects the reset IO mode and Ignore delay time
    *            OB_RST_NoEN - Reset IO disable (PD7)
    *            OB_RST_EN_DT12ms - Reset IO enable (PD7) and  Ignore delay time 12ms
    *            OB_RST_EN_DT1ms - Reset IO enable (PD7) and  Ignore delay time 1ms
    *            OB_RST_EN_DT128ms - Reset IO enable (PD7) and  Ignore delay time 128ms
    *          OB_BOOT - Selects bootloader or usercode for on start.
    *            OB_STARTMODE_USER - Boot directly to flash
    *            OB_STARTMODE_BOOT - Boot to bootloader
    */
    uint16_t OB_STOP = OB_STOP_NoRST;
    uint16_t OB_IWDG = OB_IWDG_SW;
    uint16_t OB_STDBY = OB_STDBY_NoRST;
    uint16_t OB_RST = OB_RST_NoEN;
    uint16_t OB_BOOT = OB_STARTMODE_USER;

    printf( "OB->USER = %08x\n", OB->USER);
    if( OB->USER == (OB_BOOT | OB_IWDG | (uint16_t)(OB_STOP | (uint16_t)(OB_STDBY | (uint16_t)(OB_RST | (uint16_t)0xc0)))) )
    {
        printf( "Option bytes already set\n" );
        return 1;
    }
    printf( "Option bytes not set\n" );
 



     Delay_Ms( 100 );
 	// For option bytes.
     FLASH->OBKEYR = FLASH_KEY1;
     FLASH->OBKEYR = FLASH_KEY2;
     
	// Unkock flash - be aware you need extra stuff for the bootloader.
     FLASH->KEYR = FLASH_KEY1;
     FLASH->KEYR = FLASH_KEY2;

    // For unlocking programming, in general.
     FLASH->MODEKEYR = FLASH_KEY1;
     FLASH->MODEKEYR = FLASH_KEY2;
     
	printf( "FLASH->CTLR = %08lx\n", FLASH->CTLR );
	if( FLASH->CTLR & 0x8080 ) 
	{
		printf( "Flash still locked\n" );
		while(1);
	}
 
     printf( "Option bytes started as:%04x\n", OB->USER );
 
    


     uint16_t rdptmp = RDP_Key;
 
 
     int status = FLASH_WaitForLastOperation(EraseTimeout);
     if(status == FLASH_COMPLETE)
     {
         FLASH->OBKEYR = FLASH_KEY1;
         FLASH->OBKEYR = FLASH_KEY2;
 
         FLASH->CTLR |= CR_OPTER_Set;
         FLASH->CTLR |= CR_STRT_Set;
         status = FLASH_WaitForLastOperation(EraseTimeout);
 
         if(status == FLASH_COMPLETE)
         {
             FLASH->CTLR &= CR_OPTER_Reset;
             FLASH->CTLR |= CR_OPTPG_Set;
             OB->RDPR = (uint16_t)rdptmp;
             status = FLASH_WaitForLastOperation(ProgramTimeout);
 
             if(status != FLASH_TIMEOUT)
             {
                 FLASH->CTLR &= CR_OPTPG_Reset;
             }
         }
         else
         {
             if(status != FLASH_TIMEOUT)
             {
                 FLASH->CTLR &= CR_OPTPG_Reset;
             }
         }
     }
 
 
     printf( "After Clear:%04x\n", OB->USER );
 
     FLASH->OBKEYR = FLASH_KEY1;
     FLASH->OBKEYR = FLASH_KEY2;
     status = FLASH_WaitForLastOperation(10000);
 
     if(status == FLASH_COMPLETE)
     {
         FLASH->CTLR |= CR_OPTPG_Set;
         OB->USER = OB_BOOT | OB_IWDG | (uint16_t)(OB_STOP | (uint16_t)(OB_STDBY | (uint16_t)(OB_RST | (uint16_t)0xc0)));
 
         status = FLASH_WaitForLastOperation(10000);
         if(status != FLASH_TIMEOUT)
         {
             FLASH->CTLR &= CR_OPTPG_Reset;
         }
     }
 
     printf( "After Write:%04x\n", OB->USER );
 
     return 0;
 }
 
 
 int FLASH_GetBank1Status(void)
 {
     int flashstatus = FLASH_COMPLETE;
 
     if((FLASH->STATR & FLASH_FLAG_BANK1_BSY) == FLASH_FLAG_BSY)
     {
         flashstatus = FLASH_BUSY;
     }
     else
     {
         if((FLASH->STATR & FLASH_FLAG_BANK1_WRPRTERR) != 0)
         {
             flashstatus = FLASH_ERROR_WRP;
         }
         else
         {
             flashstatus = FLASH_COMPLETE;
         }
     }
     return flashstatus;
 }
 
 
 int FLASH_WaitForLastOperation(uint32_t Timeout)
 {
     int status = FLASH_COMPLETE;
 
     status = FLASH_GetBank1Status();
     while((status == FLASH_BUSY) && (Timeout != 0x00))
     {
         status = FLASH_GetBank1Status();
         Timeout--;
     }
     if(Timeout == 0x00)
     {
         status = FLASH_TIMEOUT;
     }
     return status;
 }