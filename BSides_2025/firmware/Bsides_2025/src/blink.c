#include "ch32fun.h"
#include <stdio.h>


int main()
{
	SystemInit();

	// Enable GPIOs
	funGpioInitAll();
	


	while(1)
	{
	       
	
	        funPinMode(PC7, GPIO_Speed_10MHz| GPIO_CNF_IN_FLOATING);
                funPinMode(PC6, GPIO_Speed_10MHz| GPIO_CNF_IN_FLOATING);
                funPinMode(PC5,     GPIO_Speed_10MHz | GPIO_CNF_OUT_PP );
                funPinMode(PC4,     GPIO_Speed_10MHz | GPIO_CNF_OUT_PP );
                Delay_Ms( 10 );
		//funDigitalWrite( PC7,     FUN_LOW );
		//funDigitalWrite( PC6,     FUN_LOW );
		funDigitalWrite( PC5,   FUN_HIGH );
		funDigitalWrite( PC4,   FUN_LOW );
		Delay_Ms( 500 );
		//funDigitalWrite( PC7,     FUN_LOW );
		//funDigitalWrite( PC6,     FUN_LOW );
		funDigitalWrite( PC5,   FUN_LOW );
		funDigitalWrite( PC4,   FUN_LOW );
		
		
		
	        funPinMode(PC7, GPIO_Speed_10MHz| GPIO_CNF_IN_FLOATING);
                funPinMode(PC6, GPIO_Speed_10MHz| GPIO_CNF_OUT_PP);
                funPinMode(PC5,     GPIO_Speed_10MHz | GPIO_CNF_IN_FLOATING );
                funPinMode(PC4,     GPIO_Speed_10MHz | GPIO_CNF_OUT_PP );
                Delay_Ms( 10 );
		//funDigitalWrite( PC7,     FUN_LOW );
		funDigitalWrite( PC6,     FUN_HIGH );
		//funDigitalWrite( PC5,   FUN_LOW );
		funDigitalWrite( PC4,   FUN_LOW );
		Delay_Ms( 500 );
		//funDigitalWrite( PC7,     FUN_LOW );
		funDigitalWrite( PC6,     FUN_LOW );
		//funDigitalWrite( PC5,   FUN_LOW );
		funDigitalWrite( PC4,   FUN_LOW );
	}
}
