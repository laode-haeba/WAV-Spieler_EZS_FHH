/*****************************************************************
  Projekt-Name    : ringbuffer
  File-Name         : ringbuffer.c
  Programm-Zweck  :  Implementierung eines Ringbuffers fuer short-Werte.

  Der Einfachheit halber wird hier auf Pruefungen wie "rungbuffer voll"
  oder "leer" verzichtet. Der Ringbuffer hat eine feste Länge, die einer
  maximalen Verzögerung von 1sec entspricht.
  Ringbuffer gefuellt mit Nullen, maximales Delay:
    0   1                                                         len-1
  ---------------------------------------------------------------------
  | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
  ---------------------------------------------------------------------
    |                        ------->                               |
   rd                     Arbeitsrichtung                          wr
  nach einem Lesezugriff:
    0   1                                                         len-1
  ---------------------------------------------------------------------
  | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
  ---------------------------------------------------------------------
        |                    ------->                               |
       rd                 Arbeitsrichtung                          wr
  nach einem Schreibzugriff:
    0   1                                                         len-1
  ---------------------------------------------------------------------
  | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | x |
  ---------------------------------------------------------------------
    |   |                    ------->
   wr  rd                 Arbeitsrichtung
Die beiden Indizes laufen zirkular durch das Feld, was durch
inkrementieren und anschliessendes Modulo len erreicht wird.

  Erstellt durch  : Fgb
  Erstellt am     : 20.03.2007
  Historie: 20.3.2007 erstellt, Modul ringbuffer entwickelt.
            29.7.2020 angepasst für Wav-Player 2.0
 *****************************************************************/


#include <stdio.h>
#include <stdlib.h>


#include "echo.h"
#include "globals.h"

#define N_BUF F_S     /* F_S ist die Abtastfrequenz in Hz, d.h. 1 Sekunde max Delay */

typedef struct {
    short buf[N_BUF];  /* max 1sec Delay */
    int wr;	/* next write at this index */
    int rd;     /* next read at this index */
}RingBufferShort_t;

static RingBufferShort_t rb={0};

static void WriteToRingBufferShort(short x);
static short ReadFromRingBufferShort(void);




/***********************************************************************/
/* TODO: Echo implementieren                                           */
/***********************************************************************/

