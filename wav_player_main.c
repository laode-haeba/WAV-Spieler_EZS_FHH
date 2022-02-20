/*Irvan Rizqullah Fikardi (MN: 1484213) und Sinan Kocak (MN: 1502451)*/

#include <stdio.h>
#include <string.h>
#include "ptl_lib.h"
#include "snd_lib.h"
#include "globals.h"
#include "player_thread.h"
#include "plotter_thread.h"
#include "gui.h"



/* globale Daten */
sRam_t sRam;
plot_data_t plot_data;
PTL_sem_t sRamSema;
PTL_sem_t endSema;
PTL_sem_t plotSema;


/* Prototypen der Funktionen die main()  benutzt*/
void CreateSemaphores(void);
void InitGlobals(void);
void UserInterface(void);
int  PrintMenue(void);
void ExecuteMenue(int c);

/*---------------------------------------------*/

int main(int argc, char *argv[])
{   PTL_thread_t ThreadID, PlotterThreadID;

    printf("WAV-Player Version 2.0\n");


    /* globale Daten initialisieren, create semaphores */
    CreateSemaphores();
    InitGlobals();
    /* thread starten */
    if(0!=PTL_CreateThread(&ThreadID, WavPlayerThreadFunc, NULL))
    { puts("error starting thread");
      return -1;
    }
    if(0!=PTL_CreateThread(&PlotterThreadID, ComputeFrequncyResponseThreadFunc, NULL))
    { puts("error starting thread");
      return -1;
    }


#if 0
    /* Textmenue */
    UserInterface();
    /* Thread-Ende kommandieren: */
    PTL_SemWait(&sRamSema);
    sRam.cmd_play = 0;
    sRam.cmd_end  = 1;
    PTL_SemSignal(&sRamSema);
    puts("WAV-Player: main() wartet auf das Ende des Player-Threads...\n");
    PTL_SemWait(&endSema);
    puts("WAV-Player: main() wartet auf das Ende des Plotter-Threads...\n");
    PTL_SemWait(&endSema);
    puts("WAV-Player: main() ist beendet...\n");
#else
    gui(argc, argv);
#endif
    return 0;
}
/*---------------------------------------------*/
/*********** Funktionen die main()  benutzt ********************************/
void CreateSemaphores(void)
{   PTL_SemCreate(&sRamSema,1);
    PTL_SemCreate(&endSema,0);
    PTL_SemCreate(&plotSema,1);
}
/*---------------------------------------------*/
void InitGlobals(void)
{   int i;

    strcpy(sRam.Dateiname, "E:/ezs/wav_player_v2_getting_started/wav/PhilCol.wav"); //zum testen ob Option B geht oder nicht
    sRam.cmd_play = 0;                     // nicht spielen
    sRam.cmd_end  = 0;                     // Thread soll weiter laufen
    sRam.flag_EQ_is_active =0;   /*  ohne EQ */
    sRam.flag_Echo_is_active =0; /*  ohne Echo */
    sRam.A_TP = 0;
    sRam.A_BP = 0;
    sRam.A_HP = 0;
    sRam.B = 1.0;

    sRam.volume = 5;  //sonst muss Lautstäre erstmal eingesellet werden,
                      // bevor sound aus dem Lautsprecher kommt

    for(i=0; i< N_PLOT_POINTS; i++)
    {   plot_data.f_Hz[i] = 0;
        plot_data.H_dB[i] = 0;
    }

    sRam.TP.a1=0;
    sRam.TP.a2=0;
    sRam.TP.b0=0;
    sRam.TP.b1=0;
    sRam.TP.b2=0;
    sRam.BP = sRam.TP;
    sRam.HP = sRam.TP;

}
/*---------------------------------------------*/
void UserInterface(void)
{   int c=0;
    while ((c!='q') && (c!='Q'))
    {   c=PrintMenue();
        ExecuteMenue(c);
    }
}
/*---------------------------------------------*/
int  PrintMenue(void)
{   int c;
    PTL_SemWait(&sRamSema);
    printf("\nWAV-Player  Datei: %s\n", sRam.Dateiname);
    PTL_SemSignal(&sRamSema);
    printf("a: Name der WAV-Datei eingeben\n");
    printf("b: play\n");
    printf("c: stop\n");
    printf("q: Programmende\n");
    printf("-------------------\n");
    printf(">:");
    fflush(stdin);
    c=getchar();
    return c;
}
/*---------------------------------------------*/
void ExecuteMenue(int c)
{   char Name[128];
    switch(c)
    {   case 'a':
        case 'A': printf("Dateiname: ");
                  fflush(stdin);
                  scanf("%s", Name);
                  /* hier konnte man testen, ob es die Datei wirklich gibt ?!*/
                  PTL_SemWait(&sRamSema);
                  strcpy(sRam.Dateiname, Name);
                  PTL_SemSignal(&sRamSema);
                  break;
        case 'b':
        case 'B': PTL_SemWait(&sRamSema);
                  sRam.cmd_play = 1;
                  PTL_SemSignal(&sRamSema);
                  break;
        case 'c':
        case 'C': PTL_SemWait(&sRamSema);
                  sRam.cmd_play = 0;
                  PTL_SemSignal(&sRamSema);
                  break;
        case 'q':
        case 'Q': puts("Ende einleiten...");break;
        default:  printf("unbekanntes Kommando!");
                  break;
    }
}
/*---------------------------------------------*/

