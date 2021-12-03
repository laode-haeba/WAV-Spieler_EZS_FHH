/*!
 ********************************************************************
   @file            snd_codecontrol.h
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

#ifndef _snd_codecontrol_h_
#define _snd_codecontrol_h_

/********** don't edit this *******/
#ifndef OS_MS_WINDOWS
  #define OS_MS_WINDOWS 1
#endif
#ifndef OS_LINUX
  #define OS_LINUX      2
#endif

/* for Linux, you may call gcc with option -DLINUX to
   define symbol LINUX and thus switch platforms without
   having to edit this file
*/
#ifdef LINUX
  #define PLATFORM  OS_LINUX
  /*-------- Linux only: select either OSS or ALSA -------*/
  #define LINUX_OSS 0    /* !=0 for Linux OSS support */
  #define LINUX_ALSA 1   /* !=0 for Linux ALSA support */
/*--------End of Linux only: select either OSS or ALSA -------*/
#endif

/********* end of: "don't edit this" *******/

/********************************************************/
/* select the platform snd_lib will be running on:               */
/* un-commend ONE of the following define instructions     */
/********************************************************/
#ifndef PLATFORM
  #define PLATFORM  OS_MS_WINDOWS
  /* #define PLATFORM  OS_LINUX */
#endif





#endif
