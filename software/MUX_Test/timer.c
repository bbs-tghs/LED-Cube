/**
@defgroup internalHW Chipinterne Hardwareeinheiten
@{
	*/


/**
@defgroup timer Timer
@{
	@brief	Alle Routinen für den Timerbetrieb
	
	@detail	Die Bibliothek stellt eine Reihe von Flags zur Verfügung, die in bestimmten Intervallen gesetzt werden. 
			Der Benutzer kann diese Flags in seinem Hauptprogramm prüfen und eine Aktion ausführen, wenn es gesetzt ist. 
			Der Benutzer muss das Flag wieder löschen. Auch dazu werden Funktionen bereitgestellt 
	
	@details Timer Library.
	
	@author Michael Busser
	@date 2011
	@copyright LGPL 2.0
		
*/


#include <avr\io.h>
#include <avr\interrupt.h>
#include <stddef.h>
#include <inttypes.h>

#include "timer.h"


//------------------------------------------------------------------------- 
// externe Variablen
//------------------------------------------------------------------------- 


//------------------------------------------------------------------------- 
// interne Variablen
//------------------------------------------------------------------------- 

#define TIMER_FREQ	1000UL

volatile unsigned char  cnt2Milli = 2;              ///@brief Der Vorteiler für die 2/1000-tel Sekunden
volatile unsigned char  cnt5Milli = 5;              ///@brief Der Vorteiler für die 5/1000-tel Sekunden
volatile unsigned char  cntcenti  = 10;             ///@brief Der Vorteiler für die 100-tel Sekunden 
volatile unsigned char  cntdezi   = 10;             ///@brief Der Vorteiler für die 10-tel Sekunden 
volatile unsigned char  cntsek    = 10;             ///@brief Der Vorteiler für die Sekunden 
volatile unsigned char  cnt10sek  = 10;			    ///@brief Der Vorteiler für 10 Sekunden	
volatile unsigned char  cntMin    = 6;			    ///@brief Der Vorteiler für 1 Minute

/** 
 @brief Variablen für die Adressen der Callback-Funktionen.
		Diese müssen mit den entsprechenden register-Funktionen registriert werden.
		Derzeit ist je Taktschritt nur eine Callback-Funktion möglich.
		Es sind nur die Taktschritte 1ms, 10ms und 100ms als Callback-Funktion verfügbar.
		Eine Ergänzung ist leicht möglich.
*/
timer_callback_t	CB_Event_1MS = NULL;			///@brief Adresse Callback-Routine bei 1ms
timer_callback_t	CB_Event_10MS = NULL;			///@brief Adresse Callback-Routine bei 10ms
timer_callback_t	CB_Event_100MS = NULL;			///@brief Adresse Callback-Routine bei 100ms

//------------------------------------------------------------------------- 
// 
//------------------------------------------------------------------------- 
/** 
 @brief Initialisierung des Timers 0 für den Takt \n
		Bisher: 16MHz / ( 64 * 1000Hz ) -1 = 249 \n
		Jetzt:	((uint32_t)(F_CPU) / (64UL * 1000UL)) - 1 \n
		Die Bestimmung des Wertes für das Register OCR0 wird jetzt mit Hilfe des 
		Symbols F_CPU ausgeführt, anstelle eines festen Wertes von 16MHz. \n
		Die Funktion muss zum Programmstart einmal aufgerufen werden.
*/
void timerInit( void ) {
  timerFlags.all = 0;                       			// Alle Flags auf einmal zurücksetzen  

  TCCR0 = (1<<WGM01) | (1 << CS01) | (1 << CS00);		// CTC-Modus aktivieren, Vorteiler=64
  //OCR0  = (249);                     					// Ergibt eine Aufruffrequenz des IRQ-Handlers von 1000Hz bzw. 1ms
  OCR0 = ((uint32_t)(F_CPU) / (64UL * TIMER_FREQ)) - 1;
  TIMSK = (1 << OCIE0) | (1<<TOIE0);					// Interrupt aktivieren bei Timer0-Compare
}
//-------------------------------------------------------------------------
// alle 1 ms
//-------------------------------------------------------------------------
void doMilli( void ) {
	if (CB_Event_1MS != NULL) { CB_Event_1MS(); }
}
//-------------------------------------------------------------------------
// alle 2 ms
//-------------------------------------------------------------------------
void do2Milli( void ) {

}
//-------------------------------------------------------------------------
// alle 5 ms
//-------------------------------------------------------------------------
void do5Milli( void ) {

}
//------------------------------------------------------------------------- 
// alle 10 ms
//------------------------------------------------------------------------- 
void doCenti( void ) {

}
//------------------------------------------------------------------------- 
// alle 100 ms
//------------------------------------------------------------------------- 
void doDezi( void ) {
  
}
//------------------------------------------------------------------------- 
// alle 1000 ms   bzw. 1 Sekunde
//------------------------------------------------------------------------- 
void doSek( void ) {


}
//------------------------------------------------------------------------- 
// alle 10 s 
//------------------------------------------------------------------------- 
void do10Sek( void ) {

}
//------------------------------------------------------------------------- 
// jede Minute
//------------------------------------------------------------------------- 
void doMin( void ) {

}
//------------------------------------------------------------------------- 
/** 
 @brief Interrupt-Service-Routine des Timers
		 Wird alle 1 ms aufgerufen. 
		 Aus dieser Routine leiten sich die anderen Takte ab.
*/
ISR(TIMER0_COMP_vect) {                               //  wird alle 1ms aufgerufen 

  timerFlags.flags.bMilli = 1;
  doMilli();
  
  cnt2Milli--;
  if (cnt2Milli == 0) {
	  timerFlags.flags.b2Milli = 1;
	  cnt2Milli = 2;
	  do2Milli();
  }
  
  cnt5Milli--;
  if (cnt5Milli == 0) {
	timerFlags.flags.b5Milli = 1;	  
	cnt5Milli = 5;
	do5Milli();
  }
  
  cntcenti--;
  if (cntcenti == 0) { 
	timerFlags.flags.bCenti = 1;
  	cntcenti = 10; 
	doCenti();
	
	cntdezi--;
	if (cntdezi == 0) {
	  timerFlags.flags.bDezi = 1;
	  cntdezi = 10;
	  doDezi();

	  cntsek--;
	  if (cntsek == 0) {
		timerFlags.flags.bSek = 1;
		cntsek = 10;
		doSek();

		cnt10sek--;
        if ( cnt10sek == 0) {
  		  timerFlags.flags.b10Sek = 1;
		  cnt10sek = 10;
		  do10Sek();

		  cntMin--;
		  if ( cntMin == 0) {
  		    timerFlags.flags.bMin = 1;
		    cntMin = 6;
		    doMin();
		  }
		}
	  }
	}
  }
}
//------------------------------------------------------------------------- 
void timerClearAll( void ) {
  timerFlags.all = 0;   // Alle Flags auf einmal zurücksetzen  
  cnt2Milli = 2;
  cnt5Milli = 5;		// und die Vorteiler nicht vergessen
  cntcenti  = 10;        
  cntdezi   = 10;            
  cntsek    = 10;             
  cnt10sek  = 10;				
  cntMin    = 6;		
}
//------------------------------------------------------------------------- 
void timerClearMilli( void ) {
  timerFlags.flags.bMilli = 0;
}
//-------------------------------------------------------------------------
void timerClear2Milli( void ) {
	timerFlags.flags.b2Milli = 0;
}
//-------------------------------------------------------------------------
void timerClear5Milli( void ) {
	timerFlags.flags.b5Milli = 0;
}
//------------------------------------------------------------------------- 
void timerClearCenti( void ) {
  timerFlags.flags.bCenti = 0;
}
//------------------------------------------------------------------------- 
void timerClearDezi( void ) {
  timerFlags.flags.bDezi = 0;
}
//------------------------------------------------------------------------- 
void timerClearSek( void ) {
  timerFlags.flags.bSek = 0;
}
//-------------------------------------------------------------------------
void timerClear10Sek( void ) {
	timerFlags.flags.b10Sek = 0;
}
//------------------------------------------------------------------------- 
void timerClearMin( void ) {
  timerFlags.flags.bMin = 0;
}
//------------------------------------------------------------------------- 



void register_Timer_Callback_1MS( timer_callback_t cbf ) {
  CB_Event_1MS = cbf;	
}


/**@}*/
/**@}*/