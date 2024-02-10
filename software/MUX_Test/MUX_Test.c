/*
 * MUX_Test.c
 *
 * Created: 25.12.2016 09:57:58
 *  Author: busser.michael
 *
 *
 *
 *
 *
 *
 *
 *
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>		//Interrupts einbinden
#include <util/delay.h>
#include "timer.h"
#include "drv_573.h"		//Test-purposes
#include "cube.h"
#include "uart.h"

/*
#define TEST_DDR			DDRB
#define TEST_PORT			PORTB
#define TEST_SIG0			PB0
#define TEST_SIG1			PB1
#define TEST_SIG2			PB2

#define TEST_SIG_ON(n)		(TEST_PORT |=  (1<<n))
#define TEST_SIG_OFF(n)		(TEST_PORT &= ~(1<<n))
#define TEST_SIG_TOGGLE(n)	(TEST_PORT ^=  (1<<n))

#define TEST_ALL_OFF		(TEST_PORT &= ~( (1<<TEST_SIG0) | (1<<TEST_SIG1) | (1<<TEST_SIG2) ))
#define TEST_CONFIG			(TEST_DDR |=   ( (1<<TEST_SIG0) | (1<<TEST_SIG1) | (1<<TEST_SIG2) ))
*/

// Baudrat der seriellen Schnittstelle
#define UART_BAUD_RATE      19200


//globale Variablen

uint8_t	test  = 0;		//0 = kein Test
						//1 = schaltet Layer layer ein- und aus

uint8_t	layer = 0;		//default: Layer 0
uint8_t latch = 0;
uint8_t	col	  = 0;
uint8_t	row	  = 0;
uint8_t	data  = 0;		//je nach Testfall

uint8_t	multiplexing = 0;	

uint8_t pattern = 0;

char asHex( uint8_t x ) {
	x = x & 0x0F;
	if ( x > 9 ) {
		return( (x-10) + 'A' );
	} else {
		return( x + '0' );
	}
}

//Rotate Right
uint8_t ror( uint8_t x ) {
	if (x & 0x01) {	//niederwertigstes Bit gesetzt
		return( (x >> 1) | 0x80 );
	} else {
		return( (x >> 1) );
	}
}

//Rotate Left
uint8_t rol( uint8_t x ) {
	if (x & 0x80) {	//höchstwertigstes Bit gesetzt
		return( (x << 1) | 0x01 );
	} else {
		return( (x << 1) );
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
	uint8_t	c, b, a, x,y,z;

	c = cmd_getByte();
	switch (c) {
		//- Kommanods für den Test der D-FF
		case 0x63:	//'c'	Clear = Alle Latches mit 0 beschreiben
					drv_clearAllLatches();
					uart_puts( "cleared\n\0" );
					break;			
					
		case 0x64:	//'d'	set Value to D-Latch
					a = cmd_getNextByte();		//welches Latch?
					b = cmd_getNextByte();		//welcher Wert?
					latch = a % 8;
					data  = b;
					drv_test_setLatch( latch, data );
					uart_puts( "dff=" );  uart_putc( latch );  uart_puts( " set to " ); uart_putc( data ); uart_putc( '\n' );
					break;
		
		case 0x65:	a = cmd_getNextByte();
					if (a == 0) {
						D573_LATCH_OUT_DIS;  	uart_puts( "OC=1\n" );
					} else {
						D573_LATCH_OUT_EN;		uart_puts( "OC=0\n" );
					}
					break;		
		
		case 0x66 :	// High-Nibble = 0..7 (Layer)		Low Nibble = On/Off (0|1)
					a =  cmd_getNextByte();		//welcher Layer? 
					b = a & 1;					//Bitweises UND  -On/Off
					layer = (a >> 4) & 0x0F;	//Nr des Layers
					if (b > 0) {
						drv_test_setLayer( layer, _ON );
					} else {
						drv_test_setLayer( layer, _OFF );
					}
					uart_puts( "layer=\0" );  uart_putc( layer );	uart_putc( '\n' );
					break;		
		

		//- Das Array kommt ins Spiel...
		case 0x3F:	// '?'	Die akt. Testfall-Nr ausgeben
					uart_puts( "tc=\0" );  uart_putc( test );	uart_putc( '\n' );
					break;
					
		case 0x40:  // '@'
					for (int lay=0; lay<CUBE_MAX_LAYERS; lay++) {
						for (int col=0; col<CUBE_MAX_COLS; col++) {
							a = cube_getData( lay, col );	
							uart_putc( asHex(a>>4) );	uart_putc( asHex(a) ); uart_putc( ' ' );
						}
						uart_putc( '\n' );
					}
					uart_putc( '\n' );
					break;

		case 0x62:	//'c'	Clear Array
					cube_clear();
					uart_puts( "LED-Array cleared\n" );
					break;

		case 0x68:	//'h'	set Value to LED-Bitarray
					a = cmd_getNextByte();		//welches Layer  und Col
					b = cmd_getNextByte();		//welcher Wert?
					layer = (a >> 4) & 0x0F;
					col   = a & 0x0F;
					data  = b;
					cube_setData( layer, col, data );
					uart_puts( "set " ); uart_putc( layer );uart_putc( ':' );uart_putc( col );uart_putc( ':' );uart_putc( data );uart_putc( '\n' );
					break;		

		case 0x6d:	//'m'	Multiplexing ein oder ausschalten
					a = cmd_getNextByte();
					switch (a) {
						case 0:	multiplexing = 0;	break;
						case 1: multiplexing = 1;	break;
						default: ;
					}
					uart_puts( "mux=" );  uart_putc( multiplexing ); uart_putc( '\n' );
					break;

					
		case 0x74:	//'t' Select test case
					test = cmd_getNextByte();		//welcher Testfall?
					if (test == 0) { cube_clear(); }
					if (test == 0x20 ) { pattern = cmd_getNextByte(); } 	
					uart_puts( "tc=\0" );  uart_putc( test );	uart_putc( '\n' );
					break;

					
		case 0x72:	//'r'	Clear voxel
					x = cmd_getNextByte();
					y = cmd_getNextByte();
					z = cmd_getNextByte();
					cube_clearVoxel( x, y, z );	
					break;
		case 0x73:	//'s'	Set voxel
					x = cmd_getNextByte();
					y = cmd_getNextByte();
					z = cmd_getNextByte();
					cube_setVoxel( x, y, z );
					break;					
		case 0x75:	//'u'	flip voxel
					x = cmd_getNextByte();
					y = cmd_getNextByte();
					z = cmd_getNextByte();
					cube_flipVoxel( x, y, z );
					break;
					
					
		default: ;	//including no command
	}
}



int main(void)
{	uint8_t x1=0,x2=0,y1=0,y2=0,z1=0,z2=0;
		
	_delay_ms(150);
	drv_init();	
	
	
	drv_test_setLayer( 0, _OFF );
	
	timerInit();				//den Hardware-Timer initialisieren und starten
	uart_init( UART_BAUD_SELECT_DOUBLE_SPEED(UART_BAUD_RATE,F_CPU) );

	sei();
	uart_puts( "Test LED-Cube V.01\n\0" );
    while(1)
    {
		
		//Eingabe:
		cmd_Command();		//handle commands received via serial port

		if (timerFlags.flags.bMilli) {
			if (multiplexing > 0) {
				drv_drive();					//Multiplexing der Layer erzeugen und Daten eines Layers in die Latches schreiben
			}
			timerClearMilli();
		}
		
		
		switch (test) {
			case 0:	break;								//no test case

			case 1: layer = 0;							//test case 1, first step
					col = 0;	
					data=0;					
					test++;
					break;

			case 2: if (timerFlags.flags.bDezi) {		//test case 1, main step
						if (data == 0xFF) { 
							data = 0;
							cube_setCol( layer, col, data ); 
							col++;
							if (col >= CUBE_MAX_COLS) {
								col = 0;
								layer++;
								if (layer >= CUBE_MAX_LAYERS) { layer = 0; }
							}
						}							
						data = (data << 1) | 1;
						cube_setCol( layer, col, data );
						timerClearDezi();
					}
					break;
					

			case 4: layer = 0;							//test case 4, first step
					row = 0;	
					data=0;
					test++;
					break;
					
			case 5: if (timerFlags.flags.bDezi) {		//test case 4, main step
						if (data == 0xFF) {
							data = 0;
							cube_setRow( layer, row, data );
							row++;
							if (row >= CUBE_MAX_ROWS) {
								row = 0;
								layer++;
								if (layer >= CUBE_MAX_LAYERS) { layer = 0; }
							}
						}
						data = (data << 1) | 1;
						cube_setRow( layer, row, data );
						timerClearDezi();
					}
					break;



			case 10: layer = 0;							//test case 0A, first step: running a single bit throuh the cube
 					 col = 0;	data=0;
					 test=11;
					 break;
			case 11:	//test case 0A, main step:
					if (timerFlags.flags.bDezi) {
						if (data == 0) {
							data = 1;
							cube_setData( layer, col, data );
						} else {
							data = data << 1;
							cube_setData( layer, col, data );

							if (data == 0) {
								col++;
								if (col > 7) {
									col = 0;
									layer++;
									if (layer > 7) { layer = 0; }
								}
							}
						}
						timerClearDezi();
					}
					break;
					
			case 16://0x10
					data = 0;
					test++;
					break;
			case 17:if (timerFlags.flags.bDezi) {
						cube_clearPlaneX( data );
						data++;
						if (data >= CUBE_MAX_X) { data = 0; }
						cube_setPlaneX( data );						
						timerClearDezi();
					}
					break;
					
			case 18://0x12
					data = 0;
					test++;
					break;
			case 19:if (timerFlags.flags.bDezi) {
						cube_clearPlaneY( data );
						data++;
						if (data >= CUBE_MAX_Y) { data = 0; }
						cube_setPlaneY( data );
						timerClearDezi();
					}
					break;

			case 20://0x14
					data = 0;
					test++;
					break;
					
			case 21:if (timerFlags.flags.bDezi) {
						cube_clearPlaneZ( data );
						data++;
						if (data >= CUBE_MAX_Z) { data = 0; }
						cube_setPlaneZ( data );
						timerClearDezi();
					}
					break;
					
			case 32://0x32
					data=0; 
					pattern %= 3;
					test++;
					break;
			case 33:if (timerFlags.flags.bDezi) {
						switch (data) {
							case 0:	cube_clear();
									x1=3; x2=4; 	y1=3; y2=4;		z1=3;	z2=4;
									data++;
									break; 
							case 1: cube_clear();
									switch(pattern) {						//3-4
										case 0:	cube_Filled( x1, y1, z1, x2, y2, z2 );	break;
										case 1:	 cube_Walls( x1, y1, z1, x2, y2, z2 );	break;
										default: cube_Edges( x1, y1, z1, x2, y2, z2 );	break;
									}
									x1--;	y1--;	z1--;
									x2++;	y2++;	z2++;
									data++;
									break;
							case 2: cube_clear();
									switch(pattern) {						//2-5
										case 0:	cube_Filled( x1, y1, z1, x2, y2, z2 );	break;
										case 1:	 cube_Walls( x1, y1, z1, x2, y2, z2 );	break;
										default: cube_Edges( x1, y1, z1, x2, y2, z2 );	break;
									}
									x1--;	y1--;	z1--;
									x2++;	y2++;	z2++;
									data++;
									break;
							case 3: cube_clear();
									switch(pattern) {						//1-6
										case 0:	cube_Filled( x1, y1, z1, x2, y2, z2 );	break;
										case 1:	 cube_Walls( x1, y1, z1, x2, y2, z2 );	break;
										default: cube_Edges( x1, y1, z1, x2, y2, z2 );	break;
									}
									x1--;	y1--;	z1--;
									x2++;	y2++;	z2++;
									data++;
									break;
							case 4: cube_clear();
									switch(pattern) {						//0-7
										case 0:	cube_Filled( x1, y1, z1, x2, y2, z2 );	break;
										case 1:	 cube_Walls( x1, y1, z1, x2, y2, z2 );	break;
										default: cube_Edges( x1, y1, z1, x2, y2, z2 );	break;
									}
									x1++;	y1++;	z1++;
									x2--;	y2--;	z2--;
									data++;
									break;
									
							case 5: cube_clear();
									switch(pattern) {						//1-6
										case 0:	cube_Filled( x1, y1, z1, x2, y2, z2 );	break;
										case 1:	 cube_Walls( x1, y1, z1, x2, y2, z2 );	break;
										default: cube_Edges( x1, y1, z1, x2, y2, z2 );	break;
									}
									x1++;	y1++;	z1++;
									x2--;	y2--;	z2--;
									data++;
									break;
							case 6: cube_clear();
									switch(pattern) {						//2-5
										case 0:	cube_Filled( x1, y1, z1, x2, y2, z2 );	break;
										case 1:	 cube_Walls( x1, y1, z1, x2, y2, z2 );	break;
										default: cube_Edges( x1, y1, z1, x2, y2, z2 );	break;
									}
									x1++;	y1++;	z1++;
									x2--;	y2--;	z2--;
									data++;
									break;
							case 7: cube_clear();
									switch(pattern) {						//3-4
										case 0:	cube_Filled( x1, y1, z1, x2, y2, z2 );	break;
										case 1:	 cube_Walls( x1, y1, z1, x2, y2, z2 );	break;
										default: cube_Edges( x1, y1, z1, x2, y2, z2 );	break;
									}
									data = 0;
									break;
									
						}

			
						timerClearDezi();
					}
					break;
					
					
					
			default: ;
		}
	}


	
}