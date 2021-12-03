/*!
 ******************************************************************
   @file            snd_lib.c
   @par Project   : Sound Utilities, Linux / Windows
   @par Module    : low level sound device prgramming

   @brief   consists of three sections: Win32, Linux, WAVE-file implementation


   @par Author    : Fgb
   @par Department: EIT-PIA
   @par Company   : Fachhochschule Hannover - University of Applied
                    Sciences and Arts

 ********************************************************************

   @par History   :
   @verbatim
   Version | Date        | Author | Change Description
   0.0     | 27.08. 2007 | Fgb    | First Version, Linux / Win32
   0.1     | 04.10. 2017 | Fgb    | Bugfix for Win10, search BugFixWin10
   1.0     | 04.06.2020  | Fgb    | ALSA on Linux
   @endverbatim

*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "snd_lib.h"



#define BugFixWin10 1   /* != 0 for Win10 Bug Fix!! */

/*------------- DEBUGGING ---------------------------------*/
#define DEBUG_MODULE 0

#if DEBUG_MODULE
  #define DebugCode( code_fragment ) { code_fragment }
#else
  #define DebugCode( code_fragment )
#endif

#define _MyAssert(a, msg) {  if(!(a)) fprintf(stderr,"%s",msg); assert(a); }



// debugging: better syntax higlighting...
//#define PLATFORM  OS_LINUX
//#define LINUX_ALSA 1
// end of better syntax highlighting...




/*------------------------------------------------------------*/


/*  Windows and Linux implementation of sound card functions.
    Sound card will be operated at 44100Hz only, 16 bit signed
    sample format only.
    Read, Write, ReadWrite access is supported.
    Mono or Stereo is supported.
*/

/* Definitions ************************************************/
#define     NUM_IN_OUT_CHANNELS     2   /* Stereo recording */
#define     BITS_PER_SAMPLE         16  /* Use all bits of the D/A-converter */
#define     BYTES_PER_SAMPLE        2   /* Number of bytes per sample */
#define     SOUNDCARD_SAMPLE_RATE   44100



#ifndef FALSE
    #define FALSE 0
#endif
#ifndef TRUE
    #define TRUE (!FALSE)
#endif

/**********************************************************************
* gemeinsam fuer Windows und Linux (OSS und auch ALSA):
* Funktionen zur Bearbeitung von WAV-Dateien.
* Nur die Fkt int sndWAVPlaySound(char *Filename); ist abhaengig vom
* Betriebssystem implementiert.
***********************************************************************/



/*********************************************************************
Wave-Dateiformat implementation Linux / Windows:
***********************************************************************/
/*
_\              _/    _/_\    _/        _/    _/_/_/
 _\            _/    _/  _\    _/      _/    _/
  _\          _/    _/    _\    _/    _/    _/_/_/
   _\  _/_\  _/    _/_/_/_/_\    _/  _/    _/
    _\_/  _\_/    _/        _\    _/_     _/_/_/_/
*/



/*************************************************/
static void _errMsg(char *str)
{  fprintf(stderr,"error in snd_util: %s\n", str);
}
/*************************************************/




/*!
 ***********************************************************************
  @par Description:
    Funktion liest den Header aus der Wave-Datei mit Hilfe der Struktur
    sndWaveHeader_t.

  @par Note: Diese Funktion geht davon aus, dass es nur einen Format-Chunk
    und einen Data-Chunk der Wave-Datei gibt. Das trifft nicht fuer jede
    Datei zu. In einer verbesserten Version dieser Funktion muesste man
    den Datei-Header nach dem Format-Chunk und dem Data-Chunk durchsuchen
    und ggf. weitere Chunks ueberlesen.

  @par Used by:
  @arg sndWAVPlaySound()

  @see
  @arg sndWAVWriteFileHeader()

  @param  fp -  IN, Zeiger auf die bereits geoeffnete Datei
  @param  wh -  IN/OUT, Zeiger auf Struktur der Daten des Headers der
                Wavedatei

  @retval 0 for ok, -1 on error

 ********************************************************************/
int sndWAVReadFileHeader(FILE *fp, sndWaveHeader_t *wh)
{
        /* HeaderDaten aus Datei einlesen */
        if(NULL == fp)
        {   _errMsg("sndWAVReadFileHeader, no file!");
            return -1;
        }
        if(1!=fread(wh, sizeof(sndWaveHeader_t),1,fp))
        {   _errMsg("sndWAVReadFileHeader: cannot read header");
            return -1;
        }
#if 0
        printf(" Datei-Laenge................. %u\n",wh->length);
        printf(" Laenge sub_chunk............. %u\n",wh->sub_length);
        printf(" PCM-Code..................... %u\n",wh->format);
        printf(" Anzahl Kanaele............... %u\n",wh->nChannels);
        printf(" 11kHz | 22kHz ............... %u\n",wh->nSamplesPerSec);
        printf(" Datendurchsatz Bytes per Sec. %u\n",wh->nBytesPerSec);
        printf(" Bytes per Sample   .......... %u\n",wh->nBytesPerSample);
        printf(" Bit per Sample 8,12,16....... %u\n",wh->nBitsPerSample);
        printf(" Laenge des Datenblocks....... %u\n\n",wh->data_length);
#endif
        return(0);
}
/*----------------------------------------------------------------*/

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

 **************************************************************/
 int sndWAVWriteFileHeader(FILE *fp, sndWaveHeader_t wh)
{
    if(NULL == fp)
    {   _errMsg("sndWAVWriteFileHeader, no file!");
        return -1;
    }
    if(1!=fwrite(&wh, sizeof(sndWaveHeader_t),1,fp))
    {   _errMsg("sndWAVWriteFileHeader: cannot write header");
        return -1;
    }
    return 0;
}
/*----------------------------------------------------------------*/


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

 ******************************************************************/
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

 ***********************************************************/
unsigned long  sndWAVGetNumberOfSamples(sndWaveHeader_t wh)
{   /*  berechnet die Anzahl der Abtastwerte / Abtastwertepaare aus den Angaben
        im Header. 8Bit Mono:   1 Sample = 1 Byte
                   16Bit Mono:   1 Sample = 2 Byte
                   8Bit Stereo: 1 Sample = 2 Byte
                   16Bit Stereo: 1 Sample = 4 Byte
        returns 0 on error.
    */
    if(wh.nChannels == 1) /* mono */
    {  switch(wh.nBitsPerSample)
       {  case 8:  return (wh.data_length);
          case 12:
          case 16: return (wh.data_length / 2);
          default: _errMsg("sndWAVGetNumberOfSamples: unexpected nBitsPerSample");
                   return 0;
        }
    }
    if(wh.nChannels == 2) /* stereo */
    {  switch(wh.nBitsPerSample)
       {  case 8:  return (wh.data_length / 2);
          case 12:
          case 16: return (wh.data_length / 4);
          default: _errMsg("WAVGetNumberOfSamples: cannot interpret nBitsPerSample");
                   return 0;
       }
    }
    return 0;  /* error: no match found ! */
}
/*----------------------------------------------------------------*/


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

 ***************************************************************/
int sndWAVWriteSampleMono8(FILE *fp, sndMono8_t x)
{   if(NULL == fp)
    {   _errMsg("sndWAVWriteSampleMono8, no file!");
        return -1;
    }
    if(1!=fwrite(&x, sizeof(sndMono8_t),1,fp))
    {   _errMsg("sndWAVWriteSampleMono8: cannot write sample");
        return -1;
    }
    return 0;
}

/*----------------------------------------------------------------*/

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

 ***************************************************************/
int sndWAVReadSampleMono8(FILE *fp, sndMono8_t *x)
{   if(NULL == fp)
    {   _errMsg("sndWAVReadSampleMono8, no file!");
        return -1;
    }
    if(1!=fread(x, sizeof(sndMono8_t),1,fp))
    {   _errMsg("sndWAVReadSampleMono8: cannot read sample");
        return -1;
    }
    return 0;
}
/*----------------------------------------------------------------*/


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

 ************************************************************/
int sndWAVWriteSampleStereo8(FILE *fp, sndStereo8_t x)
{   if(NULL == fp)
    {   _errMsg("sndWAVWriteSampleStereo8, no file!");
        return -1;
    }
    if(1!=fwrite(&x, sizeof(sndStereo8_t),1,fp))
    {   _errMsg("sndWAVWriteSampleStereo8: cannot write sample");
        return -1;
    }
    return 0;
}
/*----------------------------------------------------------------*/

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

 ***************************************************************/
int sndWAVReadSampleStereo8(FILE *fp, sndStereo8_t *x)
{   if(NULL == fp)
    {   _errMsg("sndWAVReadSampleStereo8, no file!");
        return -1;
    }
    if(1!=fread(x, sizeof(sndStereo8_t),1,fp))
    {   _errMsg("sndWAVReadSampleStereo8: cannot read sample");
        return -1;
    }
    return 0;
}
/*----------------------------------------------------------------*/


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

 ******************************************************************/
int sndWAVWriteSampleMono16(FILE *fp, sndMono16_t x)
{   if(NULL == fp)
    {   _errMsg("sndWAVWriteSampleMono16, no file!");
        return -1;
    }
    if(1!=fwrite(&x, sizeof(sndMono16_t),1,fp))
    {   _errMsg("sndWAVWriteSampleMono16: cannot write sample");
        return -1;
    }
    return 0;
}
/*----------------------------------------------------------------*/

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

 ********************************************************/
int sndWAVReadSampleMono16(FILE *fp, sndMono16_t *x)
{   if(NULL == fp)
    {   _errMsg("sndWAVReadSampleMono16, no file!");
        return -1;
    }
    if(1!=fread(x, sizeof(sndMono16_t),1,fp))
    {   _errMsg("sndWAVReadSampleMono16: cannot read sample");
        return -1;
    }
    return 0;
}
/*----------------------------------------------------------------*/


/*!
 ****************************************************************
  @par Description:
    Funktion schreibt ein 16Bit Stereo Samplepaar  in Ausgabedatei

  @par Used by:

  @see
  @arg sndWAVReadSampleStereo8

  @param fp - IN, Zeiger auf bereits geoffnete Datei
  @param x  - IN, Abtastwert

  @retval 0 for ok, -1 on error

 *****************************************************************/
int sndWAVWriteSampleStereo16(FILE *fp, sndStereo16_t x)
{   if(NULL == fp)
    {   _errMsg("sndWAVWriteSampleStereo16, no file!");
        return -1;
    }
    if(1!=fwrite(&x, sizeof(sndStereo16_t),1,fp))
    {   _errMsg("sndWAVWriteSampleStereo16: cannot write sample");
        return -1;
    }
    return 0;
}
/*----------------------------------------------------------------*/

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

 *****************************************************************/
int sndWAVReadSampleStereo16(FILE *fp, sndStereo16_t *x)
{   if(NULL == fp)
    {   _errMsg("sndWAVReadSampleStereo16, no file!");
        return -1;
    }
    if(1!=fread(x, sizeof(sndStereo16_t),1,fp))
    {   _errMsg("sndWAVReadSampleStereo16: cannot read sample");
        return -1;
    }
    return 0;
}
/*----------------------------------------------------------------*/
















/*******************************************************************
Windows implementation: found in DREAM DRM Radio Software
Version drm 1.1, by Volker Fischer, Technische Universitaet
Darmstadt, converted to C by Fgb
******************************************************************/
/*
_\              _/    _/    _/        _/    _/_/_/    _/_/_/
 _\            _/    _/    _/_\      _/          _/        _/
  _\          _/    _/    _/  _\    _/       _/_/        _/
   _\  _/_\  _/    _/    _/    _\  _/           _/     _/
    _\_/  _\_/    _/    _/      _\_/      _/_/_/     _/_/_/_/
*/

#if (PLATFORM==OS_MS_WINDOWS)

/*
IDE: vide mit Borland-Compiler, WindowsXP

21.8.07:
Ich nehme die Sound-Klasse aus DREAM in der Version drm1.1
Es gibt nur eine Klasse fr Ein- und Ausgabe von Sound. Weitere Quelle:
Richard J. Simon, Multimedia, ODBC & Telefonie, Windows 95/ Windows NT API Bible,
edv 3650-win/318-3
*/

/* Simon: Ein Waveform Device erhaelt mehrere Puffer, in die es kontinuierlich schreiben, bzw aus denen
    es lesen kann.  */
#define NUM_SOUND_BUFFERS_IN        2
#define NUM_SOUND_BUFFERS_OUT       4

/* Maximum number of recognized sound cards installed in the system */
#define MAX_NUMBER_SOUND_CARDS      10

/*************** private types of this module *************************/
typedef struct {
    WAVEFORMATEX    sWaveFormatEx;
    UINT            iNumDevs;   /* how many devices on this computer?*/
    UINT            iCurInDev;  /* which device to use, read */
    UINT            iCurOutDev; /* which device to use, write */
    int             bChangDevIn;  /* Check if device must be opened */
    int             bChangDevOut; /*or reinitialized? Anything changed? */
    /* wave in */
    WAVEINCAPS        m_WaveInDevCaps; /* capabilities of device */
    HWAVEIN           m_WaveIn;
    HANDLE            m_WaveInEvent;
    WAVEHDR           m_WaveInHeader[NUM_SOUND_BUFFERS_IN];
    int               iBufferSizeIn;
    int               iWhichBufferIn;
    short*            psSoundcardBuffer[NUM_SOUND_BUFFERS_IN];
    int               bBlockingRec;
    /* wave out */
    int               iBufferSizeOut;
    HWAVEOUT          m_WaveOut;
    short*            psPlaybackBuffer[NUM_SOUND_BUFFERS_OUT];
    WAVEHDR           m_WaveOutHeader[NUM_SOUND_BUFFERS_OUT];
    HANDLE            m_WaveOutEvent;
    int               bBlockingPlay;
} WaveInOut_t;

/******************** private module variable *************************/


/************** private module functions  prototypes ******************/

static void    _win_sndConstructor(SndDevice_t *psd);
static void    _win_sndDestructor(SndDevice_t *psd);
static void    _win_sndInitRecording(SndDevice_t *psd,int iNewBufferSize, int bNewBlocking);
static void    _win_sndInitPlayback(SndDevice_t *psd, int iNewBufferSize, int bNewBlocking);
static int     _win_sndRead(SndDevice_t *psd, short *psData, int n);
static int     _win_sndWrite(SndDevice_t *psd, short *psData, int n);
static void    _win_sndClose(SndDevice_t *psd);

static void _OpenInDevice(SndDevice_t *psd);
static void _OpenOutDevice(SndDevice_t *psd);
static void _PrepareInBuffer(SndDevice_t *psd, int iBufNum);
static void _PrepareOutBuffer(SndDevice_t *psd, int iBufNum);
static void _AddInBuffer(SndDevice_t *psd);
static void _AddOutBuffer(SndDevice_t *psd, int iBufNum);
static void _GetDoneBuffer(SndDevice_t *psd, int *piCntPrepBuf, int* piIndexDoneBuf);

/************** private module functions  implementation *************************/


/*---------------------------------------------------------------------------*/
static void _OpenInDevice(SndDevice_t *psd)
{   /* Open wave-input and set call-back mechanism to event handle.  iCurInDev is set in
        constructor is set to WAVE_MAPPER, this will open the first available wave device
        fitting the settings in sWaveFormatEx struct.*/

    WaveInOut_t *wpt;
    wpt = (WaveInOut_t *)psd->pt; /* convert to correct pointer type */

    DebugCode(printf("*** _OpenInDevice\n"););

    if (wpt->m_WaveIn != NULL)
    {   waveInReset(wpt->m_WaveIn);
        waveInClose(wpt->m_WaveIn);
    }

    {   MMRESULT result;
        DebugCode(printf("    _OpenInDevice: waveInOpen\n"););
        result = waveInOpen(&(wpt->m_WaveIn), wpt->iCurInDev,
                 &(wpt->sWaveFormatEx), (DWORD) wpt->m_WaveInEvent,
                 (DWORD) NULL, CALLBACK_EVENT);
        if (result != MMSYSERR_NOERROR)
        {   _MyAssert(result == MMSYSERR_NOERROR,
            "Sound Interface Start, waveInOpen() failed. This error "
            "usually occurs if another application blocks the sound in.");
        }
    }
    DebugCode(printf("--- _OpenInDevice\n"););
}
/*---------------------------------------------------------------------------*/
static void _OpenOutDevice(SndDevice_t *psd)
{   /* Open wave-output and set call-back mechanism to event handle.  iCurOutDev is set in
        constructor is set to WAVE_MAPPER, this will open the first available wave device
        fitting the settings in sWaveFormatEx struct.*/
    WaveInOut_t *wpt;
    wpt = (WaveInOut_t *)psd->pt; /* convert to correct pointer type */

    DebugCode(printf("*** _OpenOutDevice\n"););

    if (wpt->m_WaveOut != NULL)
    {   DebugCode(printf("closing waveout device...\n"););
        waveOutReset(wpt->m_WaveOut);
        waveOutClose(wpt->m_WaveOut);
    }

    {   MMRESULT result;
        DebugCode(printf("    _OpenOutDevice: waveOutOpen\n"););
        result = waveOutOpen(&(wpt->m_WaveOut), wpt->iCurOutDev,
                 &(wpt->sWaveFormatEx), (DWORD) wpt->m_WaveOutEvent,
                 (DWORD)NULL, CALLBACK_EVENT);
        if (result != MMSYSERR_NOERROR)
        {   _MyAssert(result == MMSYSERR_NOERROR,
            "Sound Interface Start, waveOutOpen() failed.");
        }
    }
    DebugCode(printf("--- _OpenOutDevice\n"););
}
/*---------------------------------------------------------------------------*/
static void _PrepareInBuffer(SndDevice_t *psd, int iBufNum)
{   /* Set struct entries */
    MMRESULT result;
    WaveInOut_t *wpt;
    wpt = (WaveInOut_t *)psd->pt; /* convert to correct pointer type */

    DebugCode(printf("*** _PrepareInBuffer\n"););

    _MyAssert(NULL!=&(wpt->psSoundcardBuffer[iBufNum][0]), "psSoundcardBuffer is NULL");
    wpt->m_WaveInHeader[iBufNum].lpData = (LPSTR) &(wpt->psSoundcardBuffer[iBufNum][0]);
    _MyAssert(wpt->m_WaveInHeader[iBufNum].lpData!=0,"data buffer is NULL...");
    wpt->m_WaveInHeader[iBufNum].dwBufferLength = wpt->iBufferSizeIn * BYTES_PER_SAMPLE;
    wpt->m_WaveInHeader[iBufNum].dwFlags = 0;

    /* Prepare wave-header */
    result = waveInPrepareHeader(wpt->m_WaveIn, &(wpt->m_WaveInHeader[iBufNum]), sizeof(WAVEHDR));
    _MyAssert(result==MMSYSERR_NOERROR, "waveInPrepareHeader crashed");
    DebugCode(printf("--- _PrepareInBuffer\n"););
}
/*---------------------------------------------------------------------------*/
static void _PrepareOutBuffer(SndDevice_t *psd, int iBufNum)
{   /* Set Header data */
    MMRESULT result;
    WaveInOut_t *wpt;
    wpt = (WaveInOut_t *)psd->pt; /* convert to correct pointer type */

    DebugCode(printf("*** _PrepareOutBuffer:%d\n",iBufNum););

    _MyAssert(NULL!=&(wpt->psPlaybackBuffer[iBufNum][0]), "psPlaybackBuffer is NULL");
    wpt->m_WaveOutHeader[iBufNum].lpData = (LPSTR) &(wpt->psPlaybackBuffer[iBufNum][0]);
    wpt->m_WaveOutHeader[iBufNum].dwBufferLength = wpt->iBufferSizeOut * BYTES_PER_SAMPLE;
    wpt->m_WaveOutHeader[iBufNum].dwFlags = 0;

    /* Prepare wave-header */
    result = waveOutPrepareHeader(wpt->m_WaveOut, &(wpt->m_WaveOutHeader[iBufNum]), sizeof(WAVEHDR));
    _MyAssert(result==MMSYSERR_NOERROR, "waveOutPrepareHeader crashed");
    DebugCode(printf("--- _PrepareOutBuffer\n"););
}
/*---------------------------------------------------------------------------*/
static void _AddInBuffer(SndDevice_t *psd)
{
    WaveInOut_t *wpt;
    wpt = (WaveInOut_t *)psd->pt; /* convert to correct pointer type */

    DebugCode(printf("*** _AddInBuffer:\n"););

    /* Unprepare old wave-header */
    waveInUnprepareHeader(wpt->m_WaveIn, &(wpt->m_WaveInHeader[wpt->iWhichBufferIn]), sizeof(WAVEHDR));

    /* Prepare buffers for sending to sound interface */
    _PrepareInBuffer(psd, wpt->iWhichBufferIn);

    /* Send buffer to driver for filling with new data */
    DebugCode(printf("    _AddInBuffer: adding buffer %d\n", wpt->iWhichBufferIn););
    waveInAddBuffer(wpt->m_WaveIn, &(wpt->m_WaveInHeader[wpt->iWhichBufferIn]), sizeof(WAVEHDR));

    /* Toggle buffers */
    (wpt->iWhichBufferIn)++;
    if (wpt->iWhichBufferIn == NUM_SOUND_BUFFERS_IN)
    {   wpt->iWhichBufferIn = 0;
    }
    DebugCode(printf("--- _AddInBuffer\n"););
}

/*---------------------------------------------------------------------------*/
static void _AddOutBuffer(SndDevice_t *psd,int iBufNum)
{   MMRESULT result;
    WaveInOut_t *wpt;
    wpt = (WaveInOut_t *)psd->pt; /* convert to correct pointer type */

    DebugCode(printf("*** _AddOutBuffer :%d\n", iBufNum););
    /* Unprepare old wave-header */
    waveOutUnprepareHeader(wpt->m_WaveOut, &(wpt->m_WaveOutHeader[iBufNum]), sizeof(WAVEHDR));

    /* Prepare buffers for sending to sound interface */
    _PrepareOutBuffer(psd, iBufNum);

    /* Send buffer to driver for filling with new data */
    result=waveOutWrite(wpt->m_WaveOut, &(wpt->m_WaveOutHeader[iBufNum]), sizeof(WAVEHDR));
    _MyAssert(result == MMSYSERR_NOERROR,"waveOutWrite crashed");
    DebugCode(printf("--- _AddOutBuffer\n"););
}

/*---------------------------------------------------------------------------*/
static void _GetDoneBuffer(SndDevice_t *psd, int *piCntPrepBuf, int* piIndexDoneBuf)
{   int i;
    WaveInOut_t *wpt;
    wpt = (WaveInOut_t *)psd->pt; /* convert to correct pointer type */

    DebugCode(printf("*** _GetDoneBuffer\n"););

    /* Get number of "done"-buffers and position of one of them */
    *piCntPrepBuf = 0;
    for (i = 0; i < NUM_SOUND_BUFFERS_OUT; i++)
    {
        if (wpt->m_WaveOutHeader[i].dwFlags & WHDR_DONE)
        {
            (*piCntPrepBuf)++;
            *piIndexDoneBuf = i;
        }
    }
    DebugCode(printf("--- _GetDoneBuffer\n"););
}

/*---------------------------------------------------------------------------*/




/****************** exported, public functions *******************************/
static void _win_sndConstructor(SndDevice_t *psd)
{   int i;
    WaveInOut_t *wpt;
    wpt = (WaveInOut_t *)psd->pt; /* convert to correct pointer type */

    DebugCode(printf("*** _win_sndConstructor\n"););

    /* init complete structure */
    memset(wpt,0,sizeof(WaveInOut_t));

    wpt->m_WaveInEvent = NULL;
    wpt->m_WaveOutEvent = NULL;
    wpt->m_WaveIn = NULL;
    wpt->m_WaveOut = NULL;
    wpt->iBufferSizeIn = 0;
    wpt->iBufferSizeOut = 0;

    /* Init buffer pointer to zero */
    for (i = 0; i < NUM_SOUND_BUFFERS_IN; i++)
    {
        memset(&(wpt->m_WaveInHeader[i]), 0, sizeof(WAVEHDR));
        wpt->psSoundcardBuffer[i] = NULL;
    }
    for (i = 0; i < NUM_SOUND_BUFFERS_OUT; i++)
    {
        memset(&(wpt->m_WaveOutHeader[i]), 0, sizeof(WAVEHDR));
        wpt->psPlaybackBuffer[i] = NULL;
    }
    /* Init wave-format structure */
    wpt->sWaveFormatEx.wFormatTag = WAVE_FORMAT_PCM;
    wpt->sWaveFormatEx.nChannels = psd->nChannels;
    wpt->sWaveFormatEx.wBitsPerSample = BITS_PER_SAMPLE;
    wpt->sWaveFormatEx.nSamplesPerSec = SOUNDCARD_SAMPLE_RATE;
    wpt->sWaveFormatEx.nBlockAlign = wpt->sWaveFormatEx.nChannels *
        wpt->sWaveFormatEx.wBitsPerSample / 8;
    wpt->sWaveFormatEx.nAvgBytesPerSec = wpt->sWaveFormatEx.nBlockAlign *
        wpt->sWaveFormatEx.nSamplesPerSec;
    wpt->sWaveFormatEx.cbSize = 0;

    /* Get the number of digital audio devices in this computer, check range */
    wpt->iNumDevs = waveInGetNumDevs();
    if (wpt->iNumDevs > MAX_NUMBER_SOUND_CARDS)
        wpt->iNumDevs = MAX_NUMBER_SOUND_CARDS;
    DebugCode(printf("found %d wave devices\n",wpt->iNumDevs););

    /* At least one device must exist in the system */
    /* BugFixWin10: Windows 10 has a bug! On some computers,
       the assert below fires, although there is an audio device
       present! Work-around: I used an external USB Soundcard.
       Here: just ignore the following assert!
    */
#if BugFixWin10
    /* Win10: just ignore the following assert! */
#else
    _MyAssert(wpt->iNumDevs != 0,"No audio device found.");
#endif

    /* We use an event controlled wave-in (wave-out) structure */
    /* Create events */
    wpt->m_WaveInEvent   = CreateEvent(NULL, FALSE, FALSE, NULL);
    wpt->m_WaveOutEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    /* Set flag to open devices */
    wpt->bChangDevIn  = TRUE;
    wpt->bChangDevOut = TRUE;

    /* Default device number, "wave mapper", */
    wpt->iCurInDev  = WAVE_MAPPER;    /* will search first device due */
    wpt->iCurOutDev = WAVE_MAPPER;  /* to WAVEFORMATEX structure */

    /* blocking wave out is default */
    wpt->bBlockingPlay = TRUE; /* test: FALSE didn't make sound output! */

    /* blocking wave in is default */
    wpt->bBlockingRec = TRUE;

    DebugCode(printf("--- _win_sndConstructor\n"););
}


/*---------------------------------------------------------------------------*/
static void _win_sndDestructor(SndDevice_t *psd)
{
    int i;
    WaveInOut_t *wpt;
    wpt = (WaveInOut_t *)psd->pt; /* convert to correct pointer type */

    DebugCode(printf("*** _win_sndDestructor\n"););

    /* Delete allocated memory */
    for (i = 0; i < NUM_SOUND_BUFFERS_IN; i++)
    {   if (wpt->psSoundcardBuffer[i] != NULL)
        {   free(wpt->psSoundcardBuffer[i]);
        }
    }

    for (i = 0; i < NUM_SOUND_BUFFERS_OUT; i++)
    {   if (wpt->psPlaybackBuffer[i] != NULL)
        {   free(wpt->psPlaybackBuffer[i]);
        }
    }

    /* Close the handle for the events */
    if (wpt->m_WaveInEvent != NULL)
    {   CloseHandle(wpt->m_WaveInEvent);
    }

    if (wpt->m_WaveOutEvent != NULL)
    {   CloseHandle(wpt->m_WaveOutEvent);
    }
    DebugCode(printf("--- _win_sndDestructor\n"););
}

/*---------------------------------------------------------------------------*/
static void _win_sndInitRecording(SndDevice_t *psd, int iNewBufferSize, int bNewBlocking)
{
    /* Check if device must be opened or reinitialized, default is bNewBlocking=TRUE */
    int i;
    WaveInOut_t *wpt;
    wpt = (WaveInOut_t *)psd->pt; /* convert to correct pointer type */

    DebugCode(printf("*** _win_sndInitRecording\n"););

    if (wpt->bChangDevIn == TRUE)
    {   _OpenInDevice(psd);
        wpt->bChangDevIn = FALSE; /* Reset flag */
    }
    wpt->iBufferSizeIn = iNewBufferSize;
    wpt->bBlockingRec = bNewBlocking;

    /* Reset interface so that all buffers are returned from the interface */
    waveInReset(wpt->m_WaveIn);
    waveInStop(wpt->m_WaveIn);

    /* Reset current buffer ID (it is important to do this BEFORE calling */
    /* "AddInBuffer()" */
    wpt->iWhichBufferIn = 0;

    /* Create memory for sound card buffer */
    for (i = 0; i < NUM_SOUND_BUFFERS_IN; i++)
    {
        /* Unprepare old wave-header in case that we "re-initialized" this */
        /* module. Calling "waveInUnprepareHeader()" with an unprepared */
        /* buffer (when the module is initialized for the first time) has */
        /* simply no effect */
        waveInUnprepareHeader(wpt->m_WaveIn, &(wpt->m_WaveInHeader[i]), sizeof(WAVEHDR));

        if (wpt->psSoundcardBuffer[i] != NULL)
            free(wpt->psSoundcardBuffer[i]);

        wpt->psSoundcardBuffer[i] = (short *) malloc(sizeof(short)*(wpt->iBufferSizeIn));
        _MyAssert(wpt->psSoundcardBuffer[i] != NULL,"malloc crashed");

        /* Send all buffers to driver for filling the queue ----------------- */
        /* Prepare buffers before sending them to the sound interface */
        _PrepareInBuffer(psd,i);
        _AddInBuffer(psd);
    }

    /* Notify that sound capturing can start now */
    waveInStart(wpt->m_WaveIn);

    /* This reset event is very important for initialization, otherwise we will get errors! */
    ResetEvent(wpt->m_WaveInEvent);
    DebugCode(printf("--- _win_sndInitRecording\n"););
}

/*---------------------------------------------------------------------------*/
static void _win_sndInitPlayback(SndDevice_t *psd, int iNewBufferSize, int bNewBlocking)
{
    /* default is bNewBlocking=TRUE, test with FALSE did not produce sound! */
    int i, j;
    WaveInOut_t *wpt;
    wpt = (WaveInOut_t *)psd->pt; /* convert to correct pointer type */

    DebugCode(printf("*** _win_sndInitPlayback: iNewBufferSize=%d bNewBlocking=%d\n",
                iNewBufferSize, bNewBlocking););

    /* Check if device must be opened or reinitialized */
    if (wpt->bChangDevOut == TRUE)
    {   _OpenOutDevice(psd);
        wpt->bChangDevOut = FALSE; /* Reset flag */
    }

    /* Set internal parameters */
    wpt->iBufferSizeOut = iNewBufferSize;
    wpt->bBlockingPlay = bNewBlocking;

    /* Reset interface */
    waveOutReset(wpt->m_WaveOut);

    for (j = 0; j < NUM_SOUND_BUFFERS_OUT; j++)
    {   /* Unprepare old wave-header (in case header was not prepared before, */
        /* simply nothing happens with this function call */
        waveOutUnprepareHeader(wpt->m_WaveOut, &(wpt->m_WaveOutHeader[j]), sizeof(WAVEHDR));

        /* Create memory for playback buffer */
        if (wpt->psPlaybackBuffer[j] != NULL)
        {   DebugCode(printf("    _win_sndInitPlayback: free buffer:%d\n",j););
            free(wpt->psPlaybackBuffer[j]);
        }

        DebugCode(printf("    _win_sndInitPlayback:buffer %d malloc(%d)\n",j,sizeof(short)*(wpt->iBufferSizeOut)););
        wpt->psPlaybackBuffer[j] = (short*) malloc(sizeof(short)*(wpt->iBufferSizeOut));
        _MyAssert(wpt->psPlaybackBuffer[j] != NULL,"malloc crashed");

        /* Clear new buffer */
        DebugCode(printf("    _win_sndInitPlayback:erasing buffer %d\n",j););
        for (i = 0; i < wpt->iBufferSizeOut; i++)
        {   wpt->psPlaybackBuffer[j][i] = 0;
        }

        /* Prepare buffer for sending to the sound interface */
        _PrepareOutBuffer(psd, j);

        /* Initially, send all buffers to the interface */
        _AddOutBuffer(psd, j);
    }
    DebugCode(printf("--- _win_sndInitPlayback\n"););
}

/*---------------------------------------------------------------------------*/
static int _win_sndRead(SndDevice_t *psd, short *psData, int n)
{   int i, bError;
    WaveInOut_t *wpt;
    wpt = (WaveInOut_t *)psd->pt; /* convert to correct pointer type */

    DebugCode(printf("*** _win_sndRead\n"););

    _MyAssert(psData != NULL, "psData is NULL");
    _MyAssert(n >= 0, "n < 0");

    /* check if data buffer size fits to device initialisation */
    if(n != wpt->iBufferSizeIn)
    {   wpt->bChangDevIn = TRUE;
        wpt->iBufferSizeIn = n;
        DebugCode(printf("need adjusting iBufferSizeIn to %d\n",n););
    }

    /* Check if device must be opened or reinitialized */
    if (wpt->bChangDevIn == TRUE)
    {   _OpenInDevice(psd);

        /* Reinit sound interface */
        _win_sndInitRecording(psd, wpt->iBufferSizeIn, wpt->bBlockingRec);

        /* Reset flag */
        wpt->bChangDevIn = FALSE;
    }

    /* Wait (i.e. block this thread!) until data is available */
    if (!(wpt->m_WaveInHeader[wpt->iWhichBufferIn].dwFlags & WHDR_DONE))
    {   DebugCode(printf("   WHDR_DONE is 0...\n"););
        if (wpt->bBlockingRec == TRUE)
        {   DebugCode(printf("   blocking read...\n"););
            WaitForSingleObject(wpt->m_WaveInEvent, INFINITE);
        }
        else
            return FALSE;
    }

    /* Check if buffers got lost, i.e. we weren't quick enough... */
    {   int iNumInBufDone = 0;
        for (i = 0; i < NUM_SOUND_BUFFERS_IN; i++)
        {   if (wpt->m_WaveInHeader[i].dwFlags & WHDR_DONE)
                iNumInBufDone++;
        }
        DebugCode(printf("   iNumInBufDone = %d...\n",iNumInBufDone););
        /* If the number of done buffers equals the total number of buffers, it is */
        /* very likely that a buffer got lost -> set error flag */
        if (iNumInBufDone == NUM_SOUND_BUFFERS_IN)
            bError = TRUE;
        else
            bError = FALSE;
    }

    /* Copy data from sound card in output buffer */
    DebugCode(printf("   read: copy data...iWhichBufferIn=%d\n",wpt->iWhichBufferIn););
    DebugCode(printf("   read: dwBytesRecorded=%d\n",
             wpt->m_WaveInHeader[wpt->iWhichBufferIn].dwBytesRecorded););

    for (i = 0; i < wpt->iBufferSizeIn; i++)
    {   psData[i] = wpt->psSoundcardBuffer[wpt->iWhichBufferIn][i];
    }

    /* Add the buffer so that it can be filled with new samples */
    _AddInBuffer(psd);

    /* In case more than one buffer was ready, reset event */
    ResetEvent(wpt->m_WaveInEvent);
    DebugCode(printf("--- _win_sndRead\n"););
    return bError;
}

/*---------------------------------------------------------------------------*/
static int _win_sndWrite(SndDevice_t *psd, short *psData, int n)
{
    int i, j;
    int iCntPrepBuf;
    int iIndexDoneBuf;
    int bError;
    WaveInOut_t *wpt;
    wpt = (WaveInOut_t *)psd->pt; /* convert to correct pointer type */

    DebugCode(printf("*** _win_sndWrite\n"););

    _MyAssert(psData != NULL, "psData is NULL");
    _MyAssert(n >= 0, "n < 0");

    /* check if data buffer size fits to device initialisation */
    if(n != wpt->iBufferSizeOut)
    {   wpt->bChangDevOut = TRUE;
        wpt->iBufferSizeOut = n;
        DebugCode(printf("need adjusting iBufferSizeOut to %d\n",n););
    }
    /* Check if device must be opened or reinitialized */
    if (wpt->bChangDevOut == TRUE)
    {   DebugCode(printf("bChangDevOut=%d\n",wpt->bChangDevOut););
        _OpenOutDevice(psd);
        /* Reinit sound interface */
        _win_sndInitPlayback(psd, wpt->iBufferSizeOut, wpt->bBlockingPlay);
        /* Reset flag */
        wpt->bChangDevOut = FALSE;
    }

    /* Get number of "done"-buffers and position of one of them */
    _GetDoneBuffer(psd, &iCntPrepBuf, &iIndexDoneBuf);
    DebugCode(printf("   iCntPrepBuf:%d  iIndexDoneBuf:%d\n",iCntPrepBuf,iIndexDoneBuf););
    /* Now check special cases (Buffer is full or empty) */
    if (iCntPrepBuf == 0)
    {
        if (wpt->bBlockingPlay == TRUE)
        {   DebugCode(printf("   bBlockingPlay...\n"););
            /* Blocking wave out routine. Needed for transmitter. Always */
            /* ensure that the buffer is completely filled to avoid buffer */
            /* underruns */
            while (iCntPrepBuf == 0)
            {   DebugCode(printf("   WaitForSingleObject\n"););
                WaitForSingleObject(wpt->m_WaveOutEvent, INFINITE);
                _GetDoneBuffer(psd, &iCntPrepBuf, &iIndexDoneBuf);
            }
        }
        else
        {
            /* All buffers are filled, dump new block ----------------------- */
            /* It would be better to kill half of the buffer blocks to set the start */
            /* back to the middle: TODO */
            DebugCode(printf("   bBlockingPlay is false\n"););
            return TRUE; /* An error occurred */
        }
    }
    else if (iCntPrepBuf == NUM_SOUND_BUFFERS_OUT)
    {
        /* --------------------------------------------------------------------- */
        /* Buffer is empty -> send as many cleared blocks to the sound- */
        /* interface until half of the buffer size is reached */
        /* Send half of the buffer size blocks to the sound-interface */
        DebugCode(printf("   Buffer is empty -> send as many cleared blocks to the sound\n"););
        for (j = 0; j < NUM_SOUND_BUFFERS_OUT / 2; j++)
        {
            /* First, clear these buffers */
            for (i = 0; i < wpt->iBufferSizeOut; i++)
                wpt->psPlaybackBuffer[j][i] = 0;
            /* Then send them to the interface */
            _AddOutBuffer(psd, j);
        }

        /* Set index for done buffer */
        iIndexDoneBuf = NUM_SOUND_BUFFERS_OUT / 2;

        bError = TRUE;
    }
    else
    {   bError = FALSE;
    }

    /* Copy stereo data from input in soundcard buffer */
    for (i = 0; i < wpt->iBufferSizeOut; i++)
        wpt->psPlaybackBuffer[iIndexDoneBuf][i] = psData[i];

    /* Now, send the current block */
    _AddOutBuffer(psd, iIndexDoneBuf);

    DebugCode(printf("--- _win_sndWrite\n"););
    return bError;
}

/*---------------------------------------------------------------------------*/



static void _win_sndClose(SndDevice_t *psd)
{   int i;
    MMRESULT result;
    WaveInOut_t *wpt;
    wpt = (WaveInOut_t *)psd->pt; /* convert to correct pointer type */

    DebugCode(printf("*** _win_sndClose\n"););

    /* Reset audio driver */
    if (wpt->m_WaveOut != NULL)
    {   result = waveOutReset(wpt->m_WaveOut);
        _MyAssert(result == MMSYSERR_NOERROR, "Sound Interface, waveOutReset() failed.");
    }

    if (wpt->m_WaveIn != NULL)
    {   result = waveInReset(wpt->m_WaveIn);
        _MyAssert(result == MMSYSERR_NOERROR, "Sound Interface, waveInReset() failed.");
    }

    /* Set event to ensure that thread leaves the waiting function */
    if (wpt->m_WaveInEvent != NULL)
    {   SetEvent(wpt->m_WaveInEvent);
    }

    /* Wait for the thread to terminate */
    Sleep(500);

    /* Unprepare wave-headers */
    if (wpt->m_WaveIn != NULL)
    {
        for (i = 0; i < NUM_SOUND_BUFFERS_IN; i++)
        {
            result = waveInUnprepareHeader(
                wpt->m_WaveIn, &(wpt->m_WaveInHeader[i]), sizeof(WAVEHDR));

            _MyAssert(result == MMSYSERR_NOERROR, "Sound Interface, waveInUnprepareHeader() failed.");
        }

        /* Close the sound in device */
        result = waveInClose(wpt->m_WaveIn);
        _MyAssert(result == MMSYSERR_NOERROR, "Sound Interface, waveInClose() failed.");
    }

    if (wpt->m_WaveOut != NULL)
    {
        for (i = 0; i < NUM_SOUND_BUFFERS_OUT; i++)
        {
            result = waveOutUnprepareHeader(
                wpt->m_WaveOut, &(wpt->m_WaveOutHeader[i]), sizeof(WAVEHDR));

            _MyAssert(result == MMSYSERR_NOERROR, "Sound Interface, waveOutUnprepareHeader() failed.");
        }

        /* Close the sound out device */
        result = waveOutClose(wpt->m_WaveOut);
        _MyAssert(result == MMSYSERR_NOERROR, "Sound Interface, waveOutClose() failed.");
    }

    /* Set flag to open devices the next time it is initialized */
    wpt->bChangDevIn = TRUE;
    wpt->bChangDevOut = TRUE;
    DebugCode(printf("--- _win_sndClose\n"););

}

/******************************************************************************
 exported module functions
******************************************************************************/

/* Opens sound device in READ, WRITE or READ_WRITE mode, MONO or STEREO.
Uses default settings: 16Bit Stereo, signed short, 44100Hz sample frequency.
Windows: default size of (signed short) Buffer is 4096 (=8192 Bytes).
Linux doesn't need a buffer size at this point.
Returns pointer to structure containing all device info required by other
module functions. */
SndDevice_t *sndOpen(int rw_mode, int mono_stereo)
{   SndDevice_t *psd;

    psd=(SndDevice_t*)malloc(sizeof(SndDevice_t));
    _MyAssert(psd!=NULL,"sndOpen:malloc() crashed");
    psd->pt=(WaveInOut_t*) malloc(sizeof(WaveInOut_t));
    _MyAssert(psd->pt!=NULL,"sndOpen:malloc() crashed");

    switch (mono_stereo)
    {   case SND_MONO:      psd->nChannels=SND_MONO;
                            break;
        case SND_STEREO:    psd->nChannels=SND_STEREO;
                            break;
        default:            fprintf(stderr,"snd_open: use either SND_MONO or SND_STEREO");
                            exit(-1);
                            break;
    }
    /* rw_mode is not important for Windows here  */
    if((rw_mode !=SND_READ_ONLY)&&
       (rw_mode !=SND_WRITE_ONLY)&&
       (rw_mode !=SND_READ_WRITE))
    {   fprintf(stderr,"snd_open: SND_READ_ONLY, SND_WRITE_ONLY or SND_READ_WRITE");
        exit(-1);
    }
    _win_sndConstructor(psd);
    return psd;
}
/*****************************************************************************/

/* Closes the sound device (Windows: 2 devices if read/write) and frees all
allocated memory. Return 0 for ok, negative integer on error */
SndDevice_t *sndClose(SndDevice_t *psd)
{   _win_sndClose(psd);
    _win_sndDestructor(psd);
    free(psd->pt);
    free(psd);
    return NULL;
}
/*****************************************************************************/

/* Performs blocking read. Buffer and buffer size (signed short) are parameters.
Windows: adjust internal buffers accordingly
Linux: ensure buffer size is a multiple of the sound card internal buffer size
in order to prevent crashes!
Returns number of (signed short) elements read or negative integer on error */
int sndRead(SndDevice_t *psd, short *buf, int buf_elements)
{   if(0==_win_sndRead(psd, buf, buf_elements))
        return buf_elements;
    else
        return -1; //error
}
/*****************************************************************************/

/* Performs blocking write. Buffer and buffer size (signed short) are parameters.
Windows: adjust internal buffers accordingly
Linux: ensure buffer size is a multiple of the sound card internal buffer size
in order to prevent crashes!
Returns number of (signed short) elements written  or negative integer on error */
int sndWrite(SndDevice_t *psd, short *buf, int buf_elements)
{   if(TRUE==_win_sndWrite(psd, buf, buf_elements))
        return -1; //error
    else
        return buf_elements;

}

/*****************************************************************************/


/***************************************************************
* Windows Version of int sndWAVPlaySound(char *Filename)
****************************************************************/
int sndWAVPlaySound(char *Filename)
{    if(TRUE != PlaySound(Filename, NULL , SND_ASYNC | SND_FILENAME | SND_NODEFAULT))
    { _errMsg("WAVOutPlaySound, PlaySound has crashed...");
      return -1;
    }
    return 0;
}


/******************************************************************************
End of Windows implementation:
******************************************************************************/
#endif  /* (PLATFORM==OS_MS_WINDOWS) */









#if (PLATFORM==OS_LINUX)

/*******************************************************************
Linux OSS implementation: old version, open sound system
******************************************************************/
#if LINUX_OSS
/*
       _/        _/   _/       _/    _/      _/  _\   _/
      _/        _/   _/ _\    _/    _/      _/    _\ _/
     _/        _/   _/   _\  _/    _/      _/      _X
    _/        _/   _/     _\_/    _/      _/        _\
   _/_/_/_/  _/   _/       _/     _/_/_/_/       _/  _\

         _/_/_/_/      _/_/_/_/   _/_/_/_/
       _/       _/  _/          _/
      _/       _/    _/_/_/_/    _/_/_/_/
     _/       _/           _/          _/
      _/_/_/_/     _/_/_/_/    _/_/_/_/


*/

/*
IDE: vide mit gcc Compiler, Suse 10.0

27.8.07:
old snd_util functions copied to this file. No mixer support, only
/dev/dsp support.
*/



/* this module will support only one sound device */
#define SOUND_DEVICE  "/dev/dsp"

/*************** private prototypes ****************************/
static int _sndSetDSPToFullDuplexMode(int fd);
static int _sndSetDSPAudioFormat8BitUnsigned(int fd);
static int _sndSetDSPAudioFormat16BitSigned(int fd);
static int _sndSetDSPAudioMono(int fd);
static int _sndSetDSPAudioStereo(int fd);
static int _sndSetDSPSamplingFrequency8000Hz(int fd);
static int _sndSetDSPSamplingFrequency11025Hz(int fd);
static int _sndSetDSPSamplingFrequency22050Hz(int fd);
static int _sndSetDSPSamplingFrequency44100Hz(int fd);
static int _sndDSPGetInternalBlockSizeInBytes(int fd, int *blocksize);
static int _sndDSPWriteBytes(int fd, char *buf, int len);
static int _sndDSPReadBytes(int fd, char* buf, int len);
/*----------------------------------------------------------------*/



/*************** implementation of private functions ***********/



/**************************************************************/
/* set to full duplex mode immediately after open() */
/*!
 ****************************************************************
 * @par Description:
 *   Schaltet die Soundkarte in den 'full duplex mode', also
 *   gleichzeitiges Aufnehmen und Wiedergeben. Nicht alle
 *   Soundkarten sind dazu in der Lage. Diese Funktion
 *   sollte unmittelbar nach dem Oeffnen der Soundkarte aufgerufen
 *   werden.
 *
 *
 * @see
 * @arg
 *
 * @param  fp    -  IN, file-descriptor des bereits geoeffneten dsp-Device
 *
 * @retval 0 for ok, -1 on error
 *
 *******************************************************************/
static int _sndSetDSPToFullDuplexMode(int fd)
{ int arg=0, retval=0;

  if(ioctl(fd, SNDCTL_DSP_SETDUPLEX,0) == -1) {
    retval = -1;
    _errMsg("SNDCTL_DSP_SETDUPLEX ioctl failed");
  }
  if(ioctl(fd, SNDCTL_DSP_GETCAPS, &arg)== -1) {
    _errMsg("SNDCTL_DSP_GETCAPS ioctl failed");
    retval = -1;
  }
  if ((arg & DSP_CAP_DUPLEX) == 0) {
    _errMsg("Soundcard not full duplex capable!");
    retval = -1;
  }
  return retval;
}

/*----------------------------------------------------------------*/
/*********************************************************/
/*!
 ****************************************************************
  @par Description:
    Schaltet die Soundkarte auf das Datenformat 8 Bit, vorzeichenlos
    (entspricht 'unsigned char', der Ruhepegel liegt bei 127)

  @see
  @arg

  @param  fp    -  IN, Zeiger auf das bereits geoeffnete dsp-Device

  @retval 0 for ok, -1 on error

 ****************************************************************/
static int _sndSetDSPAudioFormat8BitUnsigned(int fd)
{ int format=AFMT_U8;

  if(ioctl(fd, SNDCTL_DSP_SETFMT, &format) == -1) {
     _errMsg("can't set audio format");
     return -1;
   }
  return 0;
}

/*----------------------------------------------------------------*/
/*!
 ***********************************************************************
 * @par Description:
 *   Schaltet die Soundkarte auf das Datenformat 16 Bit mit Vorzeichen
 *   (entspricht 'short', der Ruhepegel liegt bei 0)
 *
 * @see
 * @arg
 *
 * @param  fp    -  IN, Zeiger auf das bereits geoeffnete dsp-Device
 *
 * @retval 0 for ok, -1 on error
 *
 **************************************************************/
static int _sndSetDSPAudioFormat16BitSigned(int fd)
{ int format=AFMT_S16_LE; /* little endian, Intel processor */

  if(ioctl(fd, SNDCTL_DSP_SETFMT, &format) == -1) {
     _errMsg("can't set audio format");
     return -1;
   }
  return 0;
}

/*----------------------------------------------------------------*/
/*********************************************************/
/* set mono / stereo */
/*!
 ********************************************************************
 * @par Description:
 *   Schaltet die Soundkarte auf Mono
 *
 * @see
 * @arg
 *
 * @param  fp    -  IN, Zeiger auf das bereits geoeffnete dsp-Device
 *
 * @retval 0 for ok, -1 on error
 *
 *********************************************************************/
static int _sndSetDSPAudioMono(int fd)
{ int format=0;

  if(ioctl(fd, SNDCTL_DSP_STEREO, &format) == -1) {
     _errMsg("can't set to mono");
     return -1;
   }
  return 0;
}
/*********************************************************/
/*!
 ******************************************************************
 * @par Description:
 *   Schaltet die Soundkarte auf Stereo
 *
 * @see
 * @arg
 *
 * @param  fp    -  IN, Zeiger auf das bereits geoeffnete dsp-Device
 *
 * @retval 0 for ok, -1 on error
 *
 ******************************************************************/
static int _sndSetDSPAudioStereo(int fd)
{ int format=1;

  if(ioctl(fd, SNDCTL_DSP_STEREO, &format) == -1) {
     _errMsg("can't set to stereo");
     return -1;
   }
  return 0;
}
/*********************************************************/

/*----------------------------------------------------------------*/

/*!
 *************************************************************
  @par Description:
    Setzt die Abtastfrequenz der Soundkarte auf 8000Hz

  @see
  @arg

  @param  fp    -  IN, Zeiger auf das bereits geoeffnete dsp-Device

  @retval 0 for ok, -1 on error

 **************************************************************/
static int _sndSetDSPSamplingFrequency8000Hz(int fd)
{ int frequencyHz=8000;

  if(ioctl(fd, SNDCTL_DSP_SPEED, &frequencyHz) == -1) {
     _errMsg("can't set sampling frequency");
     return -1;
   }
  return 0;
}
/*********************************************************/


/*!
 ***************************************************************
  @par Description:
    Setzt die Abtastfrequenz der Soundkarte auf 11025Hz

  @see
  @arg

  @param  fp    -  IN, Zeiger auf das bereits geoeffnete dsp-Device

  @retval 0 for ok, -1 on error

 **************************************************************/
static int _sndSetDSPSamplingFrequency11025Hz(int fd)
{ int frequencyHz=11025;

  if(ioctl(fd, SNDCTL_DSP_SPEED, &frequencyHz) == -1) {
     _errMsg("can't set sampling frequency");
     return -1;
   }
  return 0;
}
/*********************************************************/


/*!
 **************************************************************
  @par Description:
    Setzt die Abtastfrequenz der Soundkarte auf 22050Hz

  @see
  @arg

  @param  fp    -  IN, Zeiger auf das bereits geoeffnete dsp-Device

  @retval 0 for ok, -1 on error

 *************************************************************/
static int _sndSetDSPSamplingFrequency22050Hz(int fd)
{ int frequencyHz=22050;

  if(ioctl(fd, SNDCTL_DSP_SPEED, &frequencyHz) == -1) {
     _errMsg("can't set sampling frequency");
     return -1;
   }
  return 0;
}
/*********************************************************/


/*********************************************************/
/*!
 ***************************************************************
  @par Description:
    Setzt die Abtastfrequenz der Soundkarte auf 44100Hz

  @see
  @arg

  @param  fp    -  IN, Zeiger auf das bereits geoeffnete dsp-Device

  @retval 0 for ok, -1 on error

 **************************************************************/
static int _sndSetDSPSamplingFrequency44100Hz(int fd)
{ int frequencyHz=44100;

  if(ioctl(fd, SNDCTL_DSP_SPEED, &frequencyHz) == -1) {
     _errMsg("can't set sampling frequency");
     return -1;
   }
  return 0;
}

/*----------------------------------------------------------------*/



/*!
 ***********************************************************************
 * @par Description:
 *   Ermittelt die Groesse des interne Datenpuffers der Soundkarte in Bytes.
 *   Bei manchen Soundkarten kann man nicht beliebig viele Bytes auf einmal
 *   auslesen / schreiben (sonst Absturz...), sondern sollte immer ein
 *   ganzzahliges Vielfaches
 *   dieser Blockgroesse auslesen.
 *
 * @see
 * @arg
 *
 * @param  fp    -  IN, Zeiger auf das bereits geoeffnete dsp-Device
 * @param  blocksize - OUT, Groesse des internen Puffers in Byte
 *
 * @retval 0 for ok, -1 on error
 *
 ********************************************************************/
static int _sndDSPGetInternalBlockSizeInBytes(int fd, int *blocksize)
{  if ( ioctl(fd,SNDCTL_DSP_GETBLKSIZE, blocksize)==-1 ) {
      perror("Sound driver ioctl SNDCTL_DSP_GETBLKSIZE");
      return -1;
    }
    else
      return 0;
}


/*!
 ***********************************************************************
 * @par Description:
 *   Schreibt eine Anzahl len Bytes aus dem Buffer buf auf den Ausgang
 *   der Soundkarte. Auch wenn die Soundkarte auf 16 Bit Format
 *   eingestellt ist, wird ein Zeiger auf ein Byte-Feld uebergeben.
 *   Um also ein short-Feld mit 4096 16-Bit Werten auszugeben,
 *   muss man die Startadresse des Feldes uebergeben und als len den
 *   Wert 4096*sizeof(short) angeben. Achtung: der Parameter len ist
 *   u.U. nicht beliebig, siehe SND_BUFFER_SIZE_BYTE
 *
 * @see
 * @arg SND_BUFFER_SIZE_BYTE
 *
 * @param  fp    -  IN, Zeiger auf das bereits geoeffnete dsp-Device
 * @param  buf   - IN, Startadresse des Feldes, das die zu schreibenden
                  Daten enthaelt.
 * @param  len   - IN, Feldgroesse in Byte
 *
 * @retval 0 for ok, -1 on error
 *
 **********************************************************************/
static int _sndDSPWriteBytes(int fd, char *buf, int len)
{ static int DspBlockSize=-1;  /* internal driver buffer size in bytes */
  int retval=0;
  int BytesToSend = len;
  char *pt;

  if (DspBlockSize < 0) {
    if (0!=_sndDSPGetInternalBlockSizeInBytes(fd,&DspBlockSize)) {
      perror("Sound driver ioctl ");
      retval = -1;
    }
    //printf("block size: %d \n",DspBlockSize);
    //ioctl(fd, SNDCTL_DSP_SYNC, 0);  // write waits until samples a played
  }
  /* don't send more than the dsp can take at a time! */
  pt = buf;

  //printf("BytesToSend:%d DspBlockSize:%d retval:%d \n",
  //        BytesToSend, DspBlockSize, retval);

  while((BytesToSend >= DspBlockSize) && (retval==0))
  {  if(-1==write(fd, pt, DspBlockSize)) {
       perror("loop cannot write to device");
       retval = -1;
       /* debug */
       //printf("fd=%d, pt=%p, buf=%p, DspBlockSize=%d len=%d\n",
       //        fd,pt, buf, DspBlockSize, len);
       /* end of debug */
     }
     pt          += DspBlockSize;
     BytesToSend -= DspBlockSize;
     //printf(".");
  }

  /* on my PC, ('euratio', intel8x0 sound driver) it is only possible
     to write data chunks of DspBlockSize bytes each. Other block sizes
     will result in an error 'Speicherzugriffsfehler' on close() and/or
     program termination. Therefore: dont send any remaining bytes, that
     don't fit into the proper block size.
  */
  if(BytesToSend > 0)
    perror("use only multiples of DSP internal block size! Data truncated...");
#if 0
  if((BytesToSend > 0) && (retval==0)) { /* send the rest... */
     if(-1==write(fd, pt, BytesToSend)) {
       perror("cannot write to device");
       retval = -1;
     }
  }
#endif
  return retval;
}
/*********************************************************/

/*!
 ***********************************************************************
 * @par Description:
 *   Liest eine Anzahl len Bytes in den Buffer buf vom Eingang der
 *   Soundkarte. Auch wenn die Soundkarte auf 16 Bit Format eingestellt
 *   ist, wird ein Zeiger auf ein Byte-Feld uebergeben. Um also ein
 *   short-Feld mit 4096 16-Bit Werten einzulesen,
 *   muss man die Startadresse des Feldes uebergeben und als len den
 *   Wert 4096*sizeof(short)
 *   angeben. Achtung: der Parameter len ist u.U. nicht beliebig, siehe
 *   sndDSPGetInternalBlockSizeInBytes()
 *
 * @see
 * @arg sndDSPGetInternalBlockSizeInBytes
 *
 * @param  fp    -  IN, Zeiger auf das bereits geoeffnete dsp-Device
 * @param  buf   - OUT, Startadresse des Feldes, das die zu lesenden
                  Daten aufnimmt.
 * @param  len   - IN, Feldgroesse in Byte
 *
 * @retval 0 for ok, -1 on error
 *
 **********************************************************************/
static int _sndDSPReadBytes(int fd, char* buf, int len)
{
  static int DspBlockSize=-1;  /* internal driver buffer size in bytes */
  int retval=0;
  int BytesToRead = len;
  char *pt;


  if (DspBlockSize < 0) {
    if (0!=_sndDSPGetInternalBlockSizeInBytes(fd,&DspBlockSize)) {
      perror("Sound driver ioctl ");
      retval = -1;
    }
    printf("block size: %d \n",DspBlockSize);
  }
  /* don't read more than the dsp can take at a time! */
  pt = buf;
  while((BytesToRead >= DspBlockSize) && (retval==0))
  {  if(-1==read(fd, pt, DspBlockSize)) {
       perror("loop cannot read from device");
       retval = -1;
       /* debug */
       printf("fd=%d, pt=%p, buf=%p, DspBlockSize=%d len=%d\n",
               fd,pt, buf, DspBlockSize, len);
       /* end of debug */
     }
     pt          += DspBlockSize;
     BytesToRead -= DspBlockSize;
  }

#if 1
  /* on my PC, ('euratio', intel8x0 sound driver) it is only possible
     to write/reda data chunks of DspBlockSize bytes each.
     Other block sizes
     will result in an error 'Speicherzugriffsfehler' on close() and/or
     program termination. Therefore: dont read any remaining bytes,
     that don't fit into the proper block size.
  */
  if((BytesToRead > 0) && (retval==0)) { /* send the rest... */
     perror("use only multiples of internal DSP block size! Data truncated...");
  }
#endif
  return retval;

}


/*----------------------------------------------------------------*/

/********************************************************************
 exported module functions
********************************************************************/

/* Opens sound device in READ, WRITE or READ_WRITE mode, MONO or STEREO. Uses default settings: 16Bit Stereo, signed short, 44100Hz sample frequency.
Windows: default size of (signed short) Buffer is 4096 (=8192 Bytes).
Linux: /dev/dsp is the default sound device. Linux doesn't need a
buffer size at this point.
Returns pointer to structure containing all device info required by other module functions. */
SndDevice_t *sndOpen(int rw_mode, int mono_stereo)
{   SndDevice_t *psd;
    int berror=0;

    psd=(SndDevice_t*)malloc(sizeof(SndDevice_t));
    _MyAssert(psd!=NULL,"sndOpen:malloc() crashed");

    switch(rw_mode)
    {   case SND_READ_ONLY:     if((psd->fd = open(SOUND_DEVICE, O_RDONLY)) == -1){
                                    perror("cannot open dsp device");
                                    berror = 1;
                                 }
                                 break;
        case SND_WRITE_ONLY:    if((psd->fd = open(SOUND_DEVICE, O_WRONLY)) == -1){
                                    perror("cannot open dsp device"); berror = 1;
                                 }
                                 break;
        case SND_READ_WRITE:    if((psd->fd = open(SOUND_DEVICE, O_RDWR)) == -1){
                                    perror("cannot open dsp device"); berror = 1;
                                 }
                                 if(0!=_sndSetDSPToFullDuplexMode(psd->fd)) {
                                    perror("cannot set full duplex mode"); berror = 1;
                                 }
                                 break;
        default:               fprintf(stderr,"snd_open: use either SND_READ_ONLY,"
                                         " SND_WRITE_ONLY or SND_READ_WRITE");
                                 berror = 1;
    }

    if(!berror)
    {   if(0!=_sndSetDSPSamplingFrequency44100Hz(psd->fd)){
            perror("_sndSetDSPSamplingFrequency44100Hz"); berror=1;
        }
        if(0!=_sndSetDSPAudioFormat16BitSigned(psd->fd)){
            perror("_sndSetDSPAudioFormat16BitSigned"); berror=1;
        }
        switch (mono_stereo)
        {   case SND_MONO:     psd->nChannels=SND_MONO;
                                if(0!=_sndSetDSPAudioMono(psd->fd)){
                                    perror("_sndSetDSPAudioMono"); berror=1;
                                }
                                break;
            case SND_STEREO:   psd->nChannels=SND_STEREO;
                                if(0!=_sndSetDSPAudioStereo(psd->fd)){
                                    perror("_sndSetDSPAudioStereo"); berror=1;
                                }
                                break;
            default:          fprintf(stderr,"sndOpen: use either SND_MONO or SND_STEREO");
                                berror=1;
                                break;
        }
    }

    if(berror)
    {   perror("sndOpen has crashed!");
        free(psd);
        return NULL;
    }
    else return psd;
}
/******************************************************************/

/* Closes the sound device  and frees all
allocated memory. Return 0 for ok, negative integer on error */
SndDevice_t *sndClose(SndDevice_t *psd)
{   if(psd!=NULL)
    {   close(psd->fd);
        free(psd);
    }
    return NULL;
}
/*******************************************************************/

/* Performs blocking read. Buffer and buffer size (signed short) are
   parameters.Linux: ensure buffer size is a multiple of the sound
   card internal buffer size in order to prevent crashes!
   Returns number of (signed short) elements read or negative integer
   on error */
int sndRead(SndDevice_t *psd, short *buf, int buf_elements)
{   if(0!=_sndDSPReadBytes(psd->fd, (char *)buf, sizeof(short)*buf_elements))
    {   perror("_sndDSPReadBytes has crashed...");
        return -1; //error
    }
    return buf_elements;
}
/*******************************************************************/

/* Performs blocking write. Buffer and buffer size (signed short) are
   parameters.
   Linux: ensure buffer size is a multiple of the sound card internal
   buffer size in order to prevent crashes!
   Returns number of (signed short) elements written or negative integer
   on error */
int sndWrite(SndDevice_t *psd, short *buf, int buf_elements)
{   if(0!=_sndDSPWriteBytes(psd->fd, (char *)buf, sizeof(short)*buf_elements))
    {   perror("sndWrite: can't play audio data");
        return buf_elements;
    }
    return buf_elements;

}

/*----------------------------------------------------------------*/


/***************************************************************
* Linux OSS Version of int sndWAVPlaySound(char *Filename)
****************************************************************/
int sndWAVPlaySound(char *Filename)
{ int DspBlockSizeBytes=0, WaveDspSizeBytes=0, WaveFileDataSizeBytes=0;
  FILE *fp=NULL;
  int fd_dsp =-1;
  char *buf=NULL;
  sndWaveHeader_t wh;

  if((fp=fopen(Filename, "rb")) == NULL) {
    _errMsg("sndWAVPlaySound: cannot open file...");
    return -1;
  }
  if(0!=sndWAVReadFileHeader(fp, &wh)) {
    _errMsg("sndWAVPlaySound: cannot read WAV file header");
    fclose(fp);
    return -1;
  }
  /* can we understand this format? */
  if((wh.format != 1) ||    /* only PCM */
     ((wh.nChannels!=1)&&(wh.nChannels!=2)) || /* only mono or stereo */
     ((wh.nBytesPerSample!=1)&&(wh.nBytesPerSample!=2)&&(wh.nBytesPerSample!=4))) {
    _errMsg("sndWAVPlaySound: unkown format...");
    fclose(fp);
    return -1;
  }
  if((fd_dsp = open("/dev/dsp", O_WRONLY)) == -1){
    _errMsg("sndWAVPlaySound: cannot open dsp device");
    fclose(fp);
    return -1;
  }
  if(0!=_sndDSPGetInternalBlockSizeInBytes(fd_dsp, &DspBlockSizeBytes)){
    _errMsg("sndWAVPlaySound: sndDSPGetInternalBlockSizeInBytes");
    fclose(fp);
    close(fd_dsp);
    return -1;
  }
  {  /* adjust buffer size to a multiple of dsp buffer size: */
     int n;
     WaveFileDataSizeBytes = wh.data_length;
     n=WaveFileDataSizeBytes / DspBlockSizeBytes;
     if(0!=WaveFileDataSizeBytes % DspBlockSizeBytes) n=n+1;
     WaveDspSizeBytes = n*DspBlockSizeBytes;
#if 0
    printf("wh.data_length:%lu\n", wh.data_length);
    printf("DspBlockSizeBytes:%d\n", DspBlockSizeBytes);
    printf("WaveFileDataSizeBytes:%d\n", WaveFileDataSizeBytes);
    printf("WaveDspSizeBytes:%d\n", WaveDspSizeBytes);
    printf("n:%d\n", n);
#endif
     assert(WaveDspSizeBytes >= WaveFileDataSizeBytes);
  }
  if(NULL == (buf=(char*)malloc(WaveDspSizeBytes*sizeof(char)))){
    _errMsg("sndWAVPlaySound: cannot malloc()");
    fclose(fp);
    close(fd_dsp);
    return -1;
  }
  memset(buf,0,WaveDspSizeBytes); /* erase buffer */

  if(WaveFileDataSizeBytes!=fread(buf,sizeof(char),WaveFileDataSizeBytes,fp)) {
    _errMsg("sndWAVPlaySound: cannot fread()");
    fclose(fp);
    close(fd_dsp);
    free(buf);
    return -1;
  }
  /* configure /dev/dsp according to WAV file parameters */
  { int ok=1;
    switch(wh.nChannels)  /* only mono or stereo */
    {   case 1: if(0!=_sndSetDSPAudioMono(fd_dsp)){ ok=0;} break;
        case 2: if(0!=_sndSetDSPAudioStereo(fd_dsp)){ ok=0;} break;
        default: ok=0; break;
    }
    switch(wh.nBytesPerSample) /* only 8 or 16 bit, mono or stereo */
    {   case 1: if(0!=_sndSetDSPAudioFormat8BitUnsigned(fd_dsp)){ ok=0;} break;
        case 2: switch(wh.nChannels)  /* only mono or stereo */
                {   case 1: if(0!=_sndSetDSPAudioFormat16BitSigned(fd_dsp)){ ok=0;} break;
                    case 2: if(0!=_sndSetDSPAudioFormat8BitUnsigned(fd_dsp)){ ok=0;} break;
                    default: ok=0; break;
                }
                break;
        case 4: if(0!=_sndSetDSPAudioFormat16BitSigned(fd_dsp)){ ok=0;} break;
        default: ok=0; break;
    }
    switch(wh.nSamplesPerSec)
    {   case 8000:  if(0!=_sndSetDSPSamplingFrequency8000Hz(fd_dsp)){ ok=0;} break;
        case 11025: if(0!=_sndSetDSPSamplingFrequency11025Hz(fd_dsp)){ ok=0;} break;
        case 22050: if(0!=_sndSetDSPSamplingFrequency22050Hz(fd_dsp)){ ok=0;} break;
        case 44100: if(0!=_sndSetDSPSamplingFrequency44100Hz(fd_dsp)){ ok=0;} break;
        default: ok=0; break;
    }
    if(!ok) {
        _errMsg("sndWAVPlaySound: configure sound device");
        fclose(fp);
        close(fd_dsp);
        free(buf);
        return -1;
    }
  }
  /* write data to sound device */
  if(0!=_sndDSPWriteBytes(fd_dsp, buf, WaveDspSizeBytes)){
      _errMsg("sndWAVPlaySound: cannot write to sound device");
      fclose(fp);
      close(fd_dsp);
      free(buf);
      return -1;
  }

  fclose(fp);
  close(fd_dsp);
  return 0;
}
/*----------------------------------------------------------------*/



#endif //LINUX_OSS
/*******************************************************************
*              End of Linux OSS implementation
******************************************************************/
















/*******************************************************************
Linux ALSA implementation: advanced linux sound architecture
******************************************************************/
#if LINUX_ALSA
/*
       _/        _/   _/       _/    _/      _/  _\   _/
      _/        _/   _/ _\    _/    _/      _/    _\ _/
     _/        _/   _/   _\  _/    _/      _/      _X
    _/        _/   _/     _\_/    _/      _/        _\
   _/_/_/_/  _/   _/       _/     _/_/_/_/       _/  _\

        _/_\          _/        _/_/_/_/   _/_\
       _/  _\        _/       _/          _/  _\
      _/    _\      _/        _/_/_/_/   _/    _\
     _/_/_/_/_\    _/               _/  _/_/_/_/_\
    _/        _\  _/_/_/_/  _/_/_/_/   _/        _\

*/

/*
Taken from: http://www.linuxjournal.com/article/6735?page=0,1
linker: libasound, libm, maybe needs: libdl for function dlopen()

                read/write pointer
               |
  ---------------------------------
  | | | | | | | | | | | | | | | | |  buffer = 16 periods
  ---------------------------------
               |
        -----------------
        | | | | | | | | |  period = 8 frames
        -----------------
               |
             -----
             |L|R| frame = 2 samples(left+right)
             -----
              |
            -----
            |L|M| sample = 2 bytes (LSB, MSB)
            -----
*/


/*----------------------------------------------------------------*/
/***************************************************************/
/*************** private prototypes ****************************/
/***************************************************************/
static int _snd_pcm_open_capture(SndDevice_t *psd);
static int _snd_pcm_open_playback(SndDevice_t *psd);

static int _snd_pcm_set_parameters(SndDevice_t *psd, snd_pcm_t *handle,
            int mono_stereo, int sampleFrequencyHz);

static int _snd_pcm_write_bytes(snd_pcm_t *handle, char *buf, int buf_len_bytes, int frame_size_bytes);
static int _snd_pcm_read_bytes(snd_pcm_t *handle, char* buf, int buf_len_bytes, int frame_size_bytes);

static int _snd_pcm_getInternalBlockSizeInBytes(SndDevice_t *psd);

/***************************************************************/
/*************** end of private prototypes *********************/
/***************************************************************/



/***************************************************************/
/*************** implementation of private functions ***********/
/***************************************************************/
static int _snd_pcm_open_capture(SndDevice_t *psd)
{   int rc;

    /* open in blocking mode */
    rc = snd_pcm_open(&(psd->pcm_handle_capture),
         "default", SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0)
    {   perror("cannot open dsp capture device");
        return -1;
    }
    return 0;  // OK
}
/*-------------------------------------------------------------*/
static int _snd_pcm_open_playback(SndDevice_t *psd)
{   int rc;

    /* Open PCM device for playback, blocking mode. */
    rc = snd_pcm_open(&(psd->pcm_handle_playback),
         "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0)
    {   perror("cannot open dsp playback device");
        return -1;
    }
    return 0;  // OK
}
/*-------------------------------------------------------------*/
static int _snd_pcm_set_parameters(SndDevice_t *psd, snd_pcm_t *handle,
           int mono_stereo, int sampleFrequencyHz)
{   /* This structure contains information about    */
    /* the hardware and can be used to specify the  */
    /* configuration to be used for the PCM stream. */
    snd_pcm_hw_params_t *hwparams;
    unsigned int rate = sampleFrequencyHz; /* Sample rate */
    unsigned int exact_rate;   /* Sample rate returned by */
                               /* snd_pcm_hw_params_set_rate_near */
    int nchannels;
    snd_pcm_uframes_t frames;
    int frame_size_bytes;
    int dir;
    snd_pcm_uframes_t buffer_size_frames;
    snd_pcm_uframes_t exact_buffer_size_frames;


    /* Allocate the snd_pcm_hw_params_t structure  */
    snd_pcm_hw_params_alloca(&hwparams);

    /* Init hwparams with full configuration space */
    if (snd_pcm_hw_params_any(handle, hwparams) < 0) {
      fprintf(stderr, "Can not configure this PCM device.\n");
      return(-1);
    }

    /* Set access type. This can be either    */
    /* SND_PCM_ACCESS_RW_INTERLEAVED or       */
    /* SND_PCM_ACCESS_RW_NONINTERLEAVED.      */
    if (snd_pcm_hw_params_set_access(handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
      fprintf(stderr, "Error setting access.\n");
      return(-1);
    }

    /* Set sample format */
    if (snd_pcm_hw_params_set_format(handle, hwparams, SND_PCM_FORMAT_S16_LE) < 0) {
      fprintf(stderr, "Error setting format.\n");
      return(-1);
    }

    /* Set sample rate. If the exact rate is not supported */
    /* by the hardware, use nearest possible rate.         */
    exact_rate = rate;
    if (snd_pcm_hw_params_set_rate_near(handle, hwparams, &exact_rate, 0) < 0) {
      fprintf(stderr, "Error setting rate.\n");
      return(-1);
    }
    if (rate != exact_rate) {
      fprintf(stderr, "The rate %d Hz is not supported by your hardware.\n"
                      "==> Using %d Hz instead.\n", rate, exact_rate);
    }

    /* Set number of channels */
    switch(mono_stereo)
    {   case SND_MONO:
            nchannels = 1;

            break;
        case SND_STEREO:
            nchannels = 2;
            break;
        default:
            fprintf(stderr,"Warning! Use either SND_MONO or SND_STEREO!\n");
            fprintf(stderr,"I am using SND_STEREO now...\n");
            nchannels = 2;
            break;
    } // switch
    /* One frame is the sample data vector for all channels. */
    /* For 16 Bit stereo data, one frame has a length of four bytes. */
    frame_size_bytes = sizeof(short)*nchannels;
    buffer_size_frames = SND_BUFFER_SIZE_BYTE / frame_size_bytes;
    DebugCode(printf("Debugging: frame size:%d bytes buffer size:%d bytes\n",
              (int)frame_size_bytes, (int)buffer_size_frames););

    if (snd_pcm_hw_params_set_channels(handle, hwparams, nchannels) < 0) {
      fprintf(stderr, "Error setting channels.\n");
      return(-1);
    }

    /* Set period size to 32 frames. */
    frames = 32;
    snd_pcm_hw_params_set_period_size_near(handle, hwparams, &frames, &dir);

    DebugCode(printf("Debugging: frames:%d dir:%d\n", (int)frames, dir););


    /* Set buffer size (in frames). The resulting latency is given by */
    /* latency = periodsize * periods / (rate * bytes_per_frame)     */
    exact_buffer_size_frames = buffer_size_frames;
    if (snd_pcm_hw_params_set_buffer_size_near(handle, hwparams, &exact_buffer_size_frames) < 0) {
      fprintf(stderr, "Error setting buffersize.\n");
      return(-1);
    }
    if (buffer_size_frames != exact_buffer_size_frames) {
      fprintf(stderr, "The buffer size %d frames is not supported by your hardware.\n"
                      "==> Using %d frames instead.\n", (int)buffer_size_frames, (int)exact_buffer_size_frames);
    }
    DebugCode(printf("Debugging: buffer size:%d frames\n", (int)exact_buffer_size_frames););


    /* Apply HW parameter settings to */
    /* PCM device and prepare device  */
    if (snd_pcm_hw_params(handle, hwparams) < 0) {
      fprintf(stderr, "Error setting HW params.\n");
      return(-1);
    }

    /* write exact parameters to device structure */
    psd->buffer_size_frames = buffer_size_frames;
    psd->nChannels = nchannels;
    psd->frame_size_bytes = frame_size_bytes;


    return 0;
}
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/

static int _snd_pcm_write_bytes(snd_pcm_t *handle, char *buf, int buf_len_bytes, int frame_size_bytes)
{
    int num_frames, frames_written, bytes_written;

    /* we only use 16 bit data. One frame is the sample data vector for all channels. */
    /* For 16 Bit stereo data, one frame has a length of four bytes. */
    num_frames = buf_len_bytes / frame_size_bytes;

    /* Write num_frames frames from buffer data to    */
    /* the PCM device pointed to by pcm_handle.       */
    /* Returns the number of frames actually written. */
    frames_written = snd_pcm_writei(handle, buf, num_frames);
    if(frames_written == -EPIPE)
    {   /* EPIPE means underrun */
        fprintf(stderr, "underrun occurred\n");
        snd_pcm_prepare(handle);
    }
    else
    {   if (frames_written < 0)
        {   fprintf(stderr, "error from snd_pcm_writei: %s\n", snd_strerror(frames_written));
        }
        else
        {   if(frames_written != num_frames)
            {   fprintf(stderr, "short write, write %d frames\n", frames_written);
            }
        }
    }
    bytes_written = frames_written * frame_size_bytes;

    return bytes_written;
}
/*-------------------------------------------------------------*/
static int _snd_pcm_read_bytes(snd_pcm_t *handle, char* buf, int buf_len_bytes, int frame_size_bytes)
{
    int num_frames, rc, bytes_read;

    /* we only use 16 bit data. One frame is the sample data vector for all channels. */
    /* For 16 Bit stereo data, one frame has a length of four bytes. */
    num_frames = buf_len_bytes / frame_size_bytes;

    /* try to read num_frames frames from pcm capture device */
    rc = snd_pcm_readi(handle, buf, num_frames);
    if (rc == -EPIPE) {
      /* EPIPE means overrun */
      fprintf(stderr, "overrun occurred\n");
      snd_pcm_prepare(handle);
    } else if (rc < 0) {
      fprintf(stderr,
              "error from read: %s\n",
              snd_strerror(rc));
    } else if (rc != (int)num_frames) {
      fprintf(stderr, "short read, read %d frames\n", rc);
    }
    bytes_read = rc * frame_size_bytes;

    return bytes_read;
}
/*-------------------------------------------------------------*/
static int _snd_pcm_getInternalBlockSizeInBytes(SndDevice_t *psd)
{   return psd->buffer_size_frames * psd->frame_size_bytes;
}
/*-------------------------------------------------------------*/


/***************************************************************/
/*************** end of implementation of private functions ****/
/***************************************************************/



/***************************************************************/
/************  exported module functions ***********************/
/***************************************************************/


/*---------------- public, exported functions --------------------*/

SndDevice_t *sndOpen(int rw_mode, int mono_stereo)
{
    SndDevice_t *psd;
    int berror=0;

    psd=(SndDevice_t*)malloc(sizeof(SndDevice_t));
    _MyAssert(psd!=NULL,"sndOpen:malloc() crashed");

    psd->nChannels           = mono_stereo;
    psd->pcm_handle_capture  = NULL;
    psd->pcm_handle_playback = NULL;
    psd->rw_mode             = rw_mode;

    switch(rw_mode)
    {   case SND_READ_ONLY:
            if(0!= _snd_pcm_open_capture(psd) )
            {   berror = 1;
            }
            break;
        case SND_WRITE_ONLY:
            if(0!= _snd_pcm_open_playback(psd) )
            {   berror = 1;
            }
            break;
        case SND_READ_WRITE:
            if((0!= _snd_pcm_open_capture(psd) ) ||
               (0!= _snd_pcm_open_playback(psd)) )
            {   berror = 1;
            }
            break;
        default:
            fprintf(stderr,"snd_open: use either SND_READ_ONLY,"
                    " SND_WRITE_ONLY or SND_READ_WRITE");
            berror = 1;
            break;
    } // switch

    /* set hardware parameters of used devices (handle !=NULL)*/
    if((!berror) && (psd->pcm_handle_capture!=NULL))
    {   if(0!=_snd_pcm_set_parameters(psd, psd->pcm_handle_capture, mono_stereo, SOUNDCARD_SAMPLE_RATE))
        {   berror = 1; }
    }

    if((!berror) && (psd->pcm_handle_playback!=NULL))
    {   if(0!=_snd_pcm_set_parameters(psd, psd->pcm_handle_playback, mono_stereo, SOUNDCARD_SAMPLE_RATE))
        {   berror = 1; }
    }

    if(berror)
    {   perror("sndOpen has crashed!");
        free(psd);
        return NULL;
    }
    else return psd;
}


/*------------------------------------------------------------------*/

SndDevice_t *sndClose(SndDevice_t *psd)
{

    /* close the used devices (handle !=NULL)*/
    if(psd->pcm_handle_capture != NULL)
    {   snd_pcm_drain(psd->pcm_handle_capture);
        snd_pcm_close(psd->pcm_handle_capture);
    }
    if(psd->pcm_handle_playback != NULL)
    {   snd_pcm_drain(psd->pcm_handle_playback);
        snd_pcm_close(psd->pcm_handle_playback);
    }
    /* free memory, if allocated */
    if(psd != NULL)
    {   free(psd);
    }

    return NULL;
}

/*------------------------------------------------------------------*/



int sndRead(SndDevice_t *psd, short *buf, int buf_elements)
{
    int rc;

    _MyAssert(psd->pcm_handle_capture != NULL, "capture device not initialized!");

    rc = _snd_pcm_read_bytes(psd->pcm_handle_capture,
                            (char*) buf,
                            buf_elements*sizeof(short),
                            psd->frame_size_bytes);
    if(rc<0) return -1;

    return (rc / sizeof(short));
}


/*------------------------------------------------------------------*/



int sndWrite(SndDevice_t *psd, short *buf, int buf_elements)
{
    int rc;

    _MyAssert(psd->pcm_handle_playback != NULL, "playback device not initialized!");

    rc = _snd_pcm_write_bytes(psd->pcm_handle_playback,
                             (char *) buf,
                              buf_elements*sizeof(short),
                              psd->frame_size_bytes);
    if(rc<0) return -1;

    return (rc / sizeof(short));
}


/*------------------------------------------------------------------*/



/***************************************************************/
/***************************************************************/
/***************************************************************/
/***************************************************************/






/*----------------------------------------------------------------*/



/***************************************************************
* Linux ALSA Version of int sndWAVPlaySound(char *Filename)
****************************************************************/
int sndWAVPlaySound(char *Filename)
{
    FILE *fp=NULL;
    SndDevice_t *psd=NULL;
    char *buf=NULL;
    int buf_size;
    sndWaveHeader_t wh;
    int ok=1;
    int is16BitMono, is16BitStereo;
    int i;


    if((fp=fopen(Filename, "rb")) == NULL)
    {   _errMsg("sndWAVPlaySound: cannot open file...");
        return -1;
    }
    if(0!=sndWAVReadFileHeader(fp, &wh))
    {   _errMsg("sndWAVPlaySound: cannot read WAV file header");
        fclose(fp);
        return -1;
    }
    /* can we understand this format? */
    DebugCode(printf("Info: format:%d\n",wh.format););
    DebugCode(printf("Info: channels:%d\n",wh.nChannels););
    DebugCode(printf("Info: nBytesPerSample:%d\n",wh.nBytesPerSample););
    DebugCode(printf("Info: nSamplesPerSec:%lu\n",wh.nSamplesPerSec););
    ok = 1;
    if(wh.format != 1)
    {   ok = 0; /* only PCM */
        _errMsg("sndWAVPlaySound: file is not in PCM format");
    }

    if((wh.nChannels==1) && (wh.nBytesPerSample==2))
    {   is16BitMono = 1; }
    else
    {   is16BitMono=0; }

    if((wh.nChannels==2) && (wh.nBytesPerSample==4))
    {   is16BitStereo = 1; }
    else
    {   is16BitStereo=0; }

    if((!is16BitMono) && (!is16BitStereo))
    {   _errMsg("sndWAVPlaySound: unkown format...");
        _errMsg("sndWAVPlaySound: neither 16bit mono nor 16bit stereo ...");
        ok = 0;
        fclose(fp);
        return -1;
    }
    /* up to now, we are sure to have 16bit mono or stereo */
    /* open sound card an set parameters accordingly */
    DebugCode(printf("Info: trying to open sound card...\n"););

    psd=(SndDevice_t*)malloc(sizeof(SndDevice_t));
    _MyAssert(psd!=NULL,"sndOpen:malloc() crashed");

    if(is16BitMono) psd->nChannels = 1;
    else            psd->nChannels = 2;
    psd->pcm_handle_capture  = NULL;
    psd->pcm_handle_playback = NULL;
    psd->rw_mode             = SND_WRITE_ONLY;

    if(0!= _snd_pcm_open_playback(psd) )
    {   ok = 0;
    }
    DebugCode(printf("Info: device opened for playback, configure...\n"););

    if((ok) && (psd->pcm_handle_playback!=NULL))
    {   if(0!=_snd_pcm_set_parameters(psd, psd->pcm_handle_playback,
              psd->nChannels, wh.nSamplesPerSec))
        {   ok=0; }
    }
    DebugCode(printf("Info: device configured\n"););

    if(!ok)
    {   perror("sndWAVPlaySound: sndOpen has crashed!");
        sndClose(psd);
        fclose(fp);
        return -1;
    }

    /* ok, sound device seems open and configured, now allocate a buffer */
    /* with the size of the internal sound device buffer */
    buf_size = _snd_pcm_getInternalBlockSizeInBytes(psd);
    DebugCode(printf("Info: buf_size:%d\n",buf_size););

    if(NULL == (buf=(char*)malloc(buf_size)))
    {   _errMsg("sndWAVPlaySound: cannot malloc()");
        sndClose(psd);
        fclose(fp);
        return -1;
    }
    memset(buf,0,buf_size); /* erase buffer */

    i=0;
    /* lesen aus der Datei und schreiben auf die Soundkarte */
    while(ok && (buf_size == fread(buf,sizeof(char),buf_size,fp)))
    {   if(buf_size != _snd_pcm_write_bytes(psd->pcm_handle_playback,
                                           (char *) buf, buf_size,
                                            psd->frame_size_bytes))
        {   ok=0;
            _errMsg("sndWAVPlaySound: cannot write to sound device!");
        }
        i++;
    }
    DebugCode(printf("Info: i:%d read %d bytes\n",i,i*buf_size););
    /* clean up and finish */
    free(buf);
    sndClose(psd);
    fclose(fp);
    DebugCode(printf("Info: done...\n"););

    return 0;
}


#endif // LINUX_ALSA
/*******************************************************************
*              End of Linux ALSA implementation
******************************************************************/

/**********************************************************************
End of Linux implementation:
**********************************************************************/

#endif  /* (PLATFORM==OS_LINUX) */
