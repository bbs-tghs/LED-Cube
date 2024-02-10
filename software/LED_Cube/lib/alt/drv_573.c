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

#include "drv_573.h"




uint8_t volatile d573data[D573_MAX_LAYERS][D573_MAX_COLS];		//bit map for the cube
uint8_t	d573_layer = 0;											//actually shown layer

/** 
 @brief Variablen für die Adressen der Callback-Funktionen.
		Diese müssen mit den entsprechenden register-Funktionen registriert werden.
*/
drv573_callback_t	d573_CB_Handler = NULL;			///@brief Adresse Callback-Routine



void d573_Register_Callback( drv573_callback_t cbf ) {
	d573_CB_Handler = cbf;
}

void d573_init() {

	D573_Data_DDR	= 0xFF;						//output
	D573_Data_Port	= 0;						//all off

	D573_Clock_DDR  = 0xFF;						//output
	D573_Clock_PORT = 0;						//all off

	D573_LAYER_DDR  |= (D573_LAYER_MASK);		//PD3..PD7 as output
	D573_LAYER_OFF;								//D573_LAYER_ENABLE	= 0
	D573_LATCH_OUT_DIS;							//D573_LATCH_OE = 1
	D573_LAYER_ADR_CLEAR;						//D573_LAYER_E0 = 0,	D573_LAYER_E1 = 0,	D573_LAYER_E2 = 0
	
	d573_clear();								//init bit map
}


void d573_drive() {		//call every 1 ms
	uint8_t i;

	if (d573_CB_Handler != NULL) { d573_CB_Handler(CB_DRIVE_ENTER); }		//Callback to status report
		
	D573_LAYER_OFF;			//Enable-Pin für Ebenenmultiplexer auf 0 -> alle Ebeben ausschalten
	D573_LATCH_OUT_DIS;		//Latches OE ausschalten, damit Low an Low-Side-Treiber
	D573_LAYER_ADR_CLEAR;	//Adressbits E0..E2 auf 0 setzen
	
	d573_layer = ((d573_layer+1) % D573_MAX_LAYERS);		//Dieser Layer soll jetzt als nächstes angezeigt werden  (0..7)

	if ((d573_layer == 0) && (d573_CB_Handler != NULL)) {	//Sync anzeigen, wenn es eine Callback-Funktion gibt
		d573_CB_Handler(CB_SYNC);							//Wird nur bei Layer 0 ausgelöst
	}
		
	//transfer the content of d573data array to the latches
	for (i=0; i<D573_MAX_COLS; i++) {

		D573_Data_Port  = d573data[d573_layer][i];	//Byte an die Eingänge der Latches anlegen
		asm volatile ("nop");						//	
		D573_Clock_PORT = (1<<i);					//Clock-Signal am Latch i auf H setzen
		asm volatile ("nop");						//Eine kurze Pause per NOP-Anweisung einfügen.
		asm volatile ("nop");						//Mal testen, ob das notwendig ist
		D573_Clock_PORT = 0;						//Clock-Signal wieder auf L
		asm volatile ("nop");						//
		D573_Data_Port = 0;							//Datenpins am Eingang der Latches auf L setzen   (Nötig?)
	}
	
	D573_LAYER_PORT = (D573_LAYER_PORT  | (d573_layer << D573_LAYER_E0) );		//Adresse am Ebenenmultiplexer einstellen
	D573_LATCH_OUT_EN;	//Latches aktivieren, Low-side-Treiber ein
	D573_LAYER_ON;		//High-Side-Treiber ein
	
	if (d573_CB_Handler != NULL) { d573_CB_Handler(CB_DRIVE_LEAVE); }
}



//- Working with data -

//clear the entire LED map
void d573_clear() {
	uint8_t col, layer;
	
	for (layer=0; layer<D573_MAX_LAYERS; layer++) {
		for(col=0; col<D573_MAX_COLS; col++) {
			d573data[layer][col] = 0;
		}
	}
}

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


//set a 64-Bit-Value to the layer 
void d573_setLayer( uint8_t layer, uint64_t	value ) {
	layer = layer % D573_MAX_LAYERS;	//Array-grenze beachten
	d573data[layer][0] = (value & 0xFF);
	d573data[layer][1] = (value >>  8) & 0xFF;
	d573data[layer][2] = (value >> 16) & 0xFF;
	d573data[layer][3] = (value >> 24) & 0xFF;
	d573data[layer][4] = (value >> 32) & 0xFF;
	d573data[layer][5] = (value >> 40) & 0xFF;
	d573data[layer][6] = (value >> 48) & 0xFF;
	d573data[layer][7] = (value >> 56) & 0xFF;
}


void d573_setCol( uint8_t layer, uint8_t col, uint8_t value ) {
	d573data[layer][col] = value;
}

void d573_setRow( uint8_t layer, uint8_t row, uint8_t value ) {
	uint8_t i;
	for(i=0; i<8; i++) {
		if ((value & (1<<i)) > 0) {	//Bit i in Value ist gesetzt
			d573data[layer][i] |= (1<<row); 
		} else {					//Bit i in Value ist gelöscht
			d573data[layer][i] &= ~(1<<row);
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
void d573_shiftRight( uint8_t layer, uint8_t digits, uint8_t append ) {
	uint8_t col;
	while( digits > 0) {
		for( col=(D573_MAX_COLS-1); col>0; col--) {
			d573data[layer][col] = d573data[layer][col-1]; 	
		}
		d573data[layer][0] = append;
		digits--;
	}
	
}

void d573_shiftLeft( uint8_t layer, uint8_t digits, uint8_t append ) {
	uint8_t col;
	while( digits > 0) {
		for( col=0; col<(D573_MAX_COLS-1); col++) {
			d573data[layer][col] = d573data[layer][col+1];
		}
		d573data[layer][D573_MAX_COLS-1] = append;
		digits--;
	}
}

void d573_shiftUp( uint8_t layer, uint8_t digits, uint8_t append ) {
	uint8_t col;
	for( col=0; col<D573_MAX_COLS; col++) {
		d573data[layer][col] = d573data[layer][col] >> digits;
	}
	d573_setRow( layer, D573_MAX_ROWS-1, append );
}

void d573_shiftDown( uint8_t layer, uint8_t digits, uint8_t append ) {
	uint8_t col;
	for( col=0; col<D573_MAX_COLS; col++) {
		d573data[layer][col] = d573data[layer][col] << digits;
	}
	d573_setRow( layer, 0, append );
}


//----------------------------------------------------------------------------
//--- Draw ----
/** 
 @brief Prüft, ob sich die angegebenen Koordinaten innerhalb des Würfels befinden
		
 @param	x,y,z	Die Parameter der drei Achsen
 
 @result		= 1, wenn die Werte innerhalb der Feldgrenzen liegen, sonst 0
				
*/
uint8_t inRange( uint8_t x, uint8_t y, uint8_t z) {
	if (x < D573_MAX_ROWS && 
		y < D573_MAX_COLS && 
		z < D573_MAX_LAYERS) {
		return 1;
	} else 	{	// Mind. eine Koordinate liegt außerhalb
		return 0;
	}
}

void setVoxel( uint8_t x, uint8_t y, uint8_t z ) {
	if (inRange(x,y,z)) {
		d573data[z][y] |= (1 << x);	
	}
}

void clearVoxel( uint8_t x, uint8_t y, uint8_t z ) {
	if (inRange(x,y,z)) {
		d573data[z][y] &= ~(1 << x);
	}
}

void flipVoxel( uint8_t x, uint8_t y, uint8_t z ) {
	if (inRange(x,y,z)) {
		d573data[z][y] ^= (1 << x);
	}
}

/** 
 @brief Ein Voxel umschalten.
		Wenn es gesetzt ist, so wird es gelöscht und umgekehrt
		
 @param	x,y,z	Die Parameter der drei Achsen
*/
uint8_t	getVoxel( uint8_t x, uint8_t y, uint8_t z ) {
	uint8_t res;
	res = 0;
	if (inRange(x,y,z))	{
		if (d573data[z][y] & (1 << x))	{
			res = 1;
		}
	}
	return (res);	
}

void setPlaneX( uint8_t x ) {
	uint8_t y,z;
	if (x < D573_MAX_ROWS)	{
		for (z=0; z < D573_MAX_LAYERS; z++)		{
			for (y=0; y < D573_MAX_COLS; y++)			{
				d573data[z][y] |= (1 << x);
			}
		}
	}	
}

void clearPlaneX( uint8_t x ) {
	uint8_t y,z;
	if (x < D573_MAX_ROWS)	{
		for (z=0; z < D573_MAX_LAYERS; z++)		{
			for (y=0; y < D573_MAX_COLS; y++)			{
				d573data[z][y] &= ~(1 << x);
			}
		}
	}
}

void setPlaneY( uint8_t y ) {
	uint8_t z;
	if (y < D573_MAX_COLS)	{
		for (z=0; z < D573_MAX_LAYERS; z++)	{
			d573data[z][y] = 0xff;
		}
	}
}

void clearPlaneY( uint8_t y ) {
	uint8_t z;
	if (y < D573_MAX_COLS)	{
		for (z=0; z < D573_MAX_LAYERS; z++)	{
			d573data[z][y] = 0x00;
		}
	}
}

void setPlaneZ( uint8_t z ) {
	uint8_t y;
	if (z < D573_MAX_LAYERS ) {
		for (y=0; y<D573_MAX_COLS; y++) {
			d573data[z][y] = 0xFF;	
		}
	}
}

void clearPlaneZ( uint8_t z ) {
	uint8_t y;
	if (z < D573_MAX_LAYERS ) {
		for (y=0; y<D573_MAX_COLS; y++) {
			d573data[z][y] = 0x00;
		}
	}
}

void setPlane( TAXIS_DRV573 c, uint8_t i) {
	switch(c) {
		case AXIS_X :	setPlaneX( i ); break;
		case AXIX_Y :	setPlaneY( i ); break;
		case AXIX_Z :	setPlaneZ( i ); break;
	}	
}

void clearPlane( TAXIS_DRV573 c, uint8_t i) {
	switch(c) {
		case AXIS_X :	clearPlaneX( i ); break;
		case AXIX_Y :	clearPlaneY( i ); break;
		case AXIX_Z :	clearPlaneZ( i ); break;
	}
}

/** 
 @brief Gibt eine Folge von Bits in einem Byte zurück.
		Beispiel:   bitLine(2,5) liefert   0b00111100
				
 @param	start	Das erste Bit, welches gesetzt werden soll
 @param end		Das letzte Bit, welches noch gesetzt werden soll
 
 @result		Ein Byte, dessen Bits von start bis end gesetzt sind
*/

uint8_t bitLine (uint8_t start, uint8_t end)  {
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
uint8_t flip( uint8_t x ) {
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
			d573data[iz][iy] |= bitLine( x1, x2 );
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
				d573data[iz][iy] = bitLine(x1,x2);
			} else	{
				d573data[iz][iy] |= ((0x01 << x1) | (0x01 << x2));
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
	d573data[z1][y1] = bitLine(x1,x2);
	d573data[z1][y2] = bitLine(x1,x2);
	d573data[z2][y1] = bitLine(x1,x2);
	d573data[z2][y2] = bitLine(x1,x2);

	// Linien entlang der Y-Achse
	for (iy=y1; iy <= y2; iy++)	{
		setVoxel(x1,iy,z1);
		setVoxel(x1,iy,z2);
		setVoxel(x2,iy,z1);
		setVoxel(x2,iy,z2);
	}

	// Linien entlang der Z-Achse
	for (iz=z1; iz <= z2; iz++) {
		setVoxel(x1,y1,iz);
		setVoxel(x1,y2,iz);
		setVoxel(x2,y1,iz);
		setVoxel(x2,y2,iz);
	}	
}





//Value in das Latch 0..7 schreiben;   entspricht for-Schleife in d573_drive()
void d573test_setLatch( uint8_t latch, uint8_t value) {
	latch = latch % 8;
	D573_Data_Port = value;
	asm volatile ("nop");						//
	D573_Clock_PORT = (1<<latch);
	asm volatile ("nop");						//
	asm volatile ("nop");						//
	D573_Clock_PORT = 0;
	asm volatile ("nop");						//
	D573_Data_Port = 0;
}


void d573test_clearAllLatches() {
	D573_Data_Port = 0;
	asm volatile ("nop");						//
	D573_Clock_PORT = 0xFF;
	asm volatile ("nop");						//
	asm volatile ("nop");						//
	D573_Clock_PORT = 0;
	asm volatile ("nop");						//
	D573_Data_Port = 0;
}


void d573test_setLayer( uint8_t layer, uint8_t onOff ) {
	D573_LAYER_OFF;								//1-aus-8-Dekoder abschalten
	if (onOff == _ON) {
		layer = layer % D573_MAX_LAYERS;

		D573_LAYER_ADR_CLEAR;												//Adressbits E0..E2 auf 0 setzen
		D573_LAYER_PORT = (D573_LAYER_PORT  | (layer << D573_LAYER_E0) );	//Adresse am Ebenenmultiplexer einstellen
		D573_LAYER_ON;							//1-aus-8-Dekoder einschalten
	} else {
		D573_LAYER_OFF;
		D573_LAYER_ADR_CLEAR;												//Adressbits E0..E2 auf 0 setzen
	}
	
}

uint8_t	d573test_getData( uint8_t layer, uint8_t col ) {
	return( d573data[layer][col] );
}

void	d573test_setData( uint8_t layer, uint8_t col, uint8_t data ) {
	d573data[layer][col] = data;
}

