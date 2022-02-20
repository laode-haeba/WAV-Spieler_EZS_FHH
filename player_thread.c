/*Irvan Rizqullah Fikardi (MN: 1484213) und Sinan Kocak (MN: 1502451)*/


#include "player_thread.h"
#include "dig_filter.h"
#include "echo.h"
#include <stdlib.h>

#define N 8192             /* Anzahl der El. im soundcard Buffer */


/****************** Threadfunktion und Funktionen, die der Thread nutzt *****/
PTL_THREAD_RET_TYPE WavPlayerThreadFunc(void* pt)
{   sRam_t parameter;   // fuer lokale Kopie des shared RAM

    FILE *fp;

    //Aufgabe: Soundkarte & Variablen initialisieren*************

    char address[512];
    int err = 0;        //Fehlerzeichnung
    int i = 0;          //Pufferzaehler
    int counter = 0;


    short buffer[N];    //Puffer

    SndDevice_t *psd;   //Soundkarte
    sndWaveHeader_t wh;
    sndStereo16_t st16;

    /*---Testvariablen fuer die Filtern-- Fikardi*/
    /*
    float test_fu = 20.0; //Fuer TP
    float test_fa = 44100.0; //Fuer TP, BP, HP
    float test_f0 = 500.0; //Fuer BP
    int test_Q = 5; //Fuer Q
    float test_fo = 5000; //Fuer fo
    IIR_2_coeff_t test_Ergebnis;

    //Ueberpruefen mit Testdaten in der Uebungsskript

    //test_Ergebnis = compute_TP_Filter_Parameters(test_fu,test_fa);
    //test_Ergebnis = compute_BP_Filter_Parameters(test_f0,test_Q,test_fa);
    //test_Ergebnis = compute_HP_Filter_Parameters(test_fo,test_fa);

    //printf("\n---Testergebnis---\na1: %f\na2: %f\nb0: %f\nb1: %f\nb2: %f",test_Ergebnis.a1,test_Ergebnis.a2,test_Ergebnis.b0,test_Ergebnis.b1,test_Ergebnis.b2);
    */
    /*-----------------------------------*/

    //***********************************************************

    printf("WAV-Player Thread ist gestartet...\n");

    //Aufgabe: Soundkarte-Check**********************************
    psd = sndOpen(SND_READ_WRITE, SND_STEREO);

    if(psd==NULL){
        printf("Fehler: Soundkarte nicht detektiert!");
        err = 1;
    }
    //***********************************************************



    do
    {   // neue parameter holen: Dateinamen, play, end, usw.
        parameter = set_param();

        //Puffer auf 0 setzen
        for(i=0;i<N;i++) {
            buffer[i] = 0;
        }

        // Wenn WAV-Datei gespielt wird
        if(parameter.cmd_play != 0) {
            //Aufgabe: WAV abspielen bzw. filtern *******************************
            strcpy(address, parameter.Dateiname);
            fp = fopen(address, "rb");

            if(fp == NULL){
                printf("WAV-Datei nicht gefunden\n");
                err = 1;
                break;
            }

            if(0!=sndWAVReadFileHeader(fp, &wh)){
                puts("Error in sndWAVReadFileHeader\n");
            }

            while((err == 0) && counter<sndWAVGetNumberOfSamples(wh) && parameter.cmd_play != 0){
                sndWAVReadSampleStereo16(fp, &st16); //abspielen im Stereo 16-Bit

                //Kalkulierte Werte im Puffer speichern
                buffer[i]   = st16.val_li;   //Siehe Skript Seite 16
                buffer[i+1] = st16.val_re;   //Siehe Skript Seite 16

                if(parameter.flag_Echo_is_active==1){    //Echo
                    st16 = echo_effect(st16,parameter.Echo);
                }

                //Wenn Filter vorhanden
                if(parameter.flag_EQ_is_active == 1){
                    st16.val_li = (short) EQ_filter_left((float)st16.val_li, parameter.TP, parameter.BP, parameter.HP, parameter.A_TP, parameter.A_BP, parameter.A_HP, parameter.B);
                    st16.val_re = (short) EQ_filter_right((float)st16.val_re, parameter.TP, parameter.BP, parameter.HP, parameter.A_TP, parameter.A_BP, parameter.A_HP, parameter.B);
                }

                //Lautstaerke
                buffer[i] = st16.val_li * parameter.volume * (1.0/15);
                buffer[i+1] = st16.val_re * parameter.volume * (1.0/15);

                //Counter update
                counter++;

                //WAV ausgeben wenn Puffer ausgefuellt ist
                if(i >= (N-2)){
                    sndWrite(psd, buffer, N);
                    //Puffer auf 0 setzen
                    for(i=0;i<N;i++) {
                        buffer[i] = 0;
                    }
                    //Pufferzaehler auf 0 setzen
                    i = 0;
                } else {
                    i += 2;
                }

                parameter = set_param(); //Parameter update
            }
            //********************************************************************

            //Datei schliessen
            counter = 0;
            i=0;
            fclose(fp);
        }
    } while(parameter.cmd_end == 0);
    // soundcard schliessen ...

    printf("WAV-Player Thread terminiert...");
    PTL_SemSignal(&endSema);

    return 0;
}
/* Prototyp der Funktionen, die der Thread nutzt */

/*---------------------------------------------*/

sRam_t set_param(){
    sRam_t out;
    PTL_SemWait(&sRamSema);
    out = sRam;
    PTL_SemSignal(&sRamSema);
    return out;
}

/*---------------------------------------------*/





