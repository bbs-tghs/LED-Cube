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



#define MAX(a,b) (((a)>(b))?(a):(b))		/* find maximum of a and b */
#define ABS(a) (((a)<0) ? -(a) : (a))		/* absolute value of a */

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


void cube_shift( TAXIS c, int8_t direction ) {
uint8_t y,z;	
	if (direction < 0) { direction = -1; } else { direction = 1; }
	switch (c) {
		case AXIS_X:	for (z=0; z<CUBE_MAX_Z; z++) {
							for( y=0; y<CUBE_MAX_Y; y++) {
								if (direction == 1) {
									cube[z][y] = cube[z][y] << 1;
								} else {
									cube[z][y] = cube[z][y] >> 1;
								}
							}
						}
						break;
		case AXIS_Y:	for (z=0; z < CUBE_MAX_Z; z++) {	
							if (direction == 1) {
								for( y=CUBE_MAX_Y-1; y < 0; y--) {
									cube[z][y] = cube[z][y-1];
								}
								cube[z][0] = 0;
							} else {
								for( y=0; y < CUBE_MAX_Y-1; y++) {
									cube[z][y] = cube[z][y+1];
								}
								cube[z][CUBE_MAX_Y-1] = 0;
							}
						}
						break;
		case AXIS_Z:	if (direction == 1) {
							for ( z=CUBE_MAX_Z-1; z>0; z-- ){
								for (y=0; y<CUBE_MAX_Y; y++) {
									cube[z][y] = cube[z-1][y];
								}
							}
							cube_clearPlaneZ( 0 );					
						} else {
							for ( z=0; z < CUBE_MAX_Z-1; z++ ){
								for (y=0; y<CUBE_MAX_Y; y++) {
									cube[z][y] = cube[z+1][y];
								}
							}
							cube_clearPlaneZ( CUBE_MAX_Z-1 );
						}
						break;
	}	

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

/** 
 @brief Ein Voxel einschalten.
		
 @param	x,y,z	Die Parameter der drei Achsen
*/
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

/** 
 @brief Ein Voxel umschalten.
		Wenn es gesetzt ist, so wird es gelöscht und umgekehrt
		
 @param	x,y,z	Die Parameter der drei Achsen
*/
void cube_flipVoxel( uint8_t x, uint8_t y, uint8_t z ) {
	if (cube_inRange(x,y,z)) {
		cube[z][y] ^= (1 << x);
	}
}

/** 
 @brief Ein Voxel abfragen.
		
 @param	x,y,z	Die Parameter der drei Achsen
 
 @return	0 = Voxel ist ausgeschaltet
			1 = voxel ist eingeschaltet
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

void cube_setPlane( TAXIS c, uint8_t i) {
	switch(c) {
		case AXIS_X :	cube_setPlaneX( i ); break;
		case AXIS_Y :	cube_setPlaneY( i ); break;
		case AXIS_Z :	cube_setPlaneZ( i ); break;
	}	
}

void cube_clearPlane( TAXIS c, uint8_t i) {
	switch(c) {
		case AXIS_X :	cube_clearPlaneX( i ); break;
		case AXIS_Y :	cube_clearPlaneY( i ); break;
		case AXIS_Z :	cube_clearPlaneZ( i ); break;
	}
}

void cube_setLineX( uint8_t y, uint8_t z ) {
	uint8_t x;
	for (x=0; x<CUBE_MAX_X; x++) cube_setVoxel(x,y,z);
}

void cube_clearLineX( uint8_t y, uint8_t z ) {
	uint8_t x;
	for (x=0; x<CUBE_MAX_X; x++) cube_clearVoxel(x,y,z);
}

void cube_setLineY( uint8_t x, uint8_t z ) {
	uint8_t y;
	for (y=0; y<CUBE_MAX_Y; y++) cube_setVoxel(x,y,z);
}

void cube_clearLineY( uint8_t x, uint8_t z ) {
	uint8_t y;
	for (y=0; y<CUBE_MAX_Y; y++) cube_clearVoxel(x,y,z);
}

void cube_setLineZ( uint8_t x, uint8_t y ) {
	uint8_t z;
	for (z=0; z<CUBE_MAX_Z; z++) cube_setVoxel(x,y,z);
}

void cube_clearLineZ( uint8_t x, uint8_t y ) {
	uint8_t z;
	for (z=0; z<CUBE_MAX_Z; z++) cube_clearVoxel(x,y,z);
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


// Parameetr im Bereich von 0..7
void cube_Line(  uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2  ) {
	int8_t dx, dy, dz;
    int8_t ax, ay, az;
    int8_t sx, sy, sz;
	int8_t  x,  y,  z;
	int8_t xd, yd, zd; 	
	
    dx = x2 - x1;
    dy = y2 - y1;
    dz = z2 - z1;
	
    ax = ABS(dx) << 1;
    ay = ABS(dy) << 1;
    az = ABS(dz) << 1;	
	
    sx = (dx < 0) ? -1 : dx>0 ? 1 : 0;		//Vorzeichen von dx liefern
    sy = (dy < 0) ? -1 : dy>0 ? 1 : 0;
    sz = (dz < 0) ? -1 : dz>0 ? 1 : 0;
	
    x = x1;
    y = y1;
    z = z1;	
	
    if (ax >= MAX(ay, az))  {          /* x dominant */
	    yd = ay - (ax >> 1);
	    zd = az - (ax >> 1);
	    for (;;)  {
		    cube_setVoxel(x, y, z);
		    if (x == x2) return;
		    if (yd >= 0) {
			    y += sy;
			    yd -= ax;
		    }
		    if (zd >= 0) {
			    z += sz;
			    zd -= ax;
		    }
		    x += sx;
		    yd += ay;
		    zd += az;
	    }
    } else if (ay >= MAX(ax, az))  {          /* y dominant */
	    xd = ax - (ay >> 1);
	    zd = az - (ay >> 1);
	    for (;;) {
		    cube_setVoxel(x, y, z);
		    if (y == y2) return;
		    if (xd >= 0) {
			    x += sx;
			    xd -= ay;
		    }
		    if (zd >= 0) {
			    z += sz;
			    zd -= ay;
		    }
		    y += sy;
		    xd += ax;
		    zd += az;
	    }
    } else if (az >= MAX(ax, ay)) {            /* z dominant */
	    xd = ax - (az >> 1);
	    yd = ay - (az >> 1);
	    for (;;) {
		    cube_setVoxel(x, y, z);
		    if (z == z2) return;
		    if (xd >= 0) {
			    x += sx;
			    xd -= az;
		    }
		    if (yd >= 0) {
			    y += sy;
			    yd -= az;
		    }
		    z += sz;
		    xd += ax;
		    yd += ay;
	    }
    }
	
	
}