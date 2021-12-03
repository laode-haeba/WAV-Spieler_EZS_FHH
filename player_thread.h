/* player_thread.h */

#ifndef _player_thread_h_
#define _player_thread_h_

#include "ptl_lib.h"
#include "snd_lib.h" 
#include "globals.h"



/* Prototyp der Threadfundktion */
PTL_THREAD_RET_TYPE WavPlayerThreadFunc(void* pt);



#endif
