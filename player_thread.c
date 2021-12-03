/* player_thread.c */


#include "player_thread.h"
#include "dig_filter.h"
#include "echo.h"

#define N 8192             /* Anzahl der El. im soundcard Buffer */


/****************** Threadfunktion und Funktionen, die der Thread nutzt *****/
PTL_THREAD_RET_TYPE WavPlayerThreadFunc(void* pt)
{   sRam_t parameter;  // fuer lokale Kopie des shared RAM


    printf("WAV-Player Thread ist gestartet...");
    // soundcard initialisieren ...


    do
    {   // neue parameter holen: Dateinamen, play, end, usw.
        PTL_SemWait(&sRamSema);
        parameter = sRam;
        PTL_SemSignal(&sRamSema);



    } while(parameter.cmd_end == 0);
    // soundcard schliessen ...

    printf("WAV-Player Thread terminiert...");
    PTL_SemSignal(&endSema);

    return 0;
}
/* Prototyp der Funktionen, die der Thread nutzt */



/*---------------------------------------------*/


/*---------------------------------------------*/





