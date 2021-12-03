/*****************************************************************
  Projekt-Name    : Echo
  File-Name         : echo.h
  Programm-Zweck  :  Implementierung eines Ringbuffers fuer short-Werte.


  Erstellt durch  : Fgb
  Erstellt am     : 20.03.2007
  Historie: 20.3.2007 erstellt, Modul ringbuffer entwickelt.
            29.7.2020 angepasst für Wav-Player 2.0
 *****************************************************************/
#ifndef ringbuffer_h_
#define ringbuffer_h_

#include "snd_lib.h"


typedef struct
{   int delay_n0;      /* 0...F_S-1 */
    float gain;        /* 0...1 */
    float feedback;    /* 0...1 */
}echo_params_t;


sndStereo16_t echo_effect(sndStereo16_t x, echo_params_t p);

#endif


