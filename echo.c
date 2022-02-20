/*Irvan Rizqullah Fikardi (MN: 1484213) und Sinan Kocak (MN: 1502451)*/
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
    int wr;	           /* "next write" vom zurzeitigen Indiz */
    int rd;            /* "next read" vom zurzeitigen Indiz */
}RingBufferShort_t;

static RingBufferShort_t rb={0};

static void WriteToRingBufferShort(short x){
    //"Die beiden Indizes laufen zirkular durch das Feld, was durch
    //inkrementieren und anschliessendes Modulo len erreicht wird."

    //Eingabe wird im naechsten Zelle gespeichert
    rb.buf[rb.wr] = x;

    //Indiz nimmt jetzt einen Schritt weiter bis Ende
    rb.wr ++;

    // Wenn Indiz hat die letzte Zelle erreicht, ist die Position zu Null zurueckgesetzt
    rb.wr = rb.wr % N_BUF;
}

static short ReadFromRingBufferShort(void){
    short out;

    //Ausgabe beginnt beim Lesen von erster Zelle
    out = rb.buf[rb.rd];

    //Inkrementieren
    rb.rd ++;

    //Bis Indiz gleiche Werte mit N_BUF hat, dann zu 0 zurueck
    rb.rd = rb.rd % N_BUF;
    return out;
}

/***********************************************************************/
/* TODO: Echo implementieren                                           */
/***********************************************************************/

sndStereo16_t echo_effect(sndStereo16_t x, echo_params_t p){
    sndStereo16_t sound_out;
    short current_echo;
    float buffer_input;
    //Referenz: Uebungsskript Seite 25, 26

    //Ringpuffer initialisieren
    rb.wr = rb.rd + p.delay_n0;
    rb.wr = (rb.wr % N_BUF);

    current_echo = ReadFromRingBufferShort();

    //Echo im Puffer schreiben
    buffer_input = ((float)(x.val_li + x.val_re) * p.gain + (current_echo * p.feedback)) ; //Bug? nicht so sicher bei x.val_li + x.val_re da es zu laut ist...
    WriteToRingBufferShort(buffer_input);

    //Ausgabe zur Soundkarte
    sound_out.val_li = x.val_li + current_echo;
    sound_out.val_re = x.val_re + current_echo;

    return sound_out;
}
