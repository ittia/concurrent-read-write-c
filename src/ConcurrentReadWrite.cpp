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

#include "adi_initialize.h"
#include "ConcurrentReadWrite.h"
#include <ucos_ii.h>
#include <stdio.h>

#include "ittiadb_init.h"
#include "DatabaseFunctions.h"

OS_FLAG_GRP *g_DatabaseReadyFlag;

int main(void)
{
    TaskInformation WriterInfo;
    static TaskGlobals WriterGlobals;

    TaskInformation ReadersInfo[NUM_OF_READERS];
    static TaskGlobals ReadersGlobals[NUM_OF_READERS];

    INT16U writer_task_options = OS_TASK_OPT_NONE;

    INT8U err;
    int i,j;

    if (adi_initComponents() != 0 && ittiadb_init() != 0) {
        printf("Error initializing components\n");
        exit(1);
    }

    /* Create database writer. */

    snprintf(WriterInfo.TaskName, 30, "Writer");
    WriterInfo.TaskIndex = 0;
    WriterInfo.TaskPrio = TASK_PRIO_BASE;
    err =  OSTaskCreateExt ( ReaderTaskRunFunc,   /* Task function */
                         &WriterInfo,             /* Argument for the task function*/
                         WriterGlobals.TaskStack + TASK_STK_SIZE, /* Pointer to the stack */
                         WriterInfo.TaskPrio,     /* Priority. Only one task per priority allowed*/
                         WriterInfo.TaskPrio,     /* Task id. Must be the same as the priority */
                         WriterGlobals.TaskStack, /* Stack base*/
                         TASK_STK_SIZE,           /* Stack size */
                         NULL,                    /* pointer to a TCB extension */
                         writer_task_options      /* Task specific options */
                         );

    CHECK_ERROR(err, "Error creating Writer Task");

    /* Create database readers. */
    for (i = 0; i < NUM_OF_READERS; ++i)
    {
        ReadersInfo[i].TaskIndex = i;
        ReadersInfo[i].TaskPrio = TASK_PRIO_BASE + 1 + i; /* each task must have a different priority */
        CreateReaderTask(&ReadersInfo[i], &ReadersGlobals[i]);
    }

    /* Create an event group to indicate if the database is ready to access. */
    g_DatabaseReadyFlag = OSFlagCreate(0, &err);
    CHECK_ERROR(err, "Error creating Event Flag");

    OSStart();
    CHECK_ERROR(1, "Error starting the RTOS; this should be unreachable.");

    return 0;
}

void CreateReaderTask(TaskInformation *pInTaskInfo, TaskGlobals *pInTaskGlobals) {
    INT8U err;
    snprintf(pInTaskInfo->TaskName, 30, "Reader%d",pInTaskInfo->TaskIndex);

    INT16U task_options = OS_TASK_OPT_NONE;
    err =  OSTaskCreateExt ( ReaderTaskRunFunc,     /* Task function */
                         pInTaskInfo,               /* Argument for the task function*/
                         pInTaskGlobals->TaskStack + TASK_STK_SIZE, /* Pointer to the stack */
                         pInTaskInfo->TaskPrio,     /* Priority. Only one task per priority allowed*/
                         pInTaskInfo->TaskPrio,     /* Task id. Must be the same as the priority */
                         pInTaskGlobals->TaskStack, /* Stack base*/
                         TASK_STK_SIZE,             /* Stack size */
                         NULL,                      /* pointer to a TCB extension */
                         task_options               /* Task specific options */
                         );

    CHECK_ERROR(err, "Error creating Reader Task");

    OSTaskNameSet (pInTaskInfo->TaskPrio, (INT8U*) pInTaskInfo->TaskName, &err);
    CHECK_ERROR(err, "Error naming Reader Task");
}

void WriterTaskRunFunc(void* arg)
{
    TaskInformation *info = (TaskInformation*) arg;
    INT8U err;

    PrepareDatabase(info);

    OSFlagPost(g_DatabaseReadyFlag, (OS_FLAGS)1, OS_FLAG_SET, &err);
    CHECK_ERROR(err, "Error posting flag");

    WriteToDatabase(info);
    CloseDatabaseConnection(info);

    printf("Writer task finished.\n");
    OSTaskSuspend(OS_PRIO_SELF);
}

void ReaderTaskRunFunc(void* arg)
{
    TaskInformation *info = (TaskInformation*) arg;
    INT8U err;

    if (OSFlagPend(g_DatabaseReadyFlag, (OS_FLAGS)1, OS_FLAG_WAIT_SET_ALL, 0u, &err) != 0u)
    {
        OpenDatabaseConnection(info);
        ReadFromDatabase(info);
        CloseDatabaseConnection(info);

        printf("Reader task finished.\n");
    }

    CHECK_ERROR(err, "Error pending on flag");

    OSTaskSuspend(OS_PRIO_SELF);
}
