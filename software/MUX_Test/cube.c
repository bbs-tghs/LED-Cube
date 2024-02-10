/*
 * cube.c
 *
 * Created: 04.05.2017 11:19:24
 *  Author: busser.michael
 */ 


#include <avr/io.h>
#include <stdint.h>
#include <stddef.h>
#include <util/delay.h>

#include "drv_573.h"
#include "cube.h"




/** 
 @brief Interne Funktion
		Sorgt dafür, das x1 stets kleiner, höchstens gleich x2 ist und vertauchst die Werte der Parameter ggf.
*/
void sortAsc( uint8_t *x1, uint8_t *x2 ) {
	uint8_t tmp;
	if ((*x1) > (*x2)) {
		tmp = *x1;
		*x2 = *x1;
		*x1 = tmp;
	}
}


//clear the entire LED map
void cube_clear() {
	uint8_t col, layer;
	
	for (layer=0; layer<CUBE_MAX_LAYERS; layer++) {
		for(col=0; col<CUBE_MAX_COLS; col++) {
			cube[layer][col] = 0;
		}
	}
}


//set a 64-Bit-Value to the layer 
void cube_setLayer( uint8_t layer, uint64_t	value ) {
	layer = layer % CUBE_MAX_LAYERS;	//Array-grenze beachten
	cube[layer][0] = (value & 0xFF);
	cube[layer][1] = (value >>  8) & 0xFF;
	cube[layer][2] = (value >> 16) & 0xFF;
	cube[layer][3] = (value >> 24) & 0xFF;
	cube[layer][4] = (value >> 32) & 0xFF;
	cube[layer][5] = (value >> 40) & 0xFF;
	cube[layer][6] = (value >> 48) & 0xFF;
	cube[layer][7] = (value >> 56) & 0xFF;
}


void cube_setCol( uint8_t layer, uint8_t col, uint8_t value ) {
	cube[layer][col] = value;
}

void cube_setRow( uint8_t layer, uint8_t row, uint8_t value ) {
	uint8_t i;
	for(i=0; i<8; i++) {
		if ((value & (1<<i)) > 0) {	//Bit i in Value ist gesetzt
			cube[layer][i] |= (1<<row); 
		} else {					//Bit i in Value ist gelöscht
			cube[layer][i] &= ~(1<<row);
		}
	}
}


/** 
 @brief Schiebt die komplette Ebene (layer) um die in digits angegebene Anzahl von
		Bitposition nach rechts. Von Links wird das Byte in append eingezogen.
		
 @param	layer	Die Ebene, auf der die Operation ausgeführt werden soll
 
 @param	digits	Die Anzahl von Bits, um die die Ebene verschoben werden soll
 
 @param append	Das Byte, welches von links in der Feld eingezogen werden soll
*/

void cube_shiftRight( uint8_t layer, uint8_t digits, uint8_t append ) {
	uint8_t col;
	while( digits > 0) {
		for( col=(CUBE_MAX_COLS-1); col>0; col--) {
			cube[layer][col] = cube[layer][col-1]; 	
		}
		cube[layer][0] = append;
		digits--;
	}
	
}

void cube_shiftLeft( uint8_t layer, uint8_t digits, uint8_t append ) {
	uint8_t col;
	while( digits > 0) {
		for( col=0; col<(CUBE_MAX_COLS-1); col++) {
			cube[layer][col] = cube[layer][col+1];
		}
		cube[layer][CUBE_MAX_COLS-1] = append;
		digits--;
	}
}

void cube_shiftUp( uint8_t layer, uint8_t digits, uint8_t append ) {
	uint8_t col;
	for( col=0; col<CUBE_MAX_COLS; col++) {
		cube[layer][col] = cube[layer][col] >> digits;
	}
	cube_setRow( layer, CUBE_MAX_ROWS-1, append );
}

void cube_shiftDown( uint8_t layer, uint8_t digits, uint8_t append ) {
	uint8_t col;
	for( col=0; col<CUBE_MAX_COLS; col++) {
		cube[layer][col] = cube[layer][col] << digits;
	}
	cube_setRow( layer, 0, append );
}


/** 
 @brief Prüft, ob sich die angegebenen Koordinaten innerhalb des Würfels befinden
		
 @param	x,y,z	Die Parameter der drei Achsen
 
 @result		= 1, wenn die Werte innerhalb der Feldgrenzen liegen, sonst 0
				
*/

uint8_t cube_inRange( uint8_t x, uint8_t y, uint8_t z) {
	if (x < CUBE_MAX_ROWS && 
		y < CUBE_MAX_COLS && 
		z < CUBE_MAX_LAYERS) {
		return 1;
	} else 	{	// Mind. eine Koordinate liegt außerhalb
		return 0;
	}
}

void cube_setVoxel( uint8_t x, uint8_t y, uint8_t z ) {
	if (cube_inRange(x,y,z)) {
		cube[z][y] |= (1 << x);	
	}
}

void cube_clearVoxel( uint8_t x, uint8_t y, uint8_t z ) {
	if (cube_inRange(x,y,z)) {
		cube[z][y] &= ~(1 << x);
	}
}

void cube_flipVoxel( uint8_t x, uint8_t y, uint8_t z ) {
	if (cube_inRange(x,y,z)) {
		cube[z][y] ^= (1 << x);
	}
}

/** 
 @brief Ein Voxel umschalten.
		Wenn es gesetzt ist, so wird es gelöscht und umgekehrt
		
 @param	x,y,z	Die Parameter der drei Achsen
*/
uint8_t	cube_getVoxel( uint8_t x, uint8_t y, uint8_t z ) {
	uint8_t res;
	res = 0;
	if (cube_inRange(x,y,z))	{
		if (cube[z][y] & (1 << x))	{
			res = 1;
		}
	}
	return (res);	
}

void cube_setPlaneX( uint8_t x ) {
	uint8_t y,z;
	if (x < CUBE_MAX_ROWS)	{
		for (z=0; z < CUBE_MAX_LAYERS; z++)		{
			for (y=0; y < CUBE_MAX_COLS; y++)			{
				cube[z][y] |= (1 << x);
			}
		}
	}	
}

void cube_clearPlaneX( uint8_t x ) {
	uint8_t y,z;
	if (x < CUBE_MAX_ROWS)	{
		for (z=0; z < CUBE_MAX_LAYERS; z++)		{
			for (y=0; y < CUBE_MAX_COLS; y++)			{
				cube[z][y] &= ~(1 << x);
			}
		}
	}
}

void cube_setPlaneY( uint8_t y ) {
	uint8_t z;
	if (y < CUBE_MAX_COLS)	{
		for (z=0; z < CUBE_MAX_LAYERS; z++)	{
			cube[z][y] = 0xff;
		}
	}
}

void cube_clearPlaneY( uint8_t y ) {
	uint8_t z;
	if (y < CUBE_MAX_COLS)	{
		for (z=0; z < CUBE_MAX_LAYERS; z++)	{
			cube[z][y] = 0x00;
		}
	}
}

void cube_setPlaneZ( uint8_t z ) {
	uint8_t y;
	if (z < CUBE_MAX_LAYERS ) {
		for (y=0; y<CUBE_MAX_COLS; y++) {
			cube[z][y] = 0xFF;	
		}
	}
}

void cube_clearPlaneZ( uint8_t z ) {
	uint8_t y;
	if (z < CUBE_MAX_LAYERS ) {
		for (y=0; y<CUBE_MAX_COLS; y++) {
			cube[z][y] = 0x00;
		}
	}
}

void cube_setPlane( TAXIS_DRV573 c, uint8_t i) {
	switch(c) {
		case AXIS_X :	cube_setPlaneX( i ); break;
		case AXIX_Y :	cube_setPlaneY( i ); break;
		case AXIX_Z :	cube_setPlaneZ( i ); break;
	}	
}

void cube_clearPlane( TAXIS_DRV573 c, uint8_t i) {
	switch(c) {
		case AXIS_X :	cube_clearPlaneX( i ); break;
		case AXIX_Y :	cube_clearPlaneY( i ); break;
		case AXIX_Z :	cube_clearPlaneZ( i ); break;
	}
}

/** 
 @brief Gibt eine Folge von Bits in einem Byte zurück.
		Beispiel:   bitLine(2,5) liefert   0b00111100
				
 @param	start	Das erste Bit, welches gesetzt werden soll
 @param end		Das letzte Bit, welches noch gesetzt werden soll
 
 @result		Ein Byte, dessen Bits von start bis end gesetzt sind
*/

uint8_t cube_bitLine (uint8_t start, uint8_t end)  {
	return ((0xFF << start) & ~(0xFF << (end+1)));
}



/** 
 @brief Vertauscht die Bits in einem Byte.
		MSB tauscht mit LSB usw.
		Aus der Reihenfolge 76543210  wird also 01234567

		Ein Beispiel für Assember:  (x in r16, Ergebnis in r17, Anzahl der Bits in r18)
		  ldi r18, 8
		swap_em:
		  ror r16
		  rol r17
		  dec r18
		  brne swap_em


		Atmel's AppNote AVR307 "Half duplex UART using the USI Interface"
				
 @param	x		Das umzudrehende Byte
 
 @result		Das umgedrehte Byte zu x
*/
uint8_t cube_flip( uint8_t x ) {
    x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
    x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
    x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
    return x;	
}

//ausgefüllter Würfel
void cube_Filled( uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2 ) {
	uint8_t iy,iz;
	
	sortAsc( &x1, &x2 );
	sortAsc( &y1, &y2 );
	sortAsc( &z1, &z2 );
	
	for (iz=z1; iz <= z2; iz++)	{
		for (iy=y1; iy <= y2; iy++) {
			cube[iz][iy] |= cube_bitLine( x1, x2 );
		}
	}	
}


//Seiten eines Würfel
void cube_Walls( uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2 ) {
	uint8_t iy,iz;
	
	sortAsc( &x1, &x2 );
	sortAsc( &y1, &y2 );
	sortAsc( &z1, &z2 );
	
	for (iz=z1; iz <= z2; iz++)	{
		for (iy=y1; iy <= y2; iy++) {
			
			if (iy == y1 || iy == y2 || iz == z1 || iz == z2) {
				cube[iz][iy] = cube_bitLine(x1,x2);
			} else	{
				cube[iz][iy] |= ((0x01 << x1) | (0x01 << x2));
			}
		}
	}
}

//Kanten eines Würfel
void cube_Edges( uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2 ) {
	uint8_t iy,iz;
	
	sortAsc( &x1, &x2 );
	sortAsc( &y1, &y2 );
	sortAsc( &z1, &z2 );
	
	// Linien entlang der X-Achse
	cube[z1][y1] = cube_bitLine(x1,x2);
	cube[z1][y2] = cube_bitLine(x1,x2);
	cube[z2][y1] = cube_bitLine(x1,x2);
	cube[z2][y2] = cube_bitLine(x1,x2);

	// Linien entlang der Y-Achse
	for (iy=y1; iy <= y2; iy++)	{
		cube_setVoxel(x1,iy,z1);
		cube_setVoxel(x1,iy,z2);
		cube_setVoxel(x2,iy,z1);
		cube_setVoxel(x2,iy,z2);
	}

	// Linien entlang der Z-Achse
	for (iz=z1; iz <= z2; iz++) {
		cube_setVoxel(x1,y1,iz);
		cube_setVoxel(x1,y2,iz);
		cube_setVoxel(x2,y1,iz);
		cube_setVoxel(x2,y2,iz);
	}	
}








uint8_t	cube_getData( uint8_t layer, uint8_t col ) {
	return( cube[layer][col] );
}

void	cube_setData( uint8_t layer, uint8_t col, uint8_t data ) {
	cube[layer][col] = data;
}

