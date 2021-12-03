
#include <stdio.h>
#include <math.h>

#include "plotter_thread.h"
#include "dig_filter.h"



/****************** Threadfunktion und Funktionen, die der Thread nutzt *****/
PTL_THREAD_RET_TYPE ComputeFrequncyResponseThreadFunc(void* pt)
{   sRam_t parameter;  // fuer lokale Kopie des shared RAM


    printf("ComputeFrequncyResponseThreadFunc ist gestartet...");


    do
    {
        // neue parameter holen: Dateinamen, play, end, usw.
        PTL_SemWait(&sRamSema);
        parameter = sRam;
        PTL_SemSignal(&sRamSema);

        PTL_Sleep(0.5);

        /****************************************************/
        /* TODO: Amplitudengang berechnen                   */
        /****************************************************/

    } while(parameter.cmd_end == 0);

    printf("ComputeFrequncyResponseThreadFunc terminiert...");
    PTL_SemSignal(&endSema);

    return 0;
}
/* Prototyp der Funktionen, die der Thread nutzt */



/*---------------------------------------------*/
