/* globals.h */
/*Irvan Rizqullah Fikardi (MN: 1484213) und Sinan Kocak (MN: 1502451)*/

#ifndef _globals_h_
#define _globals_h_


#define N_PLOT_POINTS 512    /* Punkte Amplitudengang */
#define F_S           44100  /* Abtasfrequenz */

#include "ptl_lib.h"
#include "dig_filter.h"
#include "echo.h"

/* struct shared RAM */
typedef struct
{  char Dateiname[256];
   int cmd_play;  /* != 0 bedeutet: Datei abspielen */
   int cmd_end;   /* == 0 bedeutet: Thread soll weiterlaufen */
   int flag_Echo_is_active; /* ==0 bedeutet: ohne Echo */
   echo_params_t Echo;
   int flag_EQ_is_active;   /* ==0 bedeutet: ohne EQ */
   float A_TP,A_BP,A_HP; /* Gewichte Equalizer, Werte -1...10 */
   IIR_2_coeff_t TP,BP,HP;
   float B; /* Gewichtung nach Equ., Uebersteuerung vermeiden, 0<B<1 */
   float volume; //Lautstärke
   float Q;
}sRam_t;

/* struct shared RAM plot window */
typedef struct
{  float f_Hz[N_PLOT_POINTS];
   float H_dB[N_PLOT_POINTS];
}plot_data_t;

/* globale Daten */
extern sRam_t sRam;
extern plot_data_t plot_data;
extern PTL_sem_t sRamSema;
extern PTL_sem_t endSema;
extern PTL_sem_t plotSema;


#endif

