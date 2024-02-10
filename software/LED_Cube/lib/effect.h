/*
 * effect.h
 *
 * Created: 01.05.2017 15:17:04
 *  Author: busser.michael
 */ 


#ifndef EFFECT_H_
#define EFFECT_H_


typedef enum {
	P_RUNNING_POINT,
	P_PLANES,
	P_CUBE_OUTLINE,
	P_CUBE_WALL,
	P_CUBE_FILLED,
	P_RAIN,
	P_SPARKLE,
	P_TUNNEL,
	P_RANDOMFILL,
	P_TEST,
	P_NONE
} TPatternTyp;

typedef enum {
	CUBE_FILLED,
	CUBE_WALL,
	CUBE_EDGES
} TCUBETYPE;

typedef struct {
	unsigned bDirX:1;		//Direction in X    0=increment		1=Decrement
	unsigned bDirY:1;		//Direction in Y    0=increment		1=Decrement
	unsigned bDirZ:1;		//Direction in Z    0=increment		1=Decrement
	unsigned bStart:1;	    //1=soll gestartet werden
	unsigned bStop:1;		//1=soll gestoppt werden
	unsigned state:3;	    //für Zustände eines Pattern
	uint8_t  cycles;		//Wie oft soll das Muster durchlaufen
} TPatternCtrl;


void pattern_do();						//Wird alle 10ms aufgerufen und sorgt für den Ablauf des Musters
void pattern_stop();					//Beendet das laufende Muster
void pattern_start( TPatternTyp pt );	//Startet das angegebene Muster

#endif /* EFFECT_H_ */