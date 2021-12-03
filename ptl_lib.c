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
 *   @file                        ptl_lib.c
 *   @par Project:    Portable Thread Library
 *   @par Module:     Select Operation System (Linux / Windows)
 *
 *   @brief  This file contains definitions of exported module functions.
 *           PTL (portable thread lib) will run on Windows and Linux.
 *           Define the PLATFORM symbol in ptl_codecontrol.h to either
 *           OS_LINUX or OS_MS_WINDOWS, re-compile and re-link the module.
 *
 *
 *   @par Author:     Ernst Forgber
 *   @par Department: E14
 *   @par Company: Fachhochschule Hannover - University of Applied Sciences and Arts
 *
 ***********************************************************************
 *
 *   @par History       :
 *   @verbatim
 *  Ver  Date        Author            Change Description
 *  0.0  11.02.2005  E. Forgber        - First Version
 *  0.1  20.02.2007  E. Forgber,Felix Bartelt        -  Second Version with Borland Compiler 5.5 testet
 *  0.2  13.12.2007  E. Forgber - unblocking of queues for thread termination implemented
 *   @endverbatim
 *
 ***********************************************************************/

/*! \mainpage PTL - A Simple Portable Thread Libray
 *
 * \section intro Introduction
 *
 * This module may be used to develop multi-threaded programs for Windows and
 * Linux. The operation system has to be selected in file ptl_codecontrol.h,
 * the module will have to be re-complied and linked to the application.
 *
 * \section install Usage
 *
 * In order to use the module, the headers ptl_codecontrol.h, ptl_lib.h and the
 * source ptl_lib.c are needed. The application has to include ptl_lib.h,
 * ptl_lib.c has to be compiled and linked to the application.
 *
 * NOTE: Linux users will have to link the library 'libpthread.lib' to the
 * programm, i.e. in VIDE the 'libpthread' will have to be added to the
 * library list. The linker call will then have an option '-lpthread'.
 *
 * Exported functions are documented in file ptl_lib.c, see its documentations for
 * details.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ptl_lib.h"

/* OS specific includes */
#if (PLATFORM==OS_LINUX)
  #include <errno.h>
  #include <time.h>
#endif
#if (PLATFORM==OS_MS_WINDOWS)
#endif



#define _MSG_ENABLE_ 1   /*!< 0 for no error messages */

static void _errMsg(const char *pt)
{
  #if (_MSG_ENABLE_ != 0)
    fprintf(stderr, "PTL-Message: %s\n", pt);
  #endif
}

/*!
 **********************************************************************
 * @par Exported Function:
 *   PTL_CreateThread
 *
 * @par Description:
 *   This function creates and immediatly starts a new thread.
 *
 * @see
 * @arg  PTL_TermiateThread()
 *
 *
 * @param  thread         - IN/OUT, pointer to thread variable
 * @param  start_routine  - IN, address of thread function
 * @param  arg            - IN, pointer that is passed to the thread
 *                          function as an argument
 *
 * @retval 0               - no error
 * @retval negative        - an error occured
 *
 * @par Example :
 *    The thread function MyThreadFunc() may be defined
 *    and started as a thread like this:
 * @verbatim
PTL_THREAD_RET_TYPE MyThreadFunc(void* pt)
{ int i;
  for(i=0; i<100; i++)
  { putc('x');
    PTL_Sleep(0.45);
  }
  return (PTL_THREAD_RET_TYPE) 0;
}

int main(void)
{ PTL_thread_t id1;

  if(0!=PTL_CreateThread(&id1, MyThreadFunc, NULL))
    puts("error starting thread");
  PTL_Sleep(10.5);
  return 0;
}
  @endverbatim
 ************************************************************************/

int PTL_CreateThread(PTL_thread_t *thread,
                     PTL_THREAD_RET_TYPE(*start_routine)(void * ), void * arg)
{
  int retval=0;

  #if (PLATFORM==OS_LINUX)
    /******************************************************
     * create the thread in 'detached' mode, i.e. it is not
     *  'joinable', but will automatically clean up after
     *  termination.
     ******************************************************/
    { pthread_attr_t attr;  /* thread attributes stucture */

      pthread_attr_init(&attr);  /* set to defaults */
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      retval = pthread_create(thread, &attr, start_routine, arg);
      pthread_attr_destroy(&attr);
    }
  #endif


  #if (PLATFORM==OS_MS_WINDOWS)
    thread->hnd = CreateThread(NULL, 0, start_routine, arg, 0, &(thread->id));
        if(NULL == thread->hnd)
      retval=-1;
  #endif


  return retval;
}

/*------------------------------------------------*/


/*!
 **********************************************************************
 * @par Exported Function:
 *   PTL_Sleep
 *
 * @par Description:
 *   This function blockes the calling thread for a period of time
 *
 *
 * @param  seconds        - IN, sleeping interval in seconds
 *
 * @retval 0               - no error
 * @retval negative        - an error occured
 *
 * @par Example :
 * @see
 *    @arg PTL_CreateThread
 ************************************************************************/
int PTL_Sleep(double seconds)
{
  #if (PLATFORM==OS_MS_WINDOWS)
    Sleep(seconds*1e3); /* Windows sleeps in milliseconds */
    return 0;
  #endif

  #if (PLATFORM==OS_LINUX)
    /***********************************************************************
     * Code listing from "Advanced Linux Programming," by CodeSourcery LLC  *
     * Copyright (C) 2001 by New Riders Publishing, pp 182, listing 8.8     *
     ***********************************************************************/
    struct timespec tv;
    /* Construct the timespec from the number of whole seconds...  */
    tv.tv_sec = (time_t) seconds;
    /* ... and the remainder in nanoseconds.  */
    tv.tv_nsec = (long) ((seconds - tv.tv_sec) * 1e+9);

    pthread_testcancel();  /* terminate thread if it has been cancelled */

    while (1)
    {
      /* Sleep for the time specified in tv.  If interrupted by a
         signal, place the remaining time left to sleep back into tv.  */
      int rval = nanosleep (&tv, &tv);
      if (rval == 0)
        /* Completed the entire sleep time; all done.  */
        return 0;
      else if (errno == EINTR)
        /* Interruped by a signal.  Try again.  */
        continue;
      else
        /* Some other error; bail out.  */
        return rval;
    }
    return 0;
  #endif


}
/*------------------------------------------------*/


/*!
 **********************************************************************
 * @par Exported Function:
 *   PTL_TerminateThread
 *
 * @par Description:
 *   This function terminates a thread. A thread will either terminate when
 *   it reaches the end of its thread function, or when it is killed using
 *   PTL_TerminateThread
 *
 * @see
 * @arg  PTL_CreateThread()
 *
 *
 * @param  thread         - IN, thread variable
 *
 * @retval 0               - no error
 * @retval negative        - an error occured
 *
 * @par Example :
 *    The thread function MyThreadFunc() may be defined,
 *    started and - after awhile - killed like this:
 * @verbatim
PTL_THREAD_RET_TYPE MyThreadFunc(void* pt)
{ int i;
  for(i=0; i<100; i++)
  { putc('x');
    PTL_Sleep(0.45);
  }
  return (PTL_THREAD_RET_TYPE) 0;
}

int main(void)
{ PTL_thread_t id1;

  if(0!=PTL_CreateThread(&id1, MyThreadFunc, NULL))
    puts("error starting thread");
  PTL_Sleep(2);
  PTL_TerminateThread(id1);
  PTL_Sleep(2);
  return 0;
}
  @endverbatim
 ************************************************************************/
int PTL_TerminateThread(PTL_thread_t thread)
{
  #if (PLATFORM==OS_MS_WINDOWS)
    TerminateThread(thread.hnd, 0);
    return 0;
  #endif

  #if (PLATFORM==OS_LINUX)
    return pthread_cancel(thread);
  #endif
}
/*------------------------------------------------*/


/*************************************************************************
 * counting semaphore implementation
 *************************************************************************/


/*!
 **********************************************************************
 * @par Exported Function:
 *   PTL_SemCreate
 *
 * @par Description:
 *   This function creates a counting semaphore. The initial value of the
 *   semaphore is passed as a parameter. Returns 0 for 'ok', -1 on error.
 *
 * @see
 * @arg  PTL_SemDestroy()
 *
 *
 * @param  s               - IN/OUT, pointer to semaphore
 * @param  value           - IN, initial counter value
 *
 * @retval 0               - no error
 * @retval negative        - an error occured
 *
 * @par Example :
 *    A global semaphore may be created, used and destroyed like this:
 * @verbatim
PTL_sem_t sema01;

int main(void)
{ ...

  if(0!= PTL_SemCreate(&sema01, 1))
    puts("error creating semaphore");
  ...
  PTL_SemWait(&sema01);
  ...
  PTL_SemSignal(&sema01);
  ...
  PTL_SemDestroy(&sema01);
  return 0;
}
  @endverbatim
 ************************************************************************/
int PTL_SemCreate(PTL_sem_t *s, unsigned int value)
{ int retval;
  #if (PLATFORM==OS_MS_WINDOWS)
    *s = CreateSemaphore(NULL, value, INT_MAX, NULL);
        if(NULL == *s) retval = -1; /* error */
        else               retval = 0;  /* success */
  #endif

  #if (PLATFORM==OS_LINUX)
    retval = sem_init(s, 0, value);
    if (retval !=0) retval = -1;  /* error */
  #endif
  return retval;
}


/*!
 **********************************************************************
 * @par Exported Function:
 *   PTL_SemDestroy
 *
 * @par Description:
 *   This function destroys a counting semaphore and frees its memory.
 *   Returns 0 for 'ok', -1 on error.
 *
 * @see
 * @arg  PTL_SemCreate()
 *
 *
 * @param  s               - IN, pointer to semaphore
 *
 * @retval 0               - no error
 * @retval negative        - an error occured
 *
 ************************************************************************/
int PTL_SemDestroy(PTL_sem_t *s)
{ int retval;
  #if (PLATFORM==OS_MS_WINDOWS)
    retval=CloseHandle(*s); /* success: !=0 */
        if(0!=retval) retval = 0;  /* ok */
        else          retval = -1; /* error */
  #endif

  #if (PLATFORM==OS_LINUX)
    retval = sem_destroy(s);
    if (retval !=0) retval =  -1;  /* error */
  #endif
  return retval;
}


/*!
 **********************************************************************
 * @par Exported Function:
 *   PTL_SemWait
 *
 * @par Description:
 *   This function waits blocking on a semaphore. It waits - without time limit -
 *   until the semphore counter greater than zero, decrements the counter
 *   by 1 and returns.
 *   Returns 0 for 'ok', -1 on error.
 *
 * @see
 * @arg  PTL_SemSignal()
 *
 *
 * @param  s               - IN, pointer to semaphore
 *
 * @retval 0               - no error
 * @retval negative        - an error occured
 *
 ************************************************************************/
int PTL_SemWait(PTL_sem_t *s)
{ int retval;
  #if (PLATFORM==OS_MS_WINDOWS)
    if(WAIT_FAILED == WaitForSingleObject(*s, INFINITE))
                retval = -1;
    else
                retval = 0;
  #endif

  #if (PLATFORM==OS_LINUX)
    retval = sem_wait(s);
    if (retval !=0) retval = -1;  /* error */
  #endif
  return retval;
}

/*!
 **********************************************************************
 * @par Exported Function:
 *   PTL_SemSignal
 *
 * @par Description:
 *   This function signals a semaphore, i.e it increments the
 *   semaphore counter by 1. It never blocks,
 *   but returns immediately.
 *   Returns 0 for 'ok', -1 on error.
 *
 * @see
 * @arg  PTL_SemWait()
 *
 *
 * @param  s               - IN, pointer to semaphore
 *
 * @retval 0               - no error
 * @retval negative        - an error occured
 *
 ************************************************************************/
int PTL_SemSignal(PTL_sem_t *s)
{ int retval;
  #if (PLATFORM==OS_MS_WINDOWS)
    if(0==ReleaseSemaphore(*s,1,NULL))
                retval = -1;  /* error */
        else
                retval = 0;   /* ok */
  #endif

  #if (PLATFORM==OS_LINUX)
  retval = sem_post(s);
  if (retval !=0) retval = -1;  /* error */
  else            retval =  0;  /* success */
  #endif
  return retval;
}
/*------------------------------------------------*/
/*------------------------------------------------*/
/*------------------------------------------------*/





/*****************************************************************
                      message queues:

                         --------
        SemWait          | 0--- |      SemSignal
    -------------------- |   || | <--------------------
    |                    --------                     |
    |                   emptySlotsSema                |
    |                                                 |
    |                    --------                     |
    |     SemWait        | 0--- |    SemWait          |
    |   ---------------> |   || | <---------------    |
    |   | SemSignal      --------    SemSignal   |    |
    |   |           criticalSectionSema          |    |
   \/  \/                                        \/   |
 |---------|            -----------            |---------|
 | producer|-----------> |  |  |  | ---------->| consumer|
 |---------|            -----------            |---------|
      |                   queue                     /\
      |                                             |
      |                 --------                    |
      |   SemSignal     | 0--- |     SemWait        |
      ----------------> |   || | --------------------
                        --------
                     filledSlotsSema

 *****************************************************************/

/*!
 **********************************************************************
 * @par Exported Function:
 *   PTL_QueueCreate
 *
 * @par Description:
 *   This function creates a queue (FIFO) to pass data among threads.
 *   The queue has nSlots data slots, each slotSize byte wide.
 *
 * @see
 * @arg  PTL_QueueDestroy()
 *
 *
 * @param  q               - IN/OUT, pointer to queue struct
 * @param  slotSize        - IN, size of data slot in byte
 * @param  nSlots          - IN, number of data slots in queue
 *
 * @retval 0               - no error
 * @retval negative        - an error occured
 *
 * @par Example :
 *    A global queue with 5 slots for float variables and a second queue with 10
 *    slots for int arrays with 100 elements each  may be created, used
 *    and destroyed like this:
 * @verbatim
PTL_queue_t q01, q02;

int main(void)
{ float x,y,z[2];
  int a[100], b[100];
  ...
  if(0!= PTL_QueueCreate(&q01, sizeof(float),5))
    puts("error creating queue");
  if(0!= PTL_QueueCreate(&q02, 100*sizeof(int),10))
    puts("error creating queue");
  ...
  if(0!=PTL_QueueWrite(&q01, 1, &x)
    puts("error writing queue");
  if(0!=PTL_QueueWrite(&q01, 1, &y)
    puts("error writing queue");
  if(0!=PTL_QueueWrite(&q02, 1, a)
    puts("error writing queue");
    ...
  if(0!=PTL_QueueRead(&q02, 1, b)
    puts("error reading queue");
  if(0!=PTL_QueueRead(&q01,2, z)
    puts("error reading queue");
  ...
  PTL_QueueDestroy(&q01);
  PTL_QueueDestroy(&q02);
  return 0;
}
  @endverbatim
 ************************************************************************/
int PTL_QueueCreate(PTL_queue_t *q, unsigned int slotSize, unsigned int nSlots)
{
  /* create semaphores */
  if(0!=PTL_SemCreate(&(q->emptySlotsSema), nSlots))
  { _errMsg("PTL_QueueCreate:PTL_QueueCreate");
    return -1;
  }
  if(0!=PTL_SemCreate(&(q->filledSlotsSema), 0))
  { PTL_SemDestroy(&(q->emptySlotsSema));
    _errMsg("PTL_QueueCreate:PTL_QueueCreate");
    return -1;
  }
  if(0!=PTL_SemCreate(&(q->criticalSectionSema), 1))
  { PTL_SemDestroy(&(q->emptySlotsSema));
    PTL_SemDestroy(&(q->filledSlotsSema));
    _errMsg("PTL_QueueCreate:PTL_QueueCreate");
    return -1;
  }
  /* create buffer */
  q->buffer = (char *) malloc(slotSize * nSlots * sizeof(char));
  if(NULL == q->buffer)
  { PTL_SemDestroy(&(q->criticalSectionSema));
    PTL_SemDestroy(&(q->emptySlotsSema));
    PTL_SemDestroy(&(q->filledSlotsSema));
    _errMsg("PTL_QueueCreate:malloc, cannot create buffer");
    return -1;
  }
  q->maxSlots  = nSlots;
  q->slotSize  = slotSize;
  q->rIndex    = 0;         /* empty queue */
  q->wIndex    = 0;
  q->usedSlots = 0;
  q->isInitialized = 1;
  q->isUnblockedForTermination = 0;
  return 0;
}

/*!
 **********************************************************************
 * @par Exported Function:
 *   PTL_QueueDestroy
 *
 * @par Description:
 *   This function destroys a queue (FIFO) and frees all its memory.
 *
 * @see
 * @arg  PTL_QueueCreate()
 *
 *
 * @param  q               - IN/OUT, pointer to queue struct
 *
 * @retval 0               - no error
 * @retval negative        - an error occured
 *
 * @par Example :
 *   @see
 *   @arg PTL_QueueCreate
 *
 ************************************************************************/
int PTL_QueueDestroy(PTL_queue_t *q)
{
  if(q->isInitialized == 0)
  { _errMsg("PTL_QueueDestroy: queue not initialized");
    return -1;
  }
  /* delete buffer */
  PTL_SemWait(&(q->criticalSectionSema));
  if(q->buffer != NULL)
  { free(q->buffer);
    q->buffer = NULL;
  }
  PTL_SemSignal(&(q->criticalSectionSema));
  /* delete semaphores */
  PTL_SemDestroy(&(q->criticalSectionSema));
  PTL_SemDestroy(&(q->emptySlotsSema));
  PTL_SemDestroy(&(q->filledSlotsSema));;
  q->isInitialized = 0;
  return 0;
}



/*!
 **********************************************************************
 * @par Exported Function:
 *   PTL_QueueIsEmpty
 *
 * @par Description:
 *   This function checks whether a queue (FIFO) is empty, i.e. holds no data.
 *
 * @see
 * @arg  PTL_QueueIsFull()
 *
 *
 * @param  q               - IN/OUT, pointer to queue struct
 *
 * @retval 0               - queue is not empty
 * @retval 1               - queue is empty
 * @retval negative        - an error occured
 ************************************************************************/
int PTL_QueueIsEmpty(PTL_queue_t *q)
{ int retval=0;

  if(q->isInitialized == 0)
  { _errMsg("PTL_QueueIsEmpty: queue not initialized");
    return -1;
  }
  PTL_SemWait(&(q->criticalSectionSema));
  if(q->usedSlots == 0)
    retval = 1;
  else
    retval = 0;
  PTL_SemSignal(&(q->criticalSectionSema));
  return retval;
}


/*!
 **********************************************************************
 * @par Exported Function:
 *   PTL_QueueIsFull
 *
 * @par Description:
 *   This function checks whether a queue (FIFO) is full, i.e. all its
 *   data slots are occupied.
 *
 * @see
 * @arg  PTL_QueueIsEmpty()
 *
 *
 * @param  q               - IN/OUT, pointer to queue struct
 *
 * @retval 0               - queue is not full
 * @retval 1               - queue is full
 * @retval negative        - an error occured
 ************************************************************************/
int PTL_QueueIsFull(PTL_queue_t *q)
{ int retval=0;

  if(q->isInitialized == 0)
  { _errMsg("PTL_QueueIsFull: queue not initialized");
    return -1;
  }
  PTL_SemWait(&(q->criticalSectionSema));
  if(q->usedSlots == q->maxSlots)
    retval = 1;
  else
    retval = 0;
  PTL_SemSignal(&(q->criticalSectionSema));
  return retval;
}




/*! \internal
 **********************************************************************
 * @par Exported Function:
 *   _qWriteSingleSlot
 *
 * @par Description:
 *   This function copys one slot of data to the queue. The number of
 *   bytes copied is specified as slotSize in function PTL_QueueCreate().
 *   The calling thread blocks until at least one slot is free.
 *
 * @see
 * @arg  _qReadSingleSlot()
 *
 *
 * @param  q               - IN/OUT, pointer to queue struct
 * @param  data            - IN, pointer to data
 *
 * @retval 0               - ok
 * @retval -1              - an error occured
 * @retval -2              - queue is unblocked for thread termination
 ************************************************************************/
static int _qWriteSingleSlot(PTL_queue_t *q, const char *data)
{ int retval = 0;
  if(q->isInitialized == 0)
  { _errMsg("_qWriteSingleSlot: queue not initialized");
    return -1;
  }
  /* thread blocks until at least one slot is free */
  PTL_SemWait(&(q->emptySlotsSema));
  /* protect queue data: only one thread may enter */
  PTL_SemWait(&(q->criticalSectionSema));
    if(q->isUnblockedForTermination)
    {  _errMsg("_qWriteSingleSlot: queue is unblocked for thread termination");
       retval = -2;
       /* no data is read or written to this queue any more... */
       /* make sure no thread will be blocked waiting for data any more... */
       PTL_SemSignal(&(q->emptySlotsSema));
    }
    else
    {
       memcpy(&(q->buffer[q->wIndex]), data, q->slotSize); /* copy data to queue */
       q->wIndex += q->slotSize;                  /* next slot */
       q->wIndex %= (q->maxSlots * q->slotSize);  /* circular buffer */
       q->usedSlots  += 1;
       PTL_SemSignal(&(q->filledSlotsSema));  /* number of filled slots */
    }   
  PTL_SemSignal(&(q->criticalSectionSema));
  return retval;
}



/*!
 **********************************************************************
 * @par Exported Function:
 *   PTL_QueueWrite
 *
 * @par Description:
 *   This function copys nSlots slots of data to the queue. The number of
 *   bytes copied is nSlots * slotSize, as specified in function PTL_QueueCreate().
 *   The function returns after all nSlots data items are written.
 *
 * @see
 * @arg  PTL_QueueRead()
 *
 *
 * @param  q               - IN/OUT, pointer to queue struct
 * @param  nSlots          - IN, number of data items to be written
 * @param  data            - IN, pointer to data source
 * @retval 0               - ok
 * @retval negative        - an error occured
 ************************************************************************/
int PTL_QueueWrite(PTL_queue_t *q, unsigned int nSlots, char *data)
{ unsigned int i, retval=0;
  char *pt;

  pt = data;
  for(i=0; (i<nSlots) && (retval==0); i++)
  { retval =  _qWriteSingleSlot(q, pt); /* blocking write */
    pt += PTL_QueueGetSlotSize(q);  /* point to next data item */
  }
  if(retval!=0)
  { _errMsg("PTL_QueueWrite: cannot write to queue");
    return -1;
  }
  return 0;
}



/*! \internal
 **********************************************************************
 * @par Exported Function:
 *   _qReadSingleSlot
 *
 * @par Description:
 *   This function reads one slot of data off the queue. The number of
 *   bytes copied is specified as slotSize in function PTL_QueueCreate().
 *   The calling thread blocks until at least one slot of data is available.
 *
 * @see
 * @arg  _qWriteSingleSlot()
 *
 *
 * @param  q               - IN/OUT, pointer to queue struct
 * @param  data            - IN/OUT, pointer to data
 *
 * @retval 0               - ok
 * @retval -1              - an error occured
 * @retval -2              - queue is unblocked for thread termination
 ************************************************************************/
static int _qReadSingleSlot(PTL_queue_t *q, char *data)
{ int retval =0;
  if(q->isInitialized == 0)
  { _errMsg("_qReadSingleSlot: queue not initialized");
    return -1;
  }
  /* thread blocks until at least one filled slot available */
  PTL_SemWait(&(q->filledSlotsSema));
  /* protect queue data: only one thread may enter */
  PTL_SemWait(&(q->criticalSectionSema));
    if(q->isUnblockedForTermination)
    {  /* no data is read or written to this queue any more... */
       _errMsg("_qReadSingleSlot: queue is unblocked for thread termination");
       retval = -2;
       /* make sure no thread will be blocked waiting for data any more... */
       PTL_SemSignal(&(q->filledSlotsSema));
    }
    else
    {
       memcpy(data, &(q->buffer[q->rIndex]), q->slotSize); /* read data from queue */
       q->rIndex += q->slotSize;                  /* next slot to read */
       q->rIndex %= (q->maxSlots * q->slotSize);  /* circular buffer */
       q->usedSlots  -= 1;
       PTL_SemSignal(&(q->emptySlotsSema));  /* incr. number of empty  slots */
    }   
  PTL_SemSignal(&(q->criticalSectionSema));
  return retval;
}




/*!
 **********************************************************************
 * @par Exported Function:
 *   PTL_QueueRead
 *
 * @par Description:
 *   This function reads nSlots slots of data from the queue. The number of
 *   bytes copied is nSlots * slotSize, as specified in function PTL_QueueCreate().
 *   The function returns after all nSlots data items are read.
 *
 * @see
 * @arg  PTL_QueueWrite()
 *
 *
 * @param  q               - IN/OUT, pointer to queue struct
 * @param  nSlots          - IN, number of data items to be read
 * @param  data            - IN, pointer to data destination
 *
 * @retval 0               - ok
 * @retval negative        - an error occured
 ************************************************************************/
int PTL_QueueRead(PTL_queue_t *q,  unsigned int nSlots, char *data)
{ unsigned int i, retval=0;
  char *pt;

  pt = data;
  for(i=0; (i<nSlots) && (retval==0); i++)
  { retval =  _qReadSingleSlot(q, pt); /* blocking write */
    pt += PTL_QueueGetSlotSize(q);  /* point to next data item */
  }
  if(retval!=0)
  { _errMsg("PTL_QueueRead: cannot read from queue");
    return -1;
  }
  return 0;
}



/*!
 **********************************************************************
 * @par Exported Function:
 *   PTL_QueueGetUsedSlots
 *
 * @par Description:
 *   This function returns the number of used slots in the queue.
 *
 * @see
 * @arg  -
 *
 *
 * @param  q               - IN/OUT, pointer to queue struct
 *
 * @retval (positive) number of used slots - ok
 * @retval negative        - an error occured
 ************************************************************************/
int PTL_QueueGetUsedSlots(PTL_queue_t *q)
{ int n;
  if(q->isInitialized == 0)
  { _errMsg("PTL_QueueGetUsedSlots: queue not initialized");
    return -1;
  }
  /* protect queue data: only one thread may enter */
  PTL_SemWait(&(q->criticalSectionSema));
  n =  q->usedSlots;
  PTL_SemSignal(&(q->criticalSectionSema));
  return n;
}


/*!
 **********************************************************************
 * @par Exported Function:
 *   PTL_QueueGetMaxSlots
 *
 * @par Description:
 *   This function returns the number of slots in the queue.
 *
 * @see
 * @arg  -
 *
 *
 * @param  q               - IN/OUT, pointer to queue struct
 *
 * @retval (positive) number of slots - ok
 * @retval negative        - an error occured
 ************************************************************************/
int PTL_QueueGetMaxSlots(PTL_queue_t *q)
{ int n;
  if(q->isInitialized == 0)
  { _errMsg("PTL_QueueGetMaxSlots: queue not initialized");
    return -1;
  }
  /* protect queue data: only one thread may enter */
  PTL_SemWait(&(q->criticalSectionSema));
  n =  q->maxSlots;
  PTL_SemSignal(&(q->criticalSectionSema));
  return n;
}


/*!
 **********************************************************************
 * @par Exported Function:
 *   PTL_QueueGetSlotSize
 *
 * @par Description:
 *   This function returns data slot size of the queue in byte.
 *
 * @see
 * @arg  -
 *
 *
 * @param  q               - IN/OUT, pointer to queue struct
 *
 * @retval (positive) slot size - ok
 * @retval negative        - an error occured
 ************************************************************************/
int PTL_QueueGetSlotSize(PTL_queue_t *q)
{ int n;
  if(q->isInitialized == 0)
  { _errMsg("PTL_QueueGetSlotSitze: queue not initialized");
    return -1;
  }
  /* protect queue data: only one thread may enter */
  PTL_SemWait(&(q->criticalSectionSema));
  n =  q->slotSize;
  PTL_SemSignal(&(q->criticalSectionSema));
  return n;
}




/*!
 **********************************************************************
 * @par Exported Function:
 *   PTL_QueueUnblockThreadsForTermination
 *
 * @par Description:
 *   Oft laesst man Threads in einer Schleife laufen, die man - gesteuert
 *   ueber eine globale Variable - beenden kann. Wenn aber ein Thread an einer 
 *   Queue blockiert, kommt er nicht ans Schleifenende und kann daher auch 
 *   nicht terminieren. Diese Funktion hebt alle Blockaden an der Queue auf.
 *   Alle folgenden Lese- und Schreibzugriffe werden fehlschlagen, aber nicht
 *   blockieren.
 *
 * @see
 * @arg  -
 *
 *
 * @param  q               - IN/OUT, pointer to queue struct
 *
 * @retval 0 - ok
 * @retval negative        - an error occured
 ************************************************************************/
int PTL_QueueUnblockThreadsForTermination(PTL_queue_t *q)
{   
    
    /* protect queue data: only one thread may enter */
    PTL_SemWait(&(q->criticalSectionSema));
    q->isUnblockedForTermination = 1;
    PTL_SemSignal(&(q->criticalSectionSema));
    
    /* unblock pending read: incr. number of filled slots */
    PTL_SemSignal(&(q->filledSlotsSema));  /* number of filled slots */
    
    /* unblock pending write; incr. number of empty slots */
    PTL_SemSignal(&(q->emptySlotsSema));  /* incr. number of empty  slots */
    
    /* Queue will not block any thread any longer, but will not transport 
       data any more, too */
    
    return 0;
}

