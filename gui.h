#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef LINUX
    #include "graphapp.h"
#else
    #include "graphapp_3_60_bcc.h"
#endif // LINUX



void gui(int argc, char *argv[]);


#endif // GUI_H_INCLUDED
