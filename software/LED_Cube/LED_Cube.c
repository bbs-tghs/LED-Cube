/*
 * LED_Cube.c
 *
 * Created: 08.10.2016 11:16:52
 *  Author: busser.michael
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>		//Interrupts einbinden
#include <util/delay.h>
#include "timer.h"
#include "drv_573.h"
#include "uart.h"
#include "effect.h"
#include "cube.h"

#define TEST_DDR			DDRA
#define TEST_PORT			PORTA
#define TEST_SIG0			PA4
#define TEST_SIG1			PA3
#define TEST_SIG2			PA2

#define TEST_SIG_ON(n)		(TEST_PORT |=  (1<<n))
#define TEST_SIG_OFF(n)		(TEST_PORT &= ~(1<<n))
#define TEST_SIG_TOGGLE(n)	(TEST_PORT ^=  (1<<n))

#define TEST_ALL_OFF		(TEST_PORT &= ~( (1<<TEST_SIG0) | (1<<TEST_SIG1) | (1<<TEST_SIG2) ))
#define TEST_CONFIG			(TEST_DDR |=   ( (1<<TEST_SIG0) | (1<<TEST_SIG1) | (1<<TEST_SIG2) ))


// Baudrat der seriellen Schnittstelle
//#define UART_BAUD_RATE      38400
#define UART_BAUD_RATE      19200		//Mehr scheint der MCP221 nicht zu können





void cb_syncFromDriver( TCB_DRV573 cb ) {
	switch (cb) {
		case CB_SYNC		: TEST_SIG_ON(TEST_SIG0); asm volatile ("nop"); TEST_SIG_OFF(TEST_SIG0); break;	//Triggerimpuls für Scopc erzeugen, wenn Layer=0 ist
		case CB_DRIVE_ENTER : TEST_SIG_ON(TEST_SIG1);	break;	//Messen der Ausführungszeit der Funktion d573_drive
		case CB_DRIVE_LEAVE : TEST_SIG_OFF(TEST_SIG1);	break;	//gemessen:  30µs
	default:	;
	}	
}

//all functions, dealing with the commands via serial port
uint8_t cmd_getByte() {
	uint16_t c;
	c = uart_getc();
	if ((c & 0xFF00) == 0) {	//Byte received
		return( (uint8_t)c );
	} else {					//some error occured
		if ( c & UART_NO_DATA ) {		//no data available
			//not really an error...
		} else {
			//Report the errors
			if ( c & UART_FRAME_ERROR )  {		// Framing Error detected, i.e no stop bit detected
				uart_puts_P("UART Frame Error: ");
			}
			if ( c & UART_OVERRUN_ERROR ) {		// Overrun error
				uart_puts_P("UART Overrun Error: ");
			}
			if ( c & UART_BUFFER_OVERFLOW ) {	//Buffer overflow
				uart_puts_P("Buffer overflow error: ");
			}
		}
		return( 0 );	//Ok, because we do not use value 0 here
	}
}

uint8_t cmd_getNextByte(){
	uint16_t	c;
	do {									//wait for the next byte to receive
		c = uart_getc();					//blocking call
	} while ((c & 0xFF00) == UART_NO_DATA);	//returns after the next character is received
	return( (uint8_t)c );					//actually without any timeout
}


void cmd_Command() {		//Use hex-Mode in hterm for testing
	uint8_t	c;

	c = cmd_getByte();
	switch (c) {
		case 0x3F:	// ?
					uart_puts( "LED-Cube V.01\n" );		//Platzhalter...
					break;
		case 0x68:	// H - Halt
					pattern_stop();
					cube_clear();
					uart_puts( "halted\n" ); 
					break;
					
		case 0x78:	// x - Execute
					pattern_start( P_RUNNING_POINT );
					uart_puts( "started\n" );
					break;
					
		case 0x31:	
					break;			
		default: ;	//including no command
	}
}




int main(void)
{	
	
	_delay_ms(150);
	drv_init();
	drv_Register_Callback( &cb_syncFromDriver );
	
	
	timerInit();				//den Hardware-Timer initialisieren und starten
	uart_init( UART_BAUD_SELECT_DOUBLE_SPEED(UART_BAUD_RATE,F_CPU) );

	sei();
	uart_puts( "LED-Cube V.01\n" );
	
	TEST_CONFIG;
	TEST_ALL_OFF;
	
	pattern_start( P_RUNNING_POINT );		//Testweise starten		P_RUNNING_POINT
//	pattern_start( P_RAIN );				//Testweise starten		P_RUNNING_POINT
	
	
    while(1)
    {

		//Eingabe:
		cmd_Command();		//handle commands received via serial port
		
		
		if (timerFlags.flags.bMilli) {
			drv_drive();					//Multiplexing der Layer erzeugen und Daten eines Layers in die Latches schreiben
			timerClearMilli();
		}
        
		
		if (timerFlags.flags.bCenti) {
			// alle 10 ms
			pattern_do();
			timerClearCenti();
		}
	
	
		if (timerFlags.flags.bDezi) {
			// alle 100 ms
			timerClearDezi();
		}
		
		
    }
}


