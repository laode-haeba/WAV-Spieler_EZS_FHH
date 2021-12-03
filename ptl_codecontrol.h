
/***********************************************************************
 *                                                                     *
 *                  _/_/_/   _/_/_/_/_/  _/                            *
 *                _/     _/     _/      _/                             *
 *               _/_/_/_/      _/      _/                              *
 *              _/            _/      _/                               *
 *             _/            _/      _/_/_/_/                          *
 *                                                                     *
 ***********************************************************************/
/*!
 ***********************************************************************
 *   @file                        ptl_codecontrol.h
 *   @par Project:    Portable Thread Library
 *   @par Module:     Select Operation System (Linux / Windows)
 *
 *   @brief  This file is used to select the platform the PTL
 *           (protable thread lib) will be running on. Define
 *           the PLATFORM symbol to either OS_LINUX or OS_MS_WINDOWS
 *           and re-compile the module.
 *
 *
 *   @par Author:     Ernst Forgber
 *   @par Department: E14
 *   @par Company:    Fachhochschule Hannover - University of Applied Sciences and Arts
 *
 ***********************************************************************/

#ifndef _ptl_codecontrol_h_
#define _ptl_codecontrol_h_

/* don't edit this *******/
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
#endif

/********* end of: "don't edit this" *******/


/***********************************************************************/
/******* select the platform PTL will be running on ********************/
/***********************************************************************/
#ifndef PLATFORM
  #define PLATFORM OS_MS_WINDOWS /*!< select OS_LINUX or OS_MS_WINDOWS */
#endif



#endif
