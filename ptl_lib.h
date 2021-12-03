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
 *   @file                        ptl_lib.h
 *   @par Project:    Portable Thread Library
 *   @par Module:     Select Operation System (Linux / Windows)
 *
 *   @brief  Defines, types and function prototypes of module ptl_lib.c
 *
 *
 *   @par Author:     Ernst Forgber
 *   @par Department: E14
 *   @par Company:    Fachhochschule Hannover - University of Applied Sciences and Arts
 *
 *   @par History       :
 *   @verbatim
 *  Ver  Date        Author            Change Description
 *  0.0  11.02.2005   E. Forgber        - First Version
 *  0.1  20.02.2007  E. Forgber,Felix Bartelt        - Second Version with Borland Compiler 5.5 testet
 *  0.2  13.12.2007  E. Forgber - unblocking of queues for thread termination implemented
 *
 *   @endverbatim
 *
 ***********************************************************************/




#ifndef _ptl_lib_h_
#define _ptl_lib_h_
/*------------------------------------------*/

#include "ptl_codecontrol.h"

#if (PLATFORM==OS_LINUX)
  #include <pthread.h>    /* POSIX threads   */
  #include <semaphore.h>  /* POSIX semphores */
#endif

#if (PLATFORM==OS_MS_WINDOWS)
  #include <windows.h>
  #include <limits.h>
#endif


/***********************************************
 * thread data structure :
 ***********************************************/
#if (PLATFORM==OS_LINUX)
  typedef pthread_t PTL_thread_t;  /*!< POSIX thread ID */
#endif
#if (PLATFORM==OS_MS_WINDOWS)
  typedef struct {
    HANDLE hnd;  /*!< Windows thread handle */
    DWORD  id;   /*!< Windows thread ID number */
  } PTL_thread_t;
#endif


/************************************************
 * macro for thread function definition
 ************************************************/
#if (PLATFORM==OS_LINUX)
  #define PTL_THREAD_RET_TYPE  void *    /*!< internal */  
#endif
#if (PLATFORM==OS_MS_WINDOWS)
  #define PTL_THREAD_RET_TYPE  DWORD WINAPI    /*!< internal */
#endif



/***********************************************
 * semaphore data structure :
 ***********************************************/
#if (PLATFORM==OS_LINUX)
  typedef sem_t PTL_sem_t;  /*!< semaphore type */
#endif
#if (PLATFORM==OS_MS_WINDOWS)
  typedef HANDLE PTL_sem_t; /*!< semaphore type */  
#endif

/***********************************************
 * queue data structure :
 ***********************************************/
typedef struct {
        PTL_sem_t emptySlotsSema;  /*!< counts number of free slots */
        PTL_sem_t filledSlotsSema; /*!< counts number of used slots */
        PTL_sem_t criticalSectionSema; /*!< protects queue struct */
        char *buffer;  /*!< queue data buffer */
        unsigned int maxSlots;     /*!< total number of slots in queue */
        unsigned int slotSize;     /*!< size of slot in byte */
        unsigned int rIndex;      /*!< read index in buffer */
        unsigned int wIndex;      /*!< write index in buffer */
        unsigned int usedSlots;   /*!< number of used slots */
        unsigned int isInitialized; /*!< 0 if not initinialized */
        unsigned int isUnblockedForTermination; /*!< !=0 to unblock waiting threads, used for thread termination*/ 
} PTL_queue_t;


/***********************************************
 * exported functions
 ***********************************************/
/* thread management */

int PTL_CreateThread(PTL_thread_t *thread,
                     PTL_THREAD_RET_TYPE (*start_routine)(void *),
                     void * arg);   
                                                         
int PTL_Sleep(double seconds);
int PTL_TerminateThread(PTL_thread_t thread);

/* counting semaphores */
int PTL_SemCreate(PTL_sem_t *s, unsigned int value);
int PTL_SemDestroy(PTL_sem_t *s);
int PTL_SemWait(PTL_sem_t *s);
int PTL_SemSignal(PTL_sem_t *s);

/* message queues */
int PTL_QueueCreate(PTL_queue_t *q, unsigned int slotSize, unsigned int nSlots);
int PTL_QueueDestroy(PTL_queue_t *q);
int PTL_QueueIsEmpty(PTL_queue_t *q);
int PTL_QueueIsFull(PTL_queue_t *q);
int PTL_QueueWrite(PTL_queue_t *q, unsigned int nSlots, char *data);
int PTL_QueueRead(PTL_queue_t *q,  unsigned int nSlots, char *data);
int PTL_QueueGetUsedSlots(PTL_queue_t *q);
int PTL_QueueGetMaxSlots(PTL_queue_t *q);
int PTL_QueueGetSlotSize(PTL_queue_t *q);
int PTL_QueueUnblockThreadsForTermination(PTL_queue_t *q);


/*------------------------------------------*/
#endif
