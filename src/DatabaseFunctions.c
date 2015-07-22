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

#include <stdio.h>

#include "ConcurrentReadWrite.h"


#define DATABASE_FILE "concurrent_read_write.ittiadb"

static const int ROW_COUNT = 100;

#define GENERATE_ID(i) ((i) * 1103515245 + 12345)


/* Database schema: table, field, and index definitions. */

#define T_ID 0
#define T_N 1
#define T_S 2

static db_fielddef_t table_t_fields[] = {
    { T_ID, "ID", DB_COLTYPE_UINT32,    0, 0, DB_NOT_NULL, NULL, 0 },
    { T_N,  "N",  DB_COLTYPE_SINT32,    0, 0, DB_NULLABLE, NULL, 0 },
    { T_S,  "S",  DB_COLTYPE_UTF8STR, 100, 0, DB_NULLABLE, NULL, 0 }
};
static db_tabledef_t table_t = {
    DB_ALLOC_INITIALIZER(),
    DB_TABLETYPE_DEFAULT,
    "T",
    DB_ARRAY_DIM(table_t_fields),
    table_t_fields,
    0,
    NULL
};

static db_indexfield_t index_t_id_fields[] = {
    { T_ID, 0 }
};
static db_indexdef_t index_t_id = {
    DB_ALLOC_INITIALIZER(),
    DB_INDEXTYPE_DEFAULT,
    "ID",
    DB_MULTISET_INDEX,
    DB_ARRAY_DIM(index_t_id_fields),
    index_t_id_fields,
};

/* Main program. */

void PrepareDatabase(TaskInformation * pTaskInfo)
{
    db_t database;

    /* Create the database, table, and index. */

    database = db_create_file_storage(DATABASE_FILE, NULL);
    if (database == NULL)
    {
        printf("Error opening database: %d", (int) get_db_error());
        return;
    }

    db_create_table(database, table_t.table_name, &table_t, 0);
    db_create_index(database, table_t.table_name, index_t_id.index_name, &index_t_id);

    pTaskInfo->TaskDbHandle = database;
}

void OpenDatabaseConnection(TaskInformation * pTaskInfo)
{
    db_t database;

    database = db_open_file_storage(DATABASE_FILE, NULL);
    if (database == NULL)
    {
        printf("Error opening database: %d", (int) get_db_error());
        return;
    }

    pTaskInfo->TaskDbHandle = database;
}

void CloseDatabaseConnection(TaskInformation * pTaskInfo)
{
    db_t database = (db_t)pTaskInfo->TaskDbHandle;

    db_shutdown(database, 0, NULL);
}

void WriteToDatabase(TaskInformation * pTaskInfo)
{
    db_t database = (db_t)pTaskInfo->TaskDbHandle;

    db_cursor_t t_cursor;
    db_row_t t_row;
    int i;

    /* Bind database fields to local variables. */

    uint32_t id;
    int32_t n;
    char s[100];

    db_bind_t t_binds[] = {
        DB_BIND_VAR(T_ID, DB_VARTYPE_UINT32,  id),
        DB_BIND_VAR(T_N,  DB_VARTYPE_SINT32,  n),
        DB_BIND_STR(T_S,  DB_VARTYPE_UTF8STR, s)
    };

    t_row = db_alloc_row(t_binds, DB_ARRAY_DIM(t_binds));

    /* Configure transactions. */
    db_set_tx_default(database, DB_GROUP_COMPLETION | DB_READ_COMMITTED);

    /* Open unordered table cursor. */
    t_cursor = db_open_table_cursor(database, table_t.table_name, NULL);

    /* Insert rows. */

    printf("Insert %d rows: ", ROW_COUNT);
    fflush(stdout);

    for(i = 1; i <= ROW_COUNT; i++) {
        db_begin_tx(database, 0);

        id = GENERATE_ID(i);
        n = ROW_COUNT / 2 - i;
        sprintf(s, "%d", i);
        db_insert(t_cursor, t_row, NULL, 0);

        db_commit_tx(database, 0);
    }

    db_close_cursor(t_cursor);

    db_free_row(t_row);
}

void ReadFromDatabase(TaskInformation * pTaskInfo)
{
    db_t database = (db_t)pTaskInfo->TaskDbHandle;

    db_cursor_t t_ordered_cursor;
    db_row_t t_row;
    db_table_cursor_t ordered_cursor_def;
    int i;

    /* Bind database fields to local variables. */

    uint32_t id;
    int32_t n;
    char s[100];

    db_bind_t t_binds[] = {
        DB_BIND_VAR(T_ID, DB_VARTYPE_UINT32,  id),
        DB_BIND_VAR(T_N,  DB_VARTYPE_SINT32,  n),
        DB_BIND_STR(T_S,  DB_VARTYPE_UTF8STR, s)
    };

    t_row = db_alloc_row(t_binds, DB_ARRAY_DIM(t_binds));

    /* Configure transactions. */
    db_set_tx_default(database, DB_GROUP_COMPLETION | DB_READ_COMMITTED);

    /* Open table cursor ordered by the fields in the ID index. */
    db_table_cursor_init(&ordered_cursor_def);
    ordered_cursor_def.index = index_t_id.index_name;
    ordered_cursor_def.flags = DB_CAN_MODIFY;
    t_ordered_cursor = db_open_table_cursor(database, table_t.table_name, &ordered_cursor_def);
    db_table_cursor_destroy(&ordered_cursor_def);

    printf("Index seek %d rows: ", ROW_COUNT);
    fflush(stdout);

    for(i = 1; i <= ROW_COUNT; i++) {
        id = GENERATE_ID(i);
        db_begin_tx(database, 0);
        db_seek(t_ordered_cursor, DB_SEEK_EQUAL, t_row, NULL, 1);
        db_fetch(t_ordered_cursor, t_row, NULL);
        db_commit_tx(database, 0);
    }

    db_close_cursor(t_ordered_cursor);

    db_free_row(t_row);
}
