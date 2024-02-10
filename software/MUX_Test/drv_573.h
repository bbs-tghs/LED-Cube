/*
 * drv_573.h
 *
 * Created: 10.10.2016 17:26:30
 *  Author: busser.michael
 * 
 * Drive a number of 74*573-Latches
 * 
 * Hardware-depended stuff
 *
 * HW_Rev_1:	Die erste Version, bestehend aus mehreren Platinen
 * HW_Rev_2:	die zweite Version, diesmal alles auf einer Platine: Controller, Multiplexer, Ebenentreiber
 *
 *
 */ 


#ifndef DRV_573_H_
#define DRV_573_H_

#define HW_Rev_2	

#define D573_Data_DDR			DDRB		//Datenbits für die Latches
#define D573_Data_Port			PORTB

#define D573_Clock_DDR			DDRC		//Strobe-Impulse für die Latches
#define D573_Clock_PORT			PORTC

#define D573_LAYER_DDR			DDRD		//Layer
#define D573_LAYER_PORT			PORTD

#define D573_LATCH_OE			PD3
#define D573_LAYER_E0			PD4
#define D573_LAYER_E1			PD5
#define D573_LAYER_E2			PD6
#define D573_LAYER_ENABLE		PD7

#define D573_LAYER_MASK			((1<<D573_LAYER_E0)|(1<<D573_LAYER_E1)|(1<<D573_LAYER_E2)|(1<<D573_LAYER_ENABLE)|(1<<D573_LATCH_OE))
#define D573_LAYER_ADR			((1<<D573_LAYER_E0)|(1<<D573_LAYER_E1)|(1<<D573_LAYER_E2))

#define D573_LAYER_0			0															//Adress Layer 0
#define D573_LAYER_1			((1<<D573_LAYER_E0))										//Adress Layer 1
#define D573_LAYER_2			((1<<D573_LAYER_E1))										//Adress Layer 2
#define D573_LAYER_3			((1<<D573_LAYER_E0)|(1<<D573_LAYER_E1))						//Adress Layer 3
#define D573_LAYER_4			((1<<D573_LAYER_E2))										//Adress Layer 4
#define D573_LAYER_5			((1<<D573_LAYER_E0)|(1<<D573_LAYER_E2))						//Adress Layer 5
#define D573_LAYER_6			((1<<D573_LAYER_E1)|(1<<D573_LAYER_E2))						//Adress Layer 6	
#define D573_LAYER_7			((1<<D573_LAYER_E0)|(1<<D573_LAYER_E1)|(1<<D573_LAYER_E2))	//Adress Layer 7

#define _ON						1
#define _OFF					0

#define D573_LAYER_ON			(D573_LAYER_PORT |=  (1<<D573_LAYER_ENABLE))		//Switch on  Layers globally
#define D573_LAYER_OFF			(D573_LAYER_PORT &= ~(1<<D573_LAYER_ENABLE))		//switch off Layers globally

#define D573_LAYER_ADR_CLEAR	(D573_LAYER_PORT &= ~(D573_LAYER_ADR))				//E0..E2 set to L

#define D573_LATCH_OUT_EN		(D573_LAYER_PORT &= ~(1<<D573_LATCH_OE))			//Enable  Latch output
#define D573_LATCH_OUT_DIS		(D573_LAYER_PORT |=  (1<<D573_LATCH_OE))			//Disable Latch output

#define D573_MIRRORMASK			0b11110000	//Verdahhtungsfehler beim anschluss der LEDs korrigieren

typedef enum {
	CB_SYNC,
	CB_DRIVE_ENTER,
	CB_DRIVE_LEAVE,
	CB_NEW_LAYER 
	} TCB_DRV573;


typedef void (*drv_callback_t) ( TCB_DRV573 cb );		//Event Callback-Funktion, es werden keine Parameter übergeben



void drv_init();			//Init driver: set required in/output pins
void drv_drive();			//call every 1 ms for multiplexing the cube
void drv_clearAllLatches();	//Clear all latches with max. speed

void drv_Register_Callback( drv_callback_t cbf );

//Testfunktionen
void drv_test_setLatch( uint8_t latch, uint8_t value);
void drv_test_setLayer( uint8_t layer, uint8_t onOff );



#endif /* DRV_573_H_ */