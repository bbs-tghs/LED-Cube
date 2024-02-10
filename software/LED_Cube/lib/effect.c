/*
 * effect.c
 *
 * Created: 01.05.2017 15:17:31
 *  Author: busser.michael
 */ 

#include <avr\io.h>
#include <stddef.h>
//#include <inttypes.h>
#include <stdlib.h>
#include "effect.h"
//#include "drv_573.h"
#include "cube.h"
#include "uart.h"

TPatternCtrl	patternCtrl;
TPatternTyp		pattern;
uint8_t			patternDelay;
uint8_t			x, y, z;			//Voxel-Koordinaten
uint8_t			i;
uint8_t			delay;				//verzögerung innerhalb des 10ms-Rahmens
uint16_t		loop;



void pattern_RunningPoint( ) {
	if (delay > 0) {
		delay--;
	} else {
		cube_clearVoxel(x,y,z);
		if (!(patternCtrl.bDirZ)) { //Hoch
			if (x<7) {
				x++;
				cube_setVoxel(x,y,z);
				} else {
				x=0;
				if (y<7) {
					y++;
					cube_setVoxel(x,y,z);
					} else {
					y=0;
					if (z<7) {
						z++;
						cube_setVoxel(x,y,z);
						} else {
						x=7;  y=7;
						cube_setVoxel(x,y,z);
						patternCtrl.bDirZ=1;
					}
				}
			}
		} else {	//Runter
			if (x>0) {
				x--;
				cube_setVoxel(x,y,z);
				} else {
				x=7;
				if (y>0) {
					y--;
					cube_setVoxel(x,y,z);
					} else {
					y=7;
					if (z>0) {
						z--;
						cube_setVoxel(x,y,z);
						} else {
						cube_setVoxel(x,y,z);
						patternCtrl.bDirZ=0;
						patternCtrl.cycles--;		//wird am Ende aller Zyklen = 0
					}
				}
			}
		}
		delay = patternDelay;
	}
}


void pattern_Planes() {
	if (delay > 0) {
		delay--;
	} else {
		switch (patternCtrl.state) {
			case 0:	cube_clearPlaneX( x );
					if (x<7) {	//X-hoch
						x++;
						cube_setPlaneX( x );
					} else {
						patternCtrl.state = 1;
					}
					break;
				
			case 1: cube_clearPlaneX( x );
					if (x>0) {	//X-runter
						x--;
						cube_setPlaneX( x );
					} else {
						patternCtrl.state=2;
						cube_setPlaneY( y );
					}
					break;
				
			case 2:	cube_clearPlaneY( y );
					if (y<7) {	//y-hoch
						y++;
						cube_setPlaneY( y );
					} else {
						patternCtrl.state = 3;
					}
					break;
				
			case 3: cube_clearPlaneY( y );
					if (y>0) {	//y-runter
						y--;
						cube_setPlaneY( y );
					} else {
						patternCtrl.state=4;
						cube_setPlaneZ( z );
					}
					break;
				
			case 4:	cube_clearPlaneZ( z );
					if (z<7) {	//z-hoch
						z++;
						cube_setPlaneZ( z );
					} else {
						patternCtrl.state = 5;
					}
					break;
				
			case 5: cube_clearPlaneZ( z );
					if (z>0) {	//z-runter
						z--;
						cube_setPlaneZ( z );
					} else {
						patternCtrl.state=0;
						cube_setPlaneX( x );
						patternCtrl.cycles--;
					}
					break;
		}
		delay = patternDelay;
	}
}


void pattern_Cube( TCUBETYPE x ) {
	if (delay > 0) {
		delay--;
	} else {
		switch (patternCtrl.state) {
			case 0:	cube_clear();
					patternCtrl.state++;
					break;
			case 1: cube_clear();
					switch(x) {						//3-4
						case CUBE_FILLED: cube_Filled( 3, 3, 3, 4, 4, 4 );	break;
						case CUBE_WALL:	  cube_Walls(  3, 3, 3, 4, 4, 4 );	break;
						case CUBE_EDGES:  cube_Edges(  3, 3, 3, 4, 4, 4 );	break;
					}
					patternCtrl.state++;
					break;
			case 2: cube_clear();
					switch(x) {						//2-5
						case CUBE_FILLED: cube_Filled( 2, 2, 2, 5, 5, 5 );	break;
						case CUBE_WALL:	  cube_Walls(  2, 2, 2, 5, 5, 5 );	break;
						case CUBE_EDGES:  cube_Edges(  2, 2, 2, 5, 5, 5 );	break;
					}
					patternCtrl.state++;
					break;

			case 3: cube_clear();
					switch(x) {						//1-6
						case CUBE_FILLED: cube_Filled( 1, 1, 1, 6, 6, 6 );	break;
						case CUBE_WALL:	  cube_Walls(  1, 1, 1, 6, 6, 6 );	break;
						case CUBE_EDGES:  cube_Edges(  1, 1, 1, 6, 6, 6 );	break;
					}
					patternCtrl.state++;
					break;
			case 4: cube_clear();
					switch(x) {						///0-7
						case CUBE_FILLED: cube_Filled( 0, 0, 0, 7, 7, 7 );	break;
						case CUBE_WALL:	  cube_Walls(  0, 0, 0, 7, 7, 7 );	break;
						case CUBE_EDGES:  cube_Edges(  0, 0, 0, 7, 7, 7 );	break;
					}
					patternCtrl.state++;
					break;
			case 5: cube_clear();
					switch(x) {						//1-6
						case CUBE_FILLED: cube_Filled( 1, 1, 1, 6, 6, 6 );	break;
						case CUBE_WALL:	  cube_Walls(  1, 1, 1, 6, 6, 6 );	break;
						case CUBE_EDGES:  cube_Edges(  1, 1, 1, 6, 6, 6 );	break;
					}
					patternCtrl.state++;
					break;

			case 6: cube_clear();
					switch(x) {						//2-5
						case CUBE_FILLED: cube_Filled( 2, 2, 2, 5, 5, 5 );	break;
						case CUBE_WALL:	  cube_Walls(  2, 2, 2, 5, 5, 5 );	break;
						case CUBE_EDGES:  cube_Edges(  2, 2, 2, 5, 5, 5 );	break;
					}
					patternCtrl.state++;
					break;

			case 7: cube_clear();
					switch(x) {						//3-4
						case CUBE_FILLED: cube_Filled( 3, 3, 3, 4, 4, 4 );	break;
						case CUBE_WALL:	  cube_Walls(  3, 3, 3, 4, 4, 4 );	break;
						case CUBE_EDGES:  cube_Edges(  3, 3, 3, 4, 4, 4 );	break;
					}
					patternCtrl.state = 0;
					patternCtrl.cycles--;
					break;
		}
		delay = patternDelay;
	}
}


void pattern_rain() {
	if (delay > 0) {
		delay--;
	} else {
		cube_shift( AXIS_Z, -1 );
		if ((patternCtrl.cycles % patternCtrl.state) == 0) {
			z = rand()%5;			//nr of raindrops created at once
			for ( i=0; i < z; i++ )	{
				cube_setVoxel( rand()%CUBE_MAX_X, rand()%CUBE_MAX_Y, CUBE_MAX_Z-1 );
			} 
		}
		patternCtrl.cycles--;		//wird am Ende aller Zyklen = 0
		delay = patternDelay;
	}
	
}

void pattern_SparkleFlash() {
	if (delay > 0) {
		delay--;
	} else {
		cube_clear();
		switch (patternCtrl.state) {
			case 0:	for (i=0; i<y; i++) {
						cube_setVoxel( rand()%CUBE_MAX_X, rand()%CUBE_MAX_Y, rand()%CUBE_MAX_Z );
					}
					loop--;
					if (loop == 0 ) {
						loop = z;
						y++;
						if (y >= x) { y=x; patternCtrl.state = 1; }
					}
					break;
			case 1: for (i=0; i<y; i++) {
						cube_setVoxel( rand()%CUBE_MAX_X, rand()%CUBE_MAX_Y, rand()%CUBE_MAX_Z );
					}
					loop--;
					if (loop == 0) {
						loop = z;
						y--;
						if (y <= 1) { y=1; patternCtrl.state = 2; }
					}
					break;
			case 2:	cube_clear();
					patternCtrl.cycles--;		//wird am Ende aller Zyklen = 0
					break;
			default: ;
		}
		delay = patternDelay;
	}
} 


void pattern_Tunnel() {
	if (delay > 0) {
		delay--;
	} else {
		switch (patternCtrl.state) {
			case 0: cube_setLineX( y, z );
					if (y < CUBE_MAX_Y-1 ) { y++; } else { z++; patternCtrl.state = 1; }
					break;
					
			case 1: cube_setLineX( y, z );
					if (z < CUBE_MAX_Z-1 ) { z++; } else { y--; patternCtrl.state = 2; }
					break;
					
			case 2: cube_setLineX( y, z );
					if (y > 0 ) { y--; } else { z--; patternCtrl.state = 3; }
					break;
			
			case 3: cube_setLineX( y, z );
					if (z > 0 ) { z--; } else { patternCtrl.state = 4; }
					break;
			
			
			case 4: cube_clearLineX( y, z );
					if (y < CUBE_MAX_Y-1 ) { y++; } else { z++; patternCtrl.state = 5; }
					break;
			
			case 5: cube_clearLineX( y, z );
					if (z < CUBE_MAX_Z-1 ) { z++; } else { y--; patternCtrl.state = 6; }
					break;
			
			case 6: cube_clearLineX( y, z );
					if (y > 0 ) { y--; } else { z--; patternCtrl.state = 7; }
					break;
			
			case 7: cube_clearLineX( y, z );
					if (z > 0 ) { z--; } else { y=0; z=0; patternCtrl.state = 0; patternCtrl.cycles--;	}
					break;
			
			default: ;	
		}
		delay = patternDelay;

	}
}

void pattern_RandomFill() {
	uint8_t done;
	if (delay > 0) {
		delay--;
	} else {
		done = 0;
		switch (patternCtrl.state) {
			case 0:	if (loop < 450) {		//fill
						while (done == 0) {
							x = rand()%8;
							y = rand()%8;
							z = rand()%8;
							if (cube_getVoxel( x, y, z ) == 0) {
								cube_setVoxel(x,y,z);
								loop++;
								done=1;
							}
						}
				
					} else {
						loop = 0;
						patternCtrl.state = 1;
					}
					break;
			
			case 1:	if (loop < 445) {	//cleanup
						while (done == 0) {
							x = rand()%8;
							y = rand()%8;
							z = rand()%8;
							if (cube_getVoxel( x, y, z ) == 1) {
								cube_clearVoxel(x,y,z);
								loop++;
								done=1;
							}
						}
				
					} else {
						loop = 0;
						patternCtrl.state = 0;
						patternCtrl.cycles--;		//wird am Ende aller Zyklen = 0
					}
					break;
			default: ;
		}
		delay = patternDelay;
	}
}




void pattern_Test() {
	if (delay > 0) {
		delay--;
	} else {
		switch (patternCtrl.state) {
			case 0:	cube_clear();		//Anfang auf z=0  Ende auf y=7
					for (i=0; i<CUBE_MAX_X; i++ ) {
						cube_Line( i, y, 0, i, 7, z );
					}	
					y++;
					z++;
					if (y >= CUBE_MAX_Y) {		
						patternCtrl.state = 1;
						z = 0;
						y = 7;	
					}  
					break;
					
			case 1:	cube_clear();	//Anfang auf y=7  Ende auf z=7
					for (i=0; i<CUBE_MAX_X; i++ ) {
						cube_Line( i, y, 7, i, 7, z );
					}
					y--;
					z++;
					if (z >= CUBE_MAX_Z) {
						patternCtrl.state = 2;
						z = 7;
						y = 7;
					}
					break;
					
			case 2:	cube_clear();	//
					for (i=0; i<CUBE_MAX_X; i++ ) {
						cube_Line( i, 0, z, i, y, 7 );
					}
					y--;
					z--;
					if (z < CUBE_MAX_Z) {	//Unterlauf führt zu z=255
						patternCtrl.state = 3;
						z = 7;
						y = 0;
					}
					break;
			case 3:	cube_clear();	//
					for (i=0; i<CUBE_MAX_X; i++ ) {
						cube_Line( i, 0, z, i, y, 0 );
					}
					y++;
					z--;
					if (y >= CUBE_MAX_Y) {	
						patternCtrl.state = 0;
						z = 0;
						y = 0;
					}
					patternCtrl.cycles--;		//wird am Ende aller Zyklen = 0
					break;
					
			default: ;
		
		}
		delay = patternDelay;
	}
}


void pattern_do() {		//alle 10ms:
	switch (pattern) {
		case P_RUNNING_POINT:	pattern_RunningPoint();	
								if (patternCtrl.cycles == 0) {
									pattern_start( P_PLANES );
								}
								break;
		case P_PLANES:			pattern_Planes();		
								if (patternCtrl.cycles == 0) {
									pattern_start( P_CUBE_OUTLINE );
								}
								break;
		case P_CUBE_OUTLINE:	pattern_Cube( CUBE_EDGES );  
								if (patternCtrl.cycles == 0) {
									pattern_start( P_CUBE_WALL );
								}
								break;
		case P_CUBE_WALL:		pattern_Cube( CUBE_WALL );
								if (patternCtrl.cycles == 0) {
									pattern_start( P_CUBE_FILLED );
								}
								break;
		case P_CUBE_FILLED:		pattern_Cube( CUBE_FILLED );
								if (patternCtrl.cycles == 0) {
									pattern_start( P_RAIN );
								}
								break;
								
		case P_RAIN:			pattern_rain();
								if (patternCtrl.cycles == 0) {
									pattern_start( P_SPARKLE );
								}
								break;
								
		case P_SPARKLE:			pattern_SparkleFlash();								
								if (patternCtrl.cycles == 0) {
									pattern_start( P_TUNNEL );
								}
								break;
								
		case P_TUNNEL:			pattern_Tunnel();
								if (patternCtrl.cycles == 0) {
									pattern_start( P_RANDOMFILL );
								}
								break;

		case P_RANDOMFILL:		pattern_RandomFill();
								if (patternCtrl.cycles == 0) {
									pattern_start( P_RUNNING_POINT );
								}
								break;
								
								
		case P_TEST:			pattern_Test();
								if (patternCtrl.cycles == 0) {
//									pattern_start( P_RAIN );
									pattern_start( P_RUNNING_POINT );
								}
								break;
								
		case P_NONE:			break;
	}
}

void pattern_stop() {
	patternCtrl.bDirX=0;
	patternCtrl.bDirY=0;
	patternCtrl.bDirZ=0;
	patternCtrl.bStart=0;		//starten
	patternCtrl.bStop=0;
	patternCtrl.state=0;
	patternDelay = 5;
	pattern = P_NONE;
}

void pattern_start( TPatternTyp pt ) {
	pattern_stop();
	pattern = pt;
	cube_clear();
	uart_puts( "starting " ); uart_putc( pt ); uart_putc( '\n' );
	switch ( pt ) {
		case P_RUNNING_POINT:	x=0; y=0; z=0;
								patternCtrl.cycles = 1;		//nur 1 Runde
								cube_setVoxel(x,y,z);
								patternDelay = 2;		    //nur alle 2 * 10ms
								delay = patternDelay;
								break;						
								
		case P_PLANES:			x=0; y=0; z=0;
								patternCtrl.cycles = 3;
								cube_setPlaneX( x );
								patternCtrl.state = 0;
								patternDelay = 7;
								delay = patternDelay;
								break;
						
		case P_CUBE_OUTLINE:	x=0; y=0; z=0;
								patternCtrl.cycles = 2;
								patternCtrl.state = 0;
								patternDelay = 7;
								delay = patternDelay;
								break;						
								
		case P_CUBE_WALL:		x=0; y=0; z=0;
								patternCtrl.cycles = 2;
								patternCtrl.state = 0;
								patternDelay = 7;
								delay = patternDelay;
								break;
								
		case P_CUBE_FILLED:		x=0; y=0; z=0;
								patternCtrl.cycles = 2;
								patternCtrl.state = 0;
								patternDelay = 7;
								delay = patternDelay;
								break;
								
		case P_RAIN:			//for (x=0; x < CUBE_MAX_X; x++) {
								//	cube_setVoxel( x, x, 7 );
								//	cube_setVoxel( x, CUBE_MAX_Y-1-x, 7 );
								//}
								for (z=0; z < (rand()%4); z++ ) {
									cube_setVoxel( rand()%CUBE_MAX_X, rand()%CUBE_MAX_Y, CUBE_MAX_Z-1 );
								}
								patternCtrl.cycles = 100;
								patternCtrl.state = 2;		//Wie oft neue Regentropfen erzeugt werden sollen
								patternDelay = 10;
								delay = patternDelay;
								break;
								
		case P_SPARKLE:			x=20;	//max. nr of voxels
								y=1;	//start with this nr of voxels 
								z=5;	//loop every nr of voxels
								patternCtrl.cycles = 20;
								patternCtrl.state = 0;
								patternDelay = 5;
								delay = patternDelay;
								loop  = z;
								break;		
								
		case P_TUNNEL:			x=0; y=0; z=0;
								patternCtrl.cycles = 2;
								patternCtrl.state = 0;
								patternDelay = 5;
								delay = patternDelay;
								break;

		case P_RANDOMFILL:		x=0; y=0; z=0;								
								patternCtrl.cycles = 1;
								patternCtrl.state = 0;
								patternDelay = 2;
								delay = patternDelay;
								break;
								
								
								
		case P_TEST:			x=0; 
								y=0; 
								z=0;	
								patternCtrl.cycles = 4;
								patternCtrl.state = 0;
								patternDelay = 100;
								delay = patternDelay;
								loop = 0;
								break;									
													
		case P_NONE:	break;
	}
	
}











