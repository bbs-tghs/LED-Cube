/*
 * misc.c
 *
 * Created: 30.04.2017 19:55:50
 *  Author: busser.michael
 */ 


#include <avr/io.h>
#include <stdint.h>
#include <stddef.h>



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
