/*
 * drv_573.h
 *
 * Created: 10.10.2016 17:26:30
 *  Author: busser.michael
 * 
 * Drive a number of 74*573-Latches
 */ 


#ifndef DRV_573_H_
#define DRV_573_H_

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

#define D573_MAX_COLS			8			//Nr of columns 0..7
#define D573_MAX_ROWS			8			//Nr of rows
#define D573_MAX_BIT_PER_LINE	8			//Nr of bits per row e.g. the columns 0..7
#define D573_MAX_LAYERS			8			//Nr of stacked layers


typedef enum {
	CB_SYNC,
	CB_DRIVE_ENTER,
	CB_DRIVE_LEAVE,
	CB_NEW_LAYER 
	} TCB_DRV573;


typedef void (*drv573_callback_t) ( TCB_DRV573 cb );		//Event Callback-Funktion, es werden keine Parameter übergeben

typedef enum {
	AXIS_X,
	AXIX_Y,
	AXIX_Z
	} TAXIS_DRV573;



void d573_init();		//Treiber initialisieren
void d573_clear();		//alle LEDs aus
void d573_drive();		//call every 1 ms

void d573_setLayer( uint8_t layer, uint64_t	value );
void d573_setCol(   uint8_t layer, uint8_t col, uint8_t value );
void d573_setRow(   uint8_t layer, uint8_t row, uint8_t value );


void d573_shiftRight( uint8_t layer, uint8_t digits, uint8_t append );
void d573_shiftLeft(  uint8_t layer, uint8_t digits, uint8_t append );
void d573_shiftUp(    uint8_t layer, uint8_t digits, uint8_t append );
void d573_shiftDown(  uint8_t layer, uint8_t digits, uint8_t append );


void d573_Register_Callback( drv573_callback_t cbf );

//---- Zeichenfunktionen ----
uint8_t inRange( uint8_t x, uint8_t y, uint8_t z);
void	setVoxel( uint8_t x, uint8_t y, uint8_t z);
void	clearVoxel( uint8_t x, uint8_t y, uint8_t z );
void	flipVoxel( uint8_t x, uint8_t y, uint8_t z );
uint8_t	getVoxel( uint8_t x, uint8_t y, uint8_t z );  

//Hilfsfunktionen
uint8_t bitLine (uint8_t start, uint8_t end);
uint8_t flip( uint8_t x ); 

//Ebenenfunktionen
void setPlaneX( uint8_t x ) ;
void clearPlaneX( uint8_t x );
void setPlaneY( uint8_t y );
void clearPlaneY( uint8_t y );
void setPlaneZ( uint8_t z );
void clearPlaneZ( uint8_t z );
void setPlane( TAXIS_DRV573 c, uint8_t i);
void clearPlane( TAXIS_DRV573 c, uint8_t i);

//Volumenfunktionen
void cube_Filled( uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2 );
void cube_Walls( uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2 );
void cube_Edges( uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2 );


//Testfunktionen
void	d573test_setLatch( uint8_t latch, uint8_t value);
void	d573test_clearAllLatches();
void	d573test_setLayer( uint8_t layer, uint8_t onOff ); 
uint8_t	d573test_getData( uint8_t layer, uint8_t col );
void	d573test_setData( uint8_t layer, uint8_t col, uint8_t data );



//Quelle:  -> https://www.mikrocontroller.net/articles/Bitmanipulation
// set bit
static inline void BIT_SET(volatile uint8_t *target, uint8_t bit) __attribute__((always_inline));
static inline void BIT_SET(volatile uint8_t *target, uint8_t bit){
	*target |= (1<<bit);
};

// set clear
static inline void BIT_CLEAR(volatile uint8_t *target, uint8_t bit) __attribute__((always_inline));
static inline void BIT_CLEAR(volatile uint8_t *target, uint8_t bit){
	*target &= ~(1<<bit);
};

// bit toogle
static inline void BIT_TOGGLE(volatile uint8_t *target, uint8_t bit) __attribute__((always_inline));
static inline void BIT_TOGGLE(volatile uint8_t *target, uint8_t bit){
	*target ^= (1<<bit);
};








#endif /* DRV_573_H_ */