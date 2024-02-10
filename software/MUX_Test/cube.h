/*
 * cube.h
 *
 * Created: 04.05.2017 11:19:38
 *  Author: busser.michael
 */ 


#ifndef CUBE_H_
#define CUBE_H_



typedef enum {
	AXIS_X,
	AXIX_Y,
	AXIX_Z
} TAXIS_DRV573;

#define CUBE_MAX_COLS			8			//Nr of columns 0..7
#define CUBE_MAX_ROWS			8			//Nr of rows
#define CUBE_MAX_BIT_PER_LINE	8			//Nr of bits per row e.g. the columns 0..7
#define CUBE_MAX_LAYERS			8			//Nr of stacked layers
#define CUBE_MAX_X				8
#define CUBE_MAX_Y				8
#define CUBE_MAX_Z				CUBE_MAX_LAYERS

//Public data structure
uint8_t volatile cube[CUBE_MAX_LAYERS][CUBE_MAX_COLS];		//bit map for the cube

void cube_clear();			//clear all leds


void cube_setLayer( uint8_t layer, uint64_t	value );
void cube_setCol(   uint8_t layer, uint8_t col, uint8_t value );				//ok
void cube_setRow(   uint8_t layer, uint8_t row, uint8_t value );				//ok


void cube_shiftRight( uint8_t layer, uint8_t digits, uint8_t append );
void cube_shiftLeft(  uint8_t layer, uint8_t digits, uint8_t append );
void cube_shiftUp(    uint8_t layer, uint8_t digits, uint8_t append );
void cube_shiftDown(  uint8_t layer, uint8_t digits, uint8_t append );



//---- Zeichenfunktionen ----
uint8_t cube_inRange( uint8_t x, uint8_t y, uint8_t z);				//ok
void	cube_setVoxel( uint8_t x, uint8_t y, uint8_t z);			//ok
void	cube_clearVoxel( uint8_t x, uint8_t y, uint8_t z );			//ok
void	cube_flipVoxel( uint8_t x, uint8_t y, uint8_t z );			//ok
uint8_t	cube_getVoxel( uint8_t x, uint8_t y, uint8_t z );

//Hilfsfunktionen
uint8_t cube_bitLine (uint8_t start, uint8_t end);
uint8_t cube_flip( uint8_t x );

//Ebenenfunktionen
void cube_setPlaneX( uint8_t x ) ;									//ok
void cube_clearPlaneX( uint8_t x );									//ok
void cube_setPlaneY( uint8_t y );									//ok
void cube_clearPlaneY( uint8_t y );									//ok
void cube_setPlaneZ( uint8_t z );									//ok
void cube_clearPlaneZ( uint8_t z );									//ok
void cube_setPlane( TAXIS_DRV573 c, uint8_t i);
void cube_clearPlane( TAXIS_DRV573 c, uint8_t i);

//Volumenfunktionen
void cube_Filled( uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2 );
void cube_Walls( uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2 );
void cube_Edges( uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2 );


//Testfunktionen
uint8_t	cube_getData( uint8_t layer, uint8_t col );
void	cube_setData( uint8_t layer, uint8_t col, uint8_t data );





#endif /* CUBE_H_ */