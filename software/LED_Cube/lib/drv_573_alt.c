/*
 * drv_573.c
 *
 * Created: 10.10.2016 17:26:13
 *  Author: busser.michael
 */ 

#include <avr/io.h>
#include <stdint.h>
#include <stddef.h>
#include <util/delay.h>

#include "drv_573.h"		//Registerdefinitionen
#include "cube.h"			//Datenstruktur für die LEDs


//private variables
uint8_t	volatile d573_layer = 0;								//actually shown layer

/** 
 @brief Variablen für die Adressen der Callback-Funktionen.
		Diese müssen mit den entsprechenden register-Funktionen registriert werden.
*/
drv_callback_t	d573_CB_Handler = NULL;			///@brief Adresse Callback-Routine



void drv_Register_Callback( drv_callback_t cbf ) {
	d573_CB_Handler = cbf;
}

void drv_init() {

	D573_Data_DDR	= 0xFF;						//output
	D573_Data_Port	= 0;						//all off

	D573_Clock_DDR  = 0xFF;						//output
	D573_Clock_PORT = 0;						//all off

	D573_LAYER_DDR  |= (D573_LAYER_MASK);		//PD3..PD7 as output
	D573_LAYER_OFF;								//D573_LAYER_ENABLE	= 0
	D573_LATCH_OUT_DIS;							//D573_LATCH_OE = 1
	D573_LAYER_ADR_CLEAR;						//D573_LAYER_E0 = 0,	D573_LAYER_E1 = 0,	D573_LAYER_E2 = 0
	
	drv_clearAllLatches();						//Latches löschen
}

//interne Funktion, nicht ins H-File aufnehmen
void drv_clockToLatch( uint8_t latch, uint8_t value ) {

	D573_Data_Port = value;			//Datenwort an Eingänge der D-Latches anlegen
	asm volatile ("nop");
	D573_Clock_PORT = (1<<latch);	//Clock-Leitung des betreffenden Latches aktivieren

	//asm volatile ("nop");			//
	_delay_us(1);					//

	D573_Clock_PORT = 0;			//Clock-Leitung wieder auf Low setzen

	asm volatile ("nop");			//warte, bis Signal Low-Pegel erreicht hat
	_delay_us(20);					//
	//Das Datenwort an den Eingangen der D-Latches stehen lassen
}

//Methode treibt alle 8 Ebenen an
void drv_drive() {		//call every 1 ms
	uint8_t i;

	if (d573_CB_Handler != NULL) { d573_CB_Handler(CB_DRIVE_ENTER); }		//Callback to status report
		
	D573_LAYER_OFF;			//Enable-Pin für Ebenenmultiplexer auf 0 -> alle Ebeben ausschalten
	D573_LATCH_OUT_DIS;		//Latches OE ausschalten, damit Low an Low-Side-Treiber
	D573_LAYER_ADR_CLEAR;	//Adressbits E0..E2 auf 0 setzen
	
	d573_layer = ((d573_layer+1) % CUBE_MAX_LAYERS);		//Dieser Layer soll jetzt als nächstes angezeigt werden  (0..7)

	if ((d573_layer == 0) && (d573_CB_Handler != NULL)) {	//Sync anzeigen, wenn es eine Callback-Funktion gibt
		d573_CB_Handler(CB_SYNC);							//Wird nur bei Layer 0 ausgelöst
	}
		
	//transfer the content of d573data array to the latches
	for (i=0; i<CUBE_MAX_COLS; i++) {
		drv_clockToLatch( i, cube[d573_layer][i]  );//Byte an die Eingänge der Latches anlegen

/*
		D573_Data_Port  = cube[d573_layer][i];	//Byte an die Eingänge der Latches anlegen
		asm volatile ("nop");						//	
		D573_Clock_PORT = (1<<i);					//Clock-Signal am Latch i auf H setzen
		asm volatile ("nop");						//Eine kurze Pause per NOP-Anweisung einfügen.
		asm volatile ("nop");						//Mal testen, ob das notwendig ist
		D573_Clock_PORT = 0;						//Clock-Signal wieder auf L
		asm volatile ("nop");						//
		D573_Data_Port = 0;							//Datenpins am Eingang der Latches auf L setzen   (Nötig?)
*/
	}
	D573_Data_Port = 0;				//Eingänge der D-Latches auf 0 legen  (obsolete)

	
	D573_LAYER_PORT = (D573_LAYER_PORT  | (d573_layer << D573_LAYER_E0) );		//Adresse am Ebenenmultiplexer einstellen
	D573_LATCH_OUT_EN;	//Latches aktivieren, Low-side-Treiber ein
	D573_LAYER_ON;		//High-Side-Treiber ein
	
	if (d573_CB_Handler != NULL) { d573_CB_Handler(CB_DRIVE_LEAVE); }
}




//Value in das Latch 0..7 schreiben;   entspricht for-Schleife in d573_drive()
void drv_test_setLatch( uint8_t latch, uint8_t value) {
	latch = latch % 8;

	drv_clockToLatch( latch, value );

	D573_Data_Port = 0;				//Eingänge der D-Latches auf 0 legen  (obsolete)
}


void drv_clearAllLatches() {
	D573_Data_Port = 0;
	asm volatile ("nop");						//
	D573_Clock_PORT = 0xFF;
	asm volatile ("nop");						//
	asm volatile ("nop");						//
	D573_Clock_PORT = 0;
	asm volatile ("nop");						//
	D573_Data_Port = 0;
}


void drv_test_setLayer( uint8_t layer, uint8_t onOff ) {
	D573_LAYER_OFF;								//1-aus-8-Dekoder abschalten
	if (onOff == _ON) {
		layer = layer % CUBE_MAX_LAYERS;
		D573_LAYER_ADR_CLEAR;												//Adressbits E0..E2 auf 0 setzen
		D573_LAYER_PORT = (D573_LAYER_PORT  | (layer << D573_LAYER_E0) );	//Adresse am Ebenenmultiplexer einstellen
		D573_LAYER_ON;							//1-aus-8-Dekoder einschalten
	} else {
		D573_LAYER_OFF;
		D573_LAYER_ADR_CLEAR;												//Adressbits E0..E2 auf 0 setzen
	}
	
}