/*!
 ********************************************************************
   @file            snd_lib.h
   @par Project   : Sound Utilities, Linux / Windows
   @par Module    : low level sound device prgramming

   @brief


   @par Author    : Fgb
   @par Department: EIT-PIA
   @par Company   : Fachhochschule Hannover - University of Applied
                    Sciences and Arts

 ********************************************************************

   @par History   :
   @verbatim
   Version | Date        | Author | Change Description
   0.0     | 27.08. 2007 | Fgb    | First Version, Linux / Win32
   1.0     | 04.06.2020  | Fgb    | ALSA on Linux
   @endverbatim

 ********************************************************************/

/*! \mainpage snd lib

  \section intro Einfuehrung

    Dieses Modul stellt Funktionen zum Betrieb der Soundkarte unter
    Linux und Windows zur Verfuegung. Zugriff auf WAV-Dateien wird
    ebenfalls unterstuetzt.


  \section install Benutzung

    Um das Modul zu benutzen muss die Datei snd_lib.h
    per ''include'' eingebunden und die Quellcodedate
    snd_lib.c in das Projekt aufgenommen, compiliert
    und gelinkt werden.

    Das benutzte Betriebssystem (Linux oder Windows) muss in der Datei
    'snd_codecontrol.h' eingetragen werden. Siehe Beschreibung in
    dieser Datei. Die Voreinstellung ist Windows.

    Die exportierten Datenstrukturen und Funktionen sind in der
    Datei snd_lib.h dokumentiert.


    \section Einige Hinweise

    Es wird nur eine Abtastfrequenz (44100Hz) und ein
    Datenformat (16 Bit mit Vorzeichen, d.h. ''signed short'')
    unterstuetzt. Einstellbar ist lediglich der Aufnahme-
    Wiedergabemodus
    (''SND_READ_ONLY'', ''SND_WRITE_ONLY'' oder ''SND_READ_WRITE'')
    sowie die Anzahl der Kanaele (''SND_MONO'' oder ''SND_STEREO'').
    Die Datenquelle (Mic, Line) kann nicht eingestellt werden, dies ist
    mit den Mixer-Applikationen des Betriebssystems zu machen.


    Bug-Reports, Gejammer, troestende Worte und hilfreiche Kommentare
    bitte an ernst.forgber@hs-hannover.de schicken.

 **********************************************************************/





#ifndef _win_snd_h_h_
#define _win_snd_h_h_

#include "snd_codecontrol.h"

#if (PLATFORM==OS_LINUX)
  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/ioctl.h>
  #if LINUX_OSS
    #include <linux/soundcard.h>  /* OSS support */
  #endif
  #if LINUX_ALSA
    /* Use the newer ALSA API */
    #define ALSA_PCM_NEW_HW_PARAMS_API
    #include <alsa/asoundlib.h>   /* ALSA Support */
    #define SND_BUFFER_SIZE_BYTE 16384 /*! Sound buffer size in bytes */
  #endif
#endif

#if (PLATFORM==OS_MS_WINDOWS)
  #include <stdio.h>
  #include <stdlib.h>
  #include <windows.h>
  #include <limits.h>
  #include <mmsystem.h>
#endif

/* ----------------public, exported defines -------------------- */
#define SND_READ_ONLY   0   /*! Arbeitsmodus der Soundkarte: nur Aufnahme */
#define SND_WRITE_ONLY  1   /*! Arbeitsmodus der Soundkarte: nur Wiedergabe */
#define SND_READ_WRITE  2   /*! Voll-Duplex Modus: Aufnahme und Wiedergabe gleichzeitig */
#define SND_MONO        1   /* don't change! Anzahl der Kanaele, Mono */
#define SND_STEREO      2   /* don't change! Anzahl der Kanaele, Stereo */




/* ----------------public, exported types ----------------------*/

#if (PLATFORM==OS_LINUX)
  typedef struct {
    int nChannels;  /*! number of channels 1:mono 2:stereo */
    int rw_mode;    /*! SND_READ_ONLY, SND_WRITE_ONLY or SND_READ_WRITE */
  #if LINUX_OSS
    int fd;         /*! file descriptor of sound device */
  #endif // LINUX_OSS
  #if LINUX_ALSA
    snd_pcm_t *pcm_handle_playback; /*! Handle for the PCM playback device */
    snd_pcm_t *pcm_handle_capture;  /*! Handle for the PCM capture device */
    int buffer_size_frames;
    int frame_size_bytes;
  #endif // LINUX_ALSA
  }SndDevice_t;
#endif

#if (PLATFORM==OS_MS_WINDOWS)
  typedef struct {
      int nChannels;  /*! number of channels 1:mono 2:stereo */
      int rw_mode; /*! mode of sound device SND_READ_ONLY, SND_WRITE_ONLY, SND_READ_WRITE */
      void *pt;  /*! pointer to interal device struture */
  }SndDevice_t;
#endif





/*---------------- public, exported functions --------------------*/
/*!
 ********************************************************************
  @par Beschreibung:
    Oeffnet die Soundkarte im gewaehlten Modus. Das Datenformat ist
    16Bit mit Vorzeichen (signed short), die Abtastrate ist 44100kHz.
    Diese Funktion muss vor der Benutzung der Soundkarte einmalig
    aufgerufen werden (alloziert auch intern genutzten Speicher).

  @see
  @arg sndClose

  @param  rw_mode -     IN, Datenrichtung: SND_READ_ONLY,SND_WRITE_ONLY
                       oder SND_READ_WRITE
  @param  mono_stereo -  IN, Kanalanzahl: SND_MONO oder SND_STEREO

  @retval Zeiger auf Struktur SndDevice_t mit Daten zur Soundkarte wenn
         kein Fehler auftritt, sonst NULL bei Fehler.
  @par Beispiel :
     Die Soundkarte wird zum Lesen und Schreiben geoeffnet. In
     einer Schleife werden Daten aufgenommen und gleich wieder ausgegeben.
  @verbatim
#include <stdio.h>
#include <stdlib.h>
#include "snd_lib.h"
#define N 2048

int main(void)
{   SndDevice_t *psd;
    short buf[N];
    int j;

    psd = sndOpen(SND_READ_WRITE, SND_STEREO);
    for(j=0; j<500; j++)
    {   printf("loop counter:%d\n", j);
        if(N != sndRead(psd, buf, N))
        {   printf("win_sndRead crashed...");
        }
        if(N != sndWrite(psd, buf, N))
        {   printf("sndWrite crashed...");
        }
    }
    psd = sndClose(psd);
    return 0;
}
  @endverbatim
 ********************************************************************/
SndDevice_t *sndOpen(int rw_mode, int mono_stereo);



/*!
 ********************************************************************
  @par Beschreibung:
    Schliesst die Soundkarte. Diese Funktion muss nach Abschluss der
    Benutzung der Soundkarte einmalig aufgerufen werden (gibt zuvor
    allozierten intern genutzten Speicher frei).

  @see
  @arg sndOpen

  @param  sd -     IN/OUT, Zeiger auf Geraetestruktur der geoffneten
                   Soundkarte

  @retval  NULL
  @par Beispiel :

  @verbatim
    ...
    SndDevice_t *psd;

    psd = sndOpen(SND_READ_WRITE, SND_STEREO);
    ...
    psd = sndClose(psd);

  @endverbatim
 ********************************************************************/
SndDevice_t *sndClose(SndDevice_t *sd);



/*!
 ********************************************************************
  @par Beschreibung:
    Liest blockierend von der Soundkarte. Das Datenformat ist
    16Bit mit Vorzeichen (signed short), die Abtastrate ist 44100kHz.
    Mono-Daten liegen nacheinander im short-Feld, Stereo-Daten
    werden abwechselnd Links,Rechts ins Feld geschrieben. Ein
    Stereo-Wertepaar (Links,Rechts) zaehlt als zwei Elemente, da es
    zwei short-Speicherplaetze belegt.

    Linux: die Anzahl der gelesenen
    Elemente ist nicht beliebig, auf manchen Rechner kam es zu
    Abstuerzen, wenn die Puffergroesse kleiner war als der intern
    Soundkartenpuffer. Als guenstig hat sich eine Puffergroesse von
    4096 Byte oder vielfache davon erwiesen. Das short-Feld sollte also
    mindestens 2048 Elemente haben.

  @see
  @arg sndWrite, sndOpen

  @param  sd    -  Zeiger auf Geraetestruktur der geoffneten Soundkarte
  @param  buf   -  Zeiger auf Puffer fuer die zu lesenden Daten
  @param  buf_elements - Anzahl der Speicherplaetze im short-Feld

  @retval Anzahl der gelesenen Feldelemente oder negative Zahl bei Fehler.

  @par Beispiel :
     Die Soundkarte wird zum Lesen und Schreiben geoeffnet. In
     einer Schleife werden Daten aufgenommen und gleich wieder ausgegeben.
  @verbatim
#include <stdio.h>
#include <stdlib.h>
#include "snd_lib.h"
#define N 2048

int main(void)
{   SndDevice_t *psd;
    short buf[N];
    int j;

    psd = sndOpen(SND_READ_WRITE, SND_STEREO);
    for(j=0; j<500; j++)
    {   printf("loop counter:%d\n", j);
        if(N != sndRead(psd, buf, N))
        {   printf("win_sndRead crashed...");
        }
        if(N != sndWrite(psd, buf, N))
        {   printf("sndWrite crashed...");
        }
    }
    psd = sndClose(psd);
    return 0;
}
  @endverbatim
 ********************************************************************/
int sndRead(SndDevice_t *sd, short *buf, int buf_elements);




/*!
 ********************************************************************
  @par Beschreibung:
    Schreibt blockierend auf die Soundkarte. Das Datenformat ist
    16Bit mit Vorzeichen (signed short), die Abtastrate ist 44100kHz.
    Mono-Daten liegen nacheinander im short-Feld, Stereo-Daten
    liegen abwechselnd Links,Rechts im Feld. Ein
    Stereo-Wertepaar (Links,Rechts) zaehlt als zwei Elemente, da es
    zwei short-Speicherplaetze belegt.

    Linux: die Anzahl der zu schreibenden
    Elemente ist nicht beliebig, auf manchen Rechner kam es zu
    Abstuerzen, wenn die Puffergroesse kleiner war als der intern
    Soundkartenpuffer. Als guenstig hat sich eine Puffergroesse von
    4096 Byte oder vielfache davon erwiesen. Das short-Feld sollte also
    mindestens 2048 Elemente haben.

  @see
  @arg sndRead, sndOpen

  @param  sd    -  Zeiger auf Geraetestruktur der geoffneten Soundkarte
  @param  buf   -  Zeiger auf Puffer mit den zu schreibenden Daten
  @param  buf_elements - Anzahl der Speicherplaetze im short-Feld

  @retval Anzahl der geschriebenen Feldelemente oder negative Zahl bei Fehler.
  @par Beispiel :
     Die Soundkarte wird zum Lesen und Schreiben geoeffnet. In
     einer Schleife werden Daten aufgenommen und gleich wieder ausgegeben.
  @verbatim
#include <stdio.h>
#include <stdlib.h>
#include "snd_lib.h"
#define N 2048

int main(void)
{   SndDevice_t *psd;
    short buf[N];
    int j;

    psd = sndOpen(SND_READ_WRITE, SND_STEREO);
    for(j=0; j<500; j++)
    {   printf("loop counter:%d\n", j);
        if(N != sndRead(psd, buf, N))
        {   printf("win_sndRead crashed...");
        }
        if(N != sndWrite(psd, buf, N))
        {   printf("sndWrite crashed...");
        }
    }
    psd = sndClose(psd);
    return 0;
}
  @endverbatim
 ********************************************************************/
int sndWrite(SndDevice_t *sd, short *buf, int buf_elements);






/* WAV file routines for windows/linux. It is assumed,
   that the WAV file has a simple header consisting of only a
   format-chunk and a data-chunk, as is shown in the struct below:
*/


/*!
 ************************************************************************
  @par Description:
    Struktur zum Auslesen des Waveheaders

  @param main_chunk  - Textinhalt "RIFF"
  @param length      - Gesamtlaenge der Datei in Bytes
  @param chunk_type  - Textinhalt "WAVE" bei Wave-Dateien
  @param sub_chunk   - Textinhalt "fmt_"
  @param sub_length  - Laenge sub_chunk, immer 16 Bytes
  @param format      - 1 = PCM [z.Z. nur PCM eingesetzt, also immer 1]
  @param nChannels   - 1 = mono
  @param nChannels       - 2 = stereo
  @param nSamplesPerSec - Abtastfrequenz der Datei
  @param nBytesPerSec  - Datendurchsatz pro Sekunde
  @param nBytesPerSample  - 1 = 8 Byte per Sample [8-Bit-Mono-Datei]
  @param nBytesPerSample  - 2 = 8 Byte per Sample [8-Bit-Stereo- oder
                                16-Bit-Mono-Datei]
  @param nBytesPerSample  - 4 = 16 Byte per Sample [16-Bit-Stereo-Datei]
  @param nBitsPerSample   - 8 = 8 Bit per Sample
  @param nBitsPerSample   - 12 = 16 Bit per Sample (Soundcard has 12 Bit,
                                but 16 Bit stored)
  @param nBitsPerSample   - 16 = 16 Bit per Sample
  @param data_chunk  - Kennung Datenbereich "data"
  @param data_length - Laenge des Datenblockes in Bytes

 ******************************************************************/
typedef struct{
    unsigned long main_chunk;   /*!<@arg  Textinhalt "RIFF" */
    unsigned long length;       /*!<@arg  Gesamtlaenge der Datei */
    unsigned long chunk_type;   /*!<@arg  Textinhalt "WAVE" */
    unsigned long sub_chunk;    /*!<@arg  Textinhalt "fmt_" */
    unsigned long sub_length;   /*!<@arg  Laenge sub_chunk, 16 Bytes */
    unsigned short format;      /*!<@arg  1 = PCM */
    unsigned short nChannels;   /*!<@arg  1 = MONO;
                           @arg  2 = STEREO */
    unsigned long nSamplesPerSec;/*!<@arg Abtastfrequenz in Hz */
    unsigned long nBytesPerSec;  /*!<@arg Datendurchsatz pro Sekunde */
    unsigned short nBytesPerSample; /*!<@arg 1 = 8Bit-Mono
                              @arg 2 = 8Bit-St. oder 16-Bit-Mono
                              @arg 4 = 16Bit-Stereo */
    unsigned short nBitsPerSample;/*!<@arg  8 = 8 Bit per Sample
                             @arg 12 = 16 Bit per Sample
                             @arg 16 = 16 Bit per Sample */
    unsigned long data_chunk;     /*!<@arg Testinhalt "data" */
    unsigned long data_length;    /*!<@arg Leange Datenblock in Bytes*/
} sndWaveHeader_t;

/*!
 ********************************************************************
  @par Description:
    Struktur zur Uebernahme und Uebergabe von 8-Bit Samplewertdaten
 *******************************************************************/
 typedef struct{ unsigned char val;   /*!<@arg Abtastwert 8-Bit MONO */
 } sndMono8_t;


/*!
 *******************************************************************
  @par Description:
    Struktur zur Uebernahme und Uebergabe von 8-Bit Stereo
    Samplewertdaten
 ********************************************************************/
 typedef struct{
    unsigned char val_li;  /*!<@arg Abtastwert linker Kanal 8-Bit STEREO */
    unsigned char val_re;  /*!<@arg Abtastwert rechter Kanal 8-Bit STEREO */
 } sndStereo8_t;


/*!
 ****************************************************************
  @par Description:
    Struktur zur Uebernahme und Uebergabe von 16-Bit Mono
    Samplewertdaten

 *****************************************************************/
 typedef struct{ short val;         /*!<@arg Abtastwert  16-Bit MONO */
 } sndMono16_t;


/*!
 *****************************************************************
  @par Description:
    Struktur zur Uebernahme und Uebergabe von 16-Bit Stereo
    Samplewertdaten

******************************************************************/
 typedef struct{
    short val_li; /*!<@arg Abtastwert linker Kanal 16Bit STEREO  */
    short val_re;    /*!<@arg Abtastwert rechter Kanal 16Bit STEREO */
 } sndStereo16_t;


/*!
 *****************************************************************
  @par Description:
    Funktion liest den Header aus der Wave-Datei mit Hilfe der Struktur
    sndWaveHeader_t.

  @par Note: Diese Funktion geht davon aus, dass es nur einen
    Format-Chunk und einen Data-Chunk der Wave-Datei gibt.
    Das trifft nicht fuer jede Datei zu. In einer verbesserten
    Version dieser Funktion muesste man den Datei-Header nach
    dem Format-Chunk und dem Data-Chunk durchsuchen
    und ggf. weitere Chunks ueberlesen.

  @par Used by:
  @arg sndWAVPlaySound()

  @see
  @arg sndWAVWriteFileHeader()

  @param  fp -  IN, Zeiger auf die bereits geoeffnete Datei
  @param  wh -  IN/OUT, Zeiger auf Struktur der Daten des Headers der
                Wavedatei

  @retval 0 for ok, -1 on error

  @par Beispiel :

  @verbatim
  sndWaveHeader_t wh;
  FILE *fp;
  ...
  if(NULL == (fp = fopen("madonna.wav","rb")))
  {  printf("error: cannot open file\n");
     return -1;
  }
  if(0!=sndWAVReadFileHeader(fp, &wh))
    puts("error in sndWAVReadFileHeader");
  printf("Abtastfrequenz: %d\n" , wh.nSamplesPerSec);
  ...
  fclose(fp);
  @endverbatim
 ********************************************************************/
int sndWAVReadFileHeader(FILE *fp, sndWaveHeader_t *wh);




/*!
 *******************************************************************
  @par Description:
    Funktion schreibt den WaveHeader in die bereits geoffnete
    Ausgabedatei. Die Datei bleibt danach geoeffnet.
  @par Used by:
  @arg

  @see
  @arg sndWAVReadFileHeader()

  @param fp  -  IN, Zeiger auf die bereits geoeffnete Datei
  @param wh - IN Strukturuebergabe Daten des Header der Wavedatei

  @retval 0 for ok, -1 on error

  @par Beispiel :

  @verbatim
  sndWaveHeader_t wh;
  FILE *fp;
  ...
  if(NULL == (fp = fopen("madonna.wav","wb")))
  {  printf("error: cannot open file\n");
     return -1;
  }
  // wh Struktur mit sinnvollen Daten fuellen...
  if(0!=sndWAVWriteFileHeader(fp, wh))
    puts("error in sndWAVWriteFileHeader");
  ...
  fclose(fp);
  @endverbatim
 **************************************************************/
int sndWAVWriteFileHeader(FILE *fp, sndWaveHeader_t wh);



/*!
 **************************************************************
  @par Description:
    Liest die Sounddaten aus der angegebenen Datei in ein Feld,
    oeffnet die Soundkarte und stellt Abtastrate und Datenformat
    der Soundkarte so ein, wie es die WAV Datei vorgibt.
    Die Funktion kopiert das Feld auf die Soundkarte.
    Die Soundkarte darf vorher noch nicht geoeffnet sein, am
    Ende wird sie wieder geschlossen.

  @par Used by:
  @arg

  @see
  @arg

  @param  Filename -  IN, Zeiger auf eine Zeichenkette, Dateiname

  @retval 0 for ok, -1 on error

  @par Beispiel :

  @verbatim
    char fileName[256];
    puts("File?");
    gets(fileName);
    if(0!=sndWAVPlaySound(fileName))
    {  puts("error in sndWAVPlaySound()");
       printf("cannot open file:'%s'\n", fileName);
       return -1;
    }
    fflush(stdin);
    getchar();    // warten, bis Abspielen fertig ist
  @endverbatim
 ******************************************************************/
int sndWAVPlaySound(char *Filename);




/*!
 *********************************************************
  @par Description:
    Funktion ermittelt die Anzahl der Abtastwerte bzw.
    Abtastwertepaare (Stereo).
    Im Header steht nur die Laenge der Daten in Bytes. Hier wird
    daraus in Abhaengigkeit vom Fomat (8/16 Bit, Mono/Stereo)
    die Anzahl der Werte / Wertepaare ermittelt.

  @par Used by:

  @see
  @arg

  @param wh - IN wave-Header vom Typ WaveHeader_t

  @retval Anzahl der Abtastwerte / Abtastwertepaare , 0 on error

  @par Beispiel :

  @verbatim
   unsigned long i;
   sndStereo16_t x;
   FILE *fp, *fp_out;
   ...
   for(i=0; i<sndWAVGetNumberOfSamples(wh); i++) // all samples
   {  if(0!=sndWAVReadSampleStereo16(fp, &x))
      { puts("error in sndWAVReadSampleStereo16");
      }
      // Abtastwertepaar verarbeiten...
      if(0!=sndWAVWriteSampleStereo16(fp_out, x))
      { puts("error in sndWAVReadSampleStereo16");
      }
    }
    ...
  @endverbatim
 ***********************************************************/
unsigned long sndWAVGetNumberOfSamples(sndWaveHeader_t wh);



/*!
 ***************************************************************
  @par Description:
    Funktion schreibt ein 8Bit Mono Sample  in Ausgabedatei

  @par Used by:

  @see
  @arg sndWAVReadSampleMono8

  @param fp - IN, Zeiger auf bereits geoffnete Datei
  @param x  - IN, Abtastwert

  @retval 0 for ok, -1 on error


  @par Beispiel :

  @verbatim
   unsigned long i;
   sndMono8_t x;
   FILE *fp, *fp_out;
   ...
   for(i=0; i<sndWAVGetNumberOfSamples(wh); i++) // all samples
   {  if(0!=sndWAVReadSampleMono8(fp, &x))
      { puts("error in sndWAVReadSampleMono8");
      }
      // Abtastwert verarbeiten...
      if(0!=sndWAVWriteSampleMono8(fp_out, x))
      { puts("error in sndWAVWriteSampleMono8");
      }
    }
    ...
  @endverbatim
 ***************************************************************/
int sndWAVWriteSampleMono8(FILE *fp, sndMono8_t x);




/*!
 *****************************************************************
  @par Description:
    Funktion liest ein 8Bit Mono Sample aus Wavedatei ein

  @par Used by:

  @see
  @arg sndWAVWriteSampleMono8

  @param fp - IN, Zeiger auf bereits geoffnete Datei
  @param x _ IN/OUT Zeiger auf Abtastwert

  @retval  0: ok, -1: error

  @par Beispiel :

  @verbatim
   unsigned long i;
   sndMono8_t x;
   FILE *fp, *fp_out;
   ...
   for(i=0; i<sndWAVGetNumberOfSamples(wh); i++) // all samples
   {  if(0!=sndWAVReadSampleMono8(fp, &x))
      { puts("error in sndWAVReadSampleMono8");
      }
      // Abtastwert verarbeiten...
      if(0!=sndWAVWriteSampleMono8(fp_out, x))
      { puts("error in sndWAVWriteSampleMono8");
      }
    }
    ...
  @endverbatim
 ***************************************************************/
int sndWAVReadSampleMono8(FILE *fp, sndMono8_t *x);



/*!
 ***************************************************************
  @par Description:
    Funktion schreibt ein 8Bit Stereo Samplepaar  in Ausgabedatei

  @par Used by:

  @see
  @arg sndWAVReadSampleStereo8

  @param fp - IN, Zeiger auf bereits geoffnete Datei
  @param x  - IN, Abtastwert

  @retval 0 for ok, -1 on error

  @par Beispiel :

  @verbatim
   unsigned long i;
   sndStereo8_t x;
   FILE *fp, *fp_out;
   ...
   for(i=0; i<sndWAVGetNumberOfSamples(wh); i++) // all samples
   {  if(0!=sndWAVReadSampleStereo8(fp, &x))
      { puts("error in sndWAVReadSampleStereo8");
      }
      // Abtastwertepaar verarbeiten...
      if(0!=sndWAVWriteSampleStereo8(fp_out, x))
      { puts("error in sndWAVWriteSampleStereo8");
      }
    }
    ...
  @endverbatim
 ************************************************************/
int sndWAVWriteSampleStereo8(FILE *fp, sndStereo8_t x);


/*!
 *****************************************************************
  @par Description:
    Funktion liest ein 8Bit Stereo Samplepaar aus Wavedatei ein

  @par Used by:

  @see
  @arg sndWAVWriteSampleStereo8

  @param fp - IN, Zeiger auf bereits geoffnete Datei
  @param x _ IN/OUT Zeiger auf Abtastwert

  @retval  0: ok, -1: error

  @par Beispiel :

  @verbatim
   unsigned long i;
   sndStereo8_t x;
   FILE *fp, *fp_out;
   ...
   for(i=0; i<sndWAVGetNumberOfSamples(wh); i++) // all samples
   {  if(0!=sndWAVReadSampleStereo8(fp, &x))
      { puts("error in sndWAVReadSampleStereo8");
      }
      // Abtastwertepaar verarbeiten...
      if(0!=sndWAVWriteSampleStereo8(fp_out, x))
      { puts("error in sndWAVWriteSampleStereo8");
      }
    }
    ...
  @endverbatim
 ***************************************************************/
int sndWAVReadSampleStereo8(FILE *fp, sndStereo8_t *x);



/*!
 ******************************************************************
  @par Description:
    Funktion schreibt ein 16Bit Mono Sample  in Ausgabedatei

  @par Used by:

  @see
  @arg sndWAVReadSampleMono16

  @param fp - IN, Zeiger auf bereits geoffnete Datei
  @param x  - IN, Abtastwert

  @retval 0 for ok, -1 on error

  @par Beispiel :

  @verbatim
   unsigned long i;
   sndMono16_t x;
   FILE *fp, *fp_out;
   ...
   for(i=0; i<sndWAVGetNumberOfSamples(wh); i++) // all samples
   {  if(0!=sndWAVReadSampleMono16(fp, &x))
      { puts("error in sndWAVReadSampleMono16");
      }
      // Abtastwert verarbeiten...
      if(0!=sndWAVWriteSampleMono16(fp_out, x))
      { puts("error in sndWAVWriteSampleMono16");
      }
    }
    ...
  @endverbatim
 ******************************************************************/
int sndWAVWriteSampleMono16(FILE *fp, sndMono16_t x);


/*!
 *************************************************************
  @par Description:
    Funktion liest ein 16Bit Mono Sample aus Wavedatei ein

  @par Used by:

  @see
  @arg sndWAVWriteSampleMono16

  @param fp - IN, Zeiger auf bereits geoffnete Datei
  @param x _ IN/OUT Zeiger auf Abtastwert

  @retval  0: ok, -1: error

  @par Beispiel :

  @verbatim
   unsigned long i;
   sndMono16_t x;
   FILE *fp, *fp_out;
   ...
   for(i=0; i<sndWAVGetNumberOfSamples(wh); i++) // all samples
   {  if(0!=sndWAVReadSampleMono16(fp, &x))
      { puts("error in sndWAVReadSampleMono16");
      }
      // Abtastwert verarbeiten...
      if(0!=sndWAVWriteSampleMono16(fp_out, x))
      { puts("error in sndWAVWriteSampleMono16");
      }
    }
    ...
  @endverbatim
 ********************************************************/
int sndWAVReadSampleMono16(FILE *fp, sndMono16_t *x);



/*!
 ****************************************************************
  @par Description:
    Funktion schreibt ein 16Bit Stereo Samplepaar  in Ausgabedatei

  @par Used by:

  @see
  @arg sndWAVReadSampleStereo16

  @param fp - IN, Zeiger auf bereits geoffnete Datei
  @param x  - IN, Abtastwert

  @retval 0 for ok, -1 on error

  @par Beispiel :

  @verbatim
   unsigned long i;
   sndStereo16_t x;
   FILE *fp, *fp_out;
   ...
   for(i=0; i<sndWAVGetNumberOfSamples(wh); i++) // all samples
   {  if(0!=sndWAVReadSampleStereo16(fp, &x))
      { puts("error in sndWAVReadSampleStereo16");
      }
      // Abtastwertepaar verarbeiten...
      if(0!=sndWAVWriteSampleStereo16(fp_out, x))
      { puts("error in sndWAVReadSampleStereo16");
      }
    }
    ...
  @endverbatim
 *****************************************************************/
int sndWAVWriteSampleStereo16(FILE *fp, sndStereo16_t x);


/*!
 ******************************************************************
  @par Description:
    Funktion liest ein 16Bit Stereo Samplepaar aus Wavedatei ein

  @par Used by:

  @see
  @arg sndWAVWriteSampleStereo16

  @param fp - IN, Zeiger auf bereits geoffnete Datei
  @param x - IN/OUT Zeiger auf Abtastwert

  @retval  0: ok, -1: error

  @par Beispiel :

  @verbatim
   unsigned long i;
   sndStereo16_t x;
   FILE *fp, *fp_out;
   ...
   for(i=0; i<sndWAVGetNumberOfSamples(wh); i++) // all samples
   {  if(0!=sndWAVReadSampleStereo16(fp, &x))
      { puts("error in sndWAVReadSampleStereo16");
      }
      // Abtastwertepaar verarbeiten...
      if(0!=sndWAVWriteSampleStereo16(fp_out, x))
      { puts("error in sndWAVReadSampleStereo16");
      }
    }
    ...
  @endverbatim
 *****************************************************************/
int sndWAVReadSampleStereo16(FILE *fp, sndStereo16_t *x);












#endif

