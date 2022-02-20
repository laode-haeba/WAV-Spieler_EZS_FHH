/*Irvan Rizqullah Fikardi (MN: 1484213) und Sinan Kocak (MN: 1502451)*/

#include <stdio.h>
#include <math.h>

#include "plotter_thread.h"
#include "dig_filter.h"

/****************** Threadfunktion und Funktionen, die der Thread nutzt *****/
PTL_THREAD_RET_TYPE ComputeFrequncyResponseThreadFunc(void* pt)
{   sRam_t parameter;  // fuer lokale Kopie des shared RAM
    unsigned int counter;


    printf("ComputeFrequncyResponseThreadFunc ist gestartet...");


    do
    {
        // neue parameter holen: Dateinamen, play, end, usw.
        PTL_SemWait(&sRamSema);
        parameter = sRam;
        PTL_SemSignal(&sRamSema);

        PTL_Sleep(0.1);

        /****************************************************/
        /* TODO: Amplitudengang berechnen                   */
        /****************************************************/
        /* fuer Bodediagramm von 1Hz bis 11kHz gibt es nur 512 Punkte.
        Counter faengt von 1 bis 512. 11000Hz/512 = 21.48, also ungefahr
        22 Hz fuer jeden Schritt.
        --Fikardi
        */

        PTL_SemWait(&plotSema);



        for(counter = 0; counter<N_PLOT_POINTS; counter++){
            if(counter == 0){
                plot_data.f_Hz[0] = (float) 1;
                plot_data.H_dB[0] = (float) H_ges_dB(parameter.TP, parameter.BP, parameter.HP, parameter.A_TP, parameter.A_BP, parameter.A_HP, parameter.B, 1, F_S) * parameter.volume * (1.0/15.0);
            } else {
                plot_data.f_Hz[counter] = (float) 22 * counter;
                plot_data.H_dB[counter] = (float) H_ges_dB(parameter.TP, parameter.BP, parameter.HP, parameter.A_TP, parameter.A_BP, parameter.A_HP, parameter.B, plot_data.f_Hz[counter], F_S) * parameter.volume* (1.0/15.0);
                //printf("\ncounter at: %i | f_Hz = %f \t | H_dB = %f",counter,plot_data.f_Hz[counter], plot_data.H_dB[counter]); //fuer Debug
            }
            //printf("\nCounter: %i | Wert im Array: %f", counter, plot_data.H_dB[counter]);
        }
        PTL_SemSignal(&plotSema);
        counter = 0;
    } while(parameter.cmd_end == 0);



    printf("ComputeFrequncyResponseThreadFunc terminiert...");
    PTL_SemSignal(&endSema);

    return 0;
}
/* Prototyp der Funktionen, die der Thread nutzt */



/*---------------------------------------------*/
