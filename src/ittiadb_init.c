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

#include <ittia/db.h>
#include <stdlib.h>
#include <stdio.h>

int ittiadb_init()
{
    int exit_code;
    int status;

    /* Initialize ITTIA DB SQL library. */
    status = db_init_ex(DB_API_VER, NULL);

    if (DB_SUCCESS(status)) {
        exit_code = EXIT_SUCCESS;
    }
    else {
        printf("ITTIA DB SQL init error: %d\n", status);
        exit_code = EXIT_FAILURE;
    }

    return exit_code;
}
