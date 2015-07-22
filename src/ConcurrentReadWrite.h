/**************************************************************************/
/*                                                                        */
/*      Copyright (c) 2005-2015 by ITTIA L.L.C. All rights reserved.      */
/*                                                                        */
/*  This software is copyrighted by and is the sole property of ITTIA     */
/*  L.L.C.  All rights, title, ownership, or other interests in the       */
/*  software remain the property of ITTIA L.L.C.  This software may only  */
/*  be used in accordance with the corresponding license agreement.  Any  */
/*  unauthorized use, duplication, transmission, distribution, or         */
/*  disclosure of this software is expressly forbidden.                   */
/*                                                                        */
/*  This Copyright notice may not be removed or modified without prior    */
/*  written consent of ITTIA L.L.C.                                       */
/*                                                                        */
/*  ITTIA L.L.C. reserves the right to modify this software without       */
/*  notice.                                                               */
/*                                                                        */
/*  info@ittia.com                                                        */
/*  http://www.ittia.com                                                  */
/*                                                                        */
/*                                                                        */
/**************************************************************************/

#ifndef CONCURRENTREADWRITE_H
#define CONCURRENTREADWRITE_H

#include <ucos_ii.h>

/* This macro indicates whether error checking should be done for all the
 * RTOS API calls
 */
#define API_RETURN_VALUE_CHECK 1

#if (API_RETURN_VALUE_CHECK)
#define CHECK_ERROR(result, message) \
do {\
    if (OS_ERR_NONE != (result))\
    {\
        printf("%s\n",(message));\
        exit(1);\
    }\
} while (0)

#else
#define CHECK_ERROR(result, message)
#endif

#define NUM_OF_READERS 5

#define TASK_STK_SIZE 0x400u
#define TASK_PRIO_BASE (6u)


typedef struct TaskInformationStruct
{
    char TaskName[30];
    INT8U TaskPrio;
    int  TaskIndex;
    void * TaskDbHandle;
} TaskInformation;

typedef struct
{
    CPU_STK TaskStack[TASK_STK_SIZE];
} TaskGlobals;


void CreateReaderTask(TaskInformation *pInTaskInfo, TaskGlobals *pInTaskGlobals);
void WriterTaskRunFunc(void* arg);
void ReaderTaskRunFunc(void* arg);

#endif /* CONCURRENTREADWRITE_H */
