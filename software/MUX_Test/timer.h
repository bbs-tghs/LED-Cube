#ifndef _TIMER_
#define _TIMER_
/**
  @file timer.h
  
  @brief  Alle Routinen f�r den Timerbetrieb
  
  Die Bibliothek stellt eine Reihe von Flags zur Verf�gung, die in bestimmten Intervallen gesetzt werden. 
  Der Benutzer kann diese Flags in seinem Hauptprogramm pr�fen und eine Aktion ausf�hren, wenn es gesetzt ist. 
  Der Benutzer muss das Flag wieder l�schen. Auch dazu werden Funktionen bereitgestellt.
  
  Der Timerinterrupt setzt in der globalen Variablen timerFlags je ein Flag f�r das Auftreten des entsprechenden Events.
  Die Flags m�ssen nach der Bearbeitung des Events vom Hauptprogramm gel�scht werden.
  Dadurch kann die Eventbearbeitung auch l�nger dauern als die Interruptzeit erlauben w�rde (asynchron)

  Synchrone Verarbeitung ist �ber die DoXXX Prozeduren m�glich.
  
  Timer0 -> internes Taktraster
  
  Created: 27.05.2011 08:27:44

  @author  Michael Busser  

*/


struct TFlagsTimer {
  unsigned bMilli:1;    //- Flag f�r 1/1000 Sekunde vergangen
  unsigned b2Milli:1;	//- Flag f�r 2/1000 Sekunde vergangen
  unsigned b5Milli:1;	//- Flag f�r 5/1000 Sekunde vergangen
  unsigned bCenti:1;    //- Flag f�r 1/100 Sekunde
  unsigned bDezi:1;     //- Flag f�r 1/10 Sekunde
  unsigned bSek:1;      //- Flag f�r 1 Sekunde
  unsigned b10Sek:1;    //- Flag f�r 10 Sekunden sind vergangen
  unsigned bMin:1;      //- Flag f�r 1 Minute ist vergangen
};


union TTimer {
  unsigned char         all;
  struct   TFlagsTimer  flags;
};


typedef void (*timer_callback_t) (void);		//Event Callback-Funktion, es werden keine Parameter �bergeben

//-----------------------------------------------------------
// Flags markieren das Event
//-----------------------------------------------------------
/**
 @brief Die Variable timerflags ist eine Struktur, in der jedes Bit �ber einen Namen angesprochen werden kann.
        Der Timerinterrupt setzt das Bit, wenn die zugeordnete Zeit abgelaufen ist.
		timerFlags.b5Mill wird also alle 5 Millisekunden gesetzt. Das Flag kann jetzt einfach gepr�ft werden.
		Das flag muss wieder gel�scht werden. Dazu gibt es f�r jedes Flag eine Funktion mit �hnlichem Namen.
		Ein Aufruf von timerClear5Milli() l�scht also das Flag f�r den 5-Millisekunden-Takt.
*/
volatile union TTimer timerFlags;

void timerInit( void );
void timerClearAll( void );
void timerClearMilli( void );
void timerClear2Milli( void );
void timerClear5Milli( void );
void timerClearCenti( void );
void timerClearDezi( void );
void timerClearSek( void );
void timerClear10Sek( void );
void timerClearMin( void );

void register_Timer_Callback_1MS( timer_callback_t cbf );

#endif	//_TIMER_