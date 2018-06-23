/*
                                                                          _             ____        _____
                                                                         | |           |___ \      | ____|
   ___    __ _   ___   _   _   ______   _ __ ___    _   _   ___    __ _  | |   __   __   __) |     | |__
  / _ \  / _` | / __| | | | | |______| | '_ ` _ \  | | | | / __|  / _` | | |   \ \ / /  |__ <      |___
 |  __/ | (_| | \__ \ | |_| |          | | | | | | | |_| | \__ \ | (_| | | |    \ V /   ___) |  _   ___) |
  \___|  \__,_| |___/  \__, |          |_| |_| |_|  \__, | |___/  \__, | |_|     \_/   |____/  (_) |____/
                        __/ |                        __/ |           | |
                       |___/                        |___/            |_|
Portions of this code are Copyright (C) 2015 ThreeKingz the Original Author
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
Credits:
	Original Author: (creator)
 	* Freddy Borja - ThePhenix AKA ThreeKingz

 	Author: (current developer)
 	* eco1999 AKA Max_Andolini
	Version: 3.6
SQL_ReadRetrievedRows has been changed and now can be used as SQL::ReadRetrievedRows to match the existing style.
*/

#if defined _tksql_included//Avoid including it again!
	#endinput
#endif
#define _tksql_included

#if !defined _samp_included
	#tryinclude <a_samp>
	#if !defined _samp_included
		#error  "<a_samp.inc> was not found. Make sure the include is correctly placed on your includes folder."
	#endif
#endif


#if !defined mysql_included
	#tryinclude <a_mysql>
	#if !defined mysql_included
		#error  "<a_mysql> was not found on your includes folder. Make sure you have the a_mysql include by BlueG on your includes folder."
	#endif
#endif


//By Y_Less
//native strcpy(dest[], src[], len = sizeof(dest));
#if !defined strcpy
#define strcpy(%0,%1) \
	strcat((%0[0] = '\0', %0), %1)
#endif
#if !defined isnull
#define isnull(%1) \
	((!(%1[0])) || (((%1[0]) == '\1') && (!(%1[1]))))
#endif
#if !defined SQL_MAX_HANDLES
	#define SQL_MAX_HANDLES                 (3)//Maximun concurrent handles running.
#endif

#define SQL_MAX_TABLE_NAME 					(64)
#define SQL_MAX_QUERY_LENGTH				(8192)
#define SQL_MAX_INDEXES						(4)
#define SQL_INVALID_HANDLE					(-1)
#define SQL_FORM_LENGTH						(256)
#define SQL:: 	SQL_T
#define SQL_Warning(%0)      				(printf("[MYSQL] - WARNING: " %0))
#define SQL_Error(%0)        				(printf("[MYSQL] - ERROR: " %0))
#define SQL_Notice(%0)						(printf("[MYSQL] - NOTICE: " %0))


/*==============================================================================
Internal variables and functions
================================================================================*/

enum SQL::datatypes {SQL_TYPE_INT, SQL_TYPE_VCHAR, SQL_TYPE_FLOAT}
enum SQL::qtypes {SQL::UPDATE, SQL::UPDATE2, SQL::TUPDATE, SQL::CREATE, SQL::INSERT, SQL_TYPE_DELETE, SQL::READ, SQL::READ2, SQL::TREAD, SQL::CALLBACK, SQL::MREAD, SQL::MREAD2, SQL::MTREAD}
enum SQL::ftypes {SQL::CASCADE, SQL::SETNULL, SQL::NOACTION, SQL::RESTRICT}

static stock
	bool:SQL::UsedHandle[SQL_MAX_HANDLES],
	MySQL:SQL::upd_connectionHandle[SQL_MAX_HANDLES],
	SQL::upd_table[SQL_MAX_HANDLES][SQL_MAX_TABLE_NAME],
	SQL::upd_rowidentifier[SQL_MAX_HANDLES][SQL_FORM_LENGTH],
	SQL::upd_query[SQL_MAX_HANDLES][SQL_MAX_QUERY_LENGTH],
	SQL::upd_query_2[SQL_MAX_HANDLES][SQL_MAX_QUERY_LENGTH],
	SQL::upd_increment_key[SQL_MAX_HANDLES][SQL_FORM_LENGTH],
	SQL::upd_form[SQL_FORM_LENGTH],
	SQL::upd_form2[SQL_FORM_LENGTH],
	SQL::upd_datacount[SQL_MAX_HANDLES],
	SQL::qtypes:SQL::upd_type[SQL_MAX_HANDLES],
	SQL::upd_pos[SQL_MAX_HANDLES],
	Cache:SQL::ReadCache[SQL_MAX_HANDLES],
	bool:SQL::upd_useautoincrement[SQL_MAX_HANDLES],
    SQL::primarykey[SQL_MAX_HANDLES][64],
    SQL::index_set[SQL_MAX_HANDLES][SQL_MAX_INDEXES],
    SQL::index[SQL_MAX_HANDLES][SQL_MAX_INDEXES][64],
	SQL::isset_primarykey[SQL_MAX_HANDLES]
;


stock IsMultiReadHandleValid(handle)
{
	if(!SQL::IsValidUpdatingSlot(handle)) return 0;
	if(SQL::upd_type[handle] != SQL::CALLBACK) return 0;
	return 1;
}
static stock SQL::GetFreeUpdatingSlot()
{
	new i = 0;
	while (i < sizeof (SQL::UsedHandle) && SQL::UsedHandle[i] == true)
	{
		i++;
	}
	if(i == sizeof (SQL::UsedHandle)) return SQL_INVALID_HANDLE;
	return i;
}

static stock SQL::GetFreeIndexSlot(handle)
{
	new i = 0;
	while (i < sizeof (SQL::index_set) && SQL::index_set[handle][i] == 1)
	{
		i++;
	}
	if(i == sizeof (SQL::index_set)) return SQL_INVALID_HANDLE;
	return i;
}


static stock SQL::RetFType(SQL::ftypes:type)
{
	new SQL::fname[64];
	switch(type)
	{
		case SQL::CASCADE: SQL::fname = "CASCADE";
		case SQL::SETNULL: SQL::fname = "SET NULL";
		case SQL::RESTRICT: SQL::fname = "RESTRICT";
		case SQL::NOACTION: SQL::fname = "NO ACTION";
	}
	return SQL::fname;
}

static stock SQL::IsValidUpdatingSlot(handle)
{
	return SQL::UsedHandle[handle];
}

static stock SQL::OpenTable_MultiRead(const table[], const column_where[] = "", row_identifier = -1, limit = -1, limit2 = -1, const desc[] = "", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable) Invalid table length.");
		return SQL_INVALID_HANDLE;
	}
	new
		i = SQL::GetFreeUpdatingSlot()
	;
	if(i == SQL_INVALID_HANDLE)
	{
		return SQL_INVALID_HANDLE;
	}
	SQL::upd_type[i] = SQL::CALLBACK;
	format(SQL::upd_form, sizeof(SQL::upd_form), "SELECT * FROM `%s`", table);
	if(!isnull(column_where))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " WHERE `%s`='%d'", column_where, row_identifier);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(desc))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " ORDER BY `%s` DESC", desc);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(limit != -1 && limit2 != -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " LIMIT %d, %d", limit, limit2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(limit != -1 && limit2 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " LIMIT %d", limit);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	SQL::ReadCache[i] = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	format(SQL::upd_query[i], SQL_MAX_QUERY_LENGTH, "%s", SQL::upd_form);
	SQL::upd_datacount[i] = 0;
	if(cache_is_valid(SQL::ReadCache[i]))
	{
		SQL::UsedHandle[i] = true;
		SQL::upd_connectionHandle[i] = connectionHandle;
		return i;
	}
	return SQL_INVALID_HANDLE;
}

static stock SQL::OpenTable_MultiRead2(const table[], const column_where[] = "", row_identifier = -1, const column_where2[] = "", row_identifier2 = -1, const row_identifier3[] = "", limit = -1, limit2 = -1, const desc[] = "", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable) Invalid table length.");
		return SQL_INVALID_HANDLE;
	}
	new
		i = SQL::GetFreeUpdatingSlot()
	;
	if(i == SQL_INVALID_HANDLE)
	{
		return SQL_INVALID_HANDLE;
	}
	SQL::upd_type[i] = SQL::CALLBACK;
	format(SQL::upd_form, sizeof(SQL::upd_form), "SELECT * FROM `%s`", table);
	if(!isnull(column_where))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " WHERE `%s`='%d'", column_where, row_identifier);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(column_where2) && row_identifier2 != -1 && isnull(row_identifier3))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%d'", column_where2, row_identifier2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(column_where2) && !isnull(row_identifier3) && row_identifier2 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%s'", column_where2, row_identifier3);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(desc))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " ORDER BY `%s` DESC", desc);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(limit != -1 && limit2 != -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " LIMIT %d, %d", limit, limit2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(limit != -1 && limit2 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " LIMIT %d", limit);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	SQL::ReadCache[i] = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	format(SQL::upd_query[i], SQL_MAX_QUERY_LENGTH, "%s", SQL::upd_form);
	SQL::upd_datacount[i] = 0;
	if(cache_is_valid(SQL::ReadCache[i]))
	{
		SQL::UsedHandle[i] = true;
		SQL::upd_connectionHandle[i] = connectionHandle;
		return i;
	}
	return SQL_INVALID_HANDLE;
}

static stock SQL::OpenTable_MultiReadEx(const table[], const column_where[] = "", const row_identifier[] = "", limit = -1, limit2 = -1, const desc[] = "", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable) Invalid table length.");
		return SQL_INVALID_HANDLE;
	}
	new
		i = SQL::GetFreeUpdatingSlot()
	;
	if(i == SQL_INVALID_HANDLE)
	{
		return SQL_INVALID_HANDLE;
	}
	SQL::upd_type[i] = SQL::CALLBACK;
	format(SQL::upd_form, sizeof(SQL::upd_form), "SELECT * FROM `%s`", table);
	if(!isnull(column_where))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " WHERE `%s`='%s'", column_where, row_identifier);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(desc))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " ORDER BY `%s` DESC", desc);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(limit != -1 && limit2 != -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " LIMIT %d, %d", limit, limit2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(limit != -1 && limit2 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " LIMIT %d", limit);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	SQL::ReadCache[i] = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	format(SQL::upd_query[i], SQL_MAX_QUERY_LENGTH, "%s", SQL::upd_form);
	SQL::upd_datacount[i] = 0;
	if(cache_is_valid(SQL::ReadCache[i]))
	{
		SQL::UsedHandle[i] = true;
		SQL::upd_connectionHandle[i] = connectionHandle;
		return i;
	}
	return SQL_INVALID_HANDLE;
}

static stock SQL::OpenTable_MultiReadEx2(const table[], const column_where[] = "", const row_identifier[] = "", const column_where2[] = "", const row_identifier2[] = "", row_identifier3 = -1, limit = -1, limit2 = -1, const desc[] = "", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable) Invalid table length.");
		return SQL_INVALID_HANDLE;
	}
	new
		i = SQL::GetFreeUpdatingSlot()
	;
	if(i == SQL_INVALID_HANDLE)
	{
		return SQL_INVALID_HANDLE;
	}
	SQL::upd_type[i] = SQL::CALLBACK;
	format(SQL::upd_form, sizeof(SQL::upd_form), "SELECT * FROM `%s`", table);
	if(!isnull(column_where))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " WHERE `%s`='%s'", column_where, row_identifier);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(column_where2) && !isnull(row_identifier2) && row_identifier3 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%s'", column_where2, row_identifier2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(column_where2) && row_identifier3 != -1 && isnull(row_identifier2))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%d'", column_where2, row_identifier3);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(desc))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " ORDER BY `%s` DESC", desc);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(limit != -1 && limit2 != -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " LIMIT %d, %d", limit, limit2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(limit != -1 && limit2 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " LIMIT %d", limit);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	SQL::ReadCache[i] = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	format(SQL::upd_query[i], SQL_MAX_QUERY_LENGTH, "%s", SQL::upd_form);
	SQL::upd_datacount[i] = 0;
	if(cache_is_valid(SQL::ReadCache[i]))
	{
		SQL::UsedHandle[i] = true;
		SQL::upd_connectionHandle[i] = connectionHandle;
		return i;
	}
	return SQL_INVALID_HANDLE;
}

static stock SQL::OpenTable_MultiTableRead(const table[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable) Invalid table length.");
		return SQL_INVALID_HANDLE;
	}
	new
		i = SQL::GetFreeUpdatingSlot()
	;
	if(i == SQL_INVALID_HANDLE)
	{
		return SQL_INVALID_HANDLE;
	}
	SQL::upd_type[i] = SQL::CALLBACK;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT * FROM `%s` ", table);
	SQL::ReadCache[i] = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	format(SQL::upd_query[i], SQL_MAX_QUERY_LENGTH, "%s", SQL::upd_form);
	SQL::upd_datacount[i] = 0;
	if(cache_is_valid(SQL::ReadCache[i]))
	{
		SQL::UsedHandle[i] = true;
		SQL::upd_connectionHandle[i] = connectionHandle;
		return i;
	}
	return SQL_INVALID_HANDLE;
}

static stock SQL::CreateTable(const tablename[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(strlen(tablename) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::CreateTable) Invalid table length.");
		return SQL_INVALID_HANDLE;
	}
	new
		i = SQL::GetFreeUpdatingSlot()
	;
	if(i == SQL_INVALID_HANDLE)
	{
		return SQL_INVALID_HANDLE;
	}
	SQL::upd_type[i] = SQL::CREATE;
	SQL::upd_connectionHandle[i] = connectionHandle;
	strcpy(SQL::upd_table[i], tablename);
	format(SQL::upd_query[i], SQL_MAX_QUERY_LENGTH, "CREATE TABLE %s (", SQL::upd_table[i]);
	SQL::upd_datacount[i] = 0;
	SQL::isset_primarykey[i] = 0;
	SQL::UsedHandle[i] = true;
	return i;
}

static stock SQL::OpenTable_Read(const table[], const column_where[] = "", row_identifier = -1, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable) Invalid table length.");
		return SQL_INVALID_HANDLE;
	}
	if(column_where[0] == '\0')
	{
		SQL_Error("(SQL::OpenTable) No format has been entered. Format example: (pID)");
		return SQL_INVALID_HANDLE;
	}
	new
		i = SQL::GetFreeUpdatingSlot()
	;
	if(i == SQL_INVALID_HANDLE)
	{
		return SQL_INVALID_HANDLE;
	}
	SQL::upd_type[i] = SQL::READ;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT * FROM `%s` WHERE `%s`='%d' ", table, column_where, row_identifier);
	SQL::ReadCache[i] = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	format(SQL::upd_query[i], SQL_MAX_QUERY_LENGTH, "%s", SQL::upd_form);
	SQL::upd_datacount[i] = 0;
	if(cache_is_valid(SQL::ReadCache[i]))
	{
		SQL::UsedHandle[i] = true;
		SQL::upd_connectionHandle[i] = connectionHandle;
		return i;
	}
	return SQL_INVALID_HANDLE;
}

static stock SQL::OpenTable_Read2(const table[], const column_where[] = "", row_identifier = -1, const column_where2[] = "", row_identifier2 = -1, const row_identifier3[] = "", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable) Invalid table length.");
		return SQL_INVALID_HANDLE;
	}
	if(column_where[0] == '\0')
	{
		SQL_Error("(SQL::OpenTable) No format has been entered. Format example: (pID)");
		return SQL_INVALID_HANDLE;
	}
	if(column_where2[0] == '\0')
	{
		SQL_Error("(SQL::OpenTable) No format has been entered. Format example: (pID)");
		return SQL_INVALID_HANDLE;
	}
	new
		i = SQL::GetFreeUpdatingSlot()
	;
	if(i == SQL_INVALID_HANDLE)
	{
		return SQL_INVALID_HANDLE;
	}
	SQL::upd_type[i] = SQL::READ;
	format(SQL::upd_form, sizeof(SQL::upd_form), "SELECT * FROM `%s` WHERE `%s`='%d'", table, column_where, row_identifier);
	if(!isnull(column_where2) && row_identifier2 != -1 && isnull(row_identifier3))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%d'", column_where2, row_identifier2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(column_where2) && !isnull(row_identifier3) && row_identifier2 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%s'", column_where2, row_identifier3);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	SQL::ReadCache[i] = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	format(SQL::upd_query[i], SQL_MAX_QUERY_LENGTH, "%s", SQL::upd_form);
	SQL::upd_datacount[i] = 0;
	if(cache_is_valid(SQL::ReadCache[i]))
	{
		SQL::UsedHandle[i] = true;
		SQL::upd_connectionHandle[i] = connectionHandle;
		return i;
	}
	return SQL_INVALID_HANDLE;
}

static stock SQL::OpenTable_ReadEx(const table[], const column_where[] = "", const row_identifier[] = "", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable_ReadEx) Invalid table length.");
		return SQL_INVALID_HANDLE;
	}
	if(column_where[0] == '\0')
	{
		SQL_Error("(SQL::OpenTable_ReadEx) No format has been entered. Format example: (pID)");
		return SQL_INVALID_HANDLE;
	}
	new
		i = SQL::GetFreeUpdatingSlot()
	;
	if(i == SQL_INVALID_HANDLE)
	{
		return SQL_INVALID_HANDLE;
	}
	SQL::upd_type[i] = SQL::READ;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT * FROM `%s` WHERE `%s`='%s' ", table, column_where, row_identifier);
	SQL::ReadCache[i] = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	format(SQL::upd_query[i], SQL_MAX_QUERY_LENGTH, "%s", SQL::upd_form);
	SQL::upd_datacount[i] = 0;
	if(cache_is_valid(SQL::ReadCache[i]))
	{
		SQL::UsedHandle[i] = true;
		SQL::upd_connectionHandle[i] = connectionHandle;
		return i;
	}
	return SQL_INVALID_HANDLE;
}

static stock SQL::OpenTable_ReadEx2(const table[], const column_where[] = "", const row_identifier[] = "", const column_where2[] = "", const row_identifier2[] = "", row_identifier3 = -1, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable_ReadEx) Invalid table length.");
		return SQL_INVALID_HANDLE;
	}
	if(column_where[0] == '\0')
	{
		SQL_Error("(SQL::OpenTable_ReadEx) No format has been entered. Format example: (pID)");
		return SQL_INVALID_HANDLE;
	}
	if(column_where2[0] == '\0')
	{
		SQL_Error("(SQL::OpenTable_ReadEx) No format has been entered. Format example: (pID)");
		return SQL_INVALID_HANDLE;
	}
	new
		i = SQL::GetFreeUpdatingSlot()
	;
	if(i == SQL_INVALID_HANDLE)
	{
		return SQL_INVALID_HANDLE;
	}
	SQL::upd_type[i] = SQL::READ;
	format(SQL::upd_form, sizeof(SQL::upd_form), "SELECT * FROM `%s` WHERE `%s`='%s'", table, column_where, row_identifier);
	if(!isnull(column_where2) && !isnull(row_identifier2) && row_identifier3 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%s'", column_where2, row_identifier2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(column_where2) && row_identifier3 != -1 && isnull(row_identifier2))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%d'", column_where2, row_identifier3);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	SQL::ReadCache[i] = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	format(SQL::upd_query[i], SQL_MAX_QUERY_LENGTH, "%s", SQL::upd_form);
	SQL::upd_datacount[i] = 0;
	if(cache_is_valid(SQL::ReadCache[i]))
	{
		SQL::UsedHandle[i] = true;
		SQL::upd_connectionHandle[i] = connectionHandle;
		return i;
	}
	return SQL_INVALID_HANDLE;
}

static stock SQL::OpenTable_TableRead(const table[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable) Invalid table length.");
		return SQL_INVALID_HANDLE;
	}
	new
		i = SQL::GetFreeUpdatingSlot()
	;
	if(i == SQL_INVALID_HANDLE)
	{
		return SQL_INVALID_HANDLE;
	}
	SQL::upd_type[i] = SQL::READ;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT * FROM `%s` ", table);
	SQL::ReadCache[i] = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	format(SQL::upd_query[i], SQL_MAX_QUERY_LENGTH, "%s", SQL::upd_form);
	SQL::upd_datacount[i] = 0;
	if(cache_is_valid(SQL::ReadCache[i]))
	{
		SQL::UsedHandle[i] = true;
		SQL::upd_connectionHandle[i] = connectionHandle;
		return i;
	}
	return SQL_INVALID_HANDLE;
}

static stock SQL::OpenTable_TableUpdate(const table[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable) Invalid table length.");
		return SQL_INVALID_HANDLE;
	}
	new
		i = SQL::GetFreeUpdatingSlot()
	;
	if(i == SQL_INVALID_HANDLE)
	{
		return SQL_INVALID_HANDLE;
	}
	SQL::upd_type[i] = SQL::TUPDATE;
	strcpy(SQL::upd_table[i], table);
	format(SQL::upd_query[i], SQL_MAX_QUERY_LENGTH, "UPDATE `%s` SET ", SQL::upd_table[i]);
	SQL::upd_datacount[i] = 0;
	SQL::UsedHandle[i] = true;
	SQL::upd_connectionHandle[i] = connectionHandle;
	return i;
}

static stock SQL::OpenTable_Update(const table[], const column_where[] = "", row_identifier = -1, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable) Invalid table length.");
		return SQL_INVALID_HANDLE;
	}
	if(column_where[0] == '\0')
	{
		SQL_Error("(SQL::OpenTable) No format has been entered. Format example: (pID)");
		return SQL_INVALID_HANDLE;
	}
	new
		i = SQL::GetFreeUpdatingSlot()
	;
	if(i == SQL_INVALID_HANDLE)
	{
		return SQL_INVALID_HANDLE;
	}
	SQL::upd_type[i] = SQL::UPDATE;
	strcpy(SQL::upd_table[i], table);
	format(SQL::upd_query[i], SQL_MAX_QUERY_LENGTH, "UPDATE `%s` SET ", SQL::upd_table[i]);
	format(SQL::upd_rowidentifier[i], SQL_FORM_LENGTH, " WHERE `%s`='%d' ", column_where, row_identifier);
	SQL::upd_datacount[i] = 0;
	SQL::UsedHandle[i] = true;
	SQL::upd_connectionHandle[i] = connectionHandle;
	return i;
}

static stock SQL::OpenTable_Update2(const table[], const column_where[] = "", row_identifier = -1, const column_where2[] = "", row_identifier2 = -1, const row_identifier3[] = "", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable) Invalid table length.");
		return SQL_INVALID_HANDLE;
	}
	if(column_where[0] == '\0')
	{
		SQL_Error("(SQL::OpenTable) No format has been entered. Format example: (pID)");
		return SQL_INVALID_HANDLE;
	}
	if(column_where2[0] == '\0')
	{
		SQL_Error("(SQL::OpenTable) No format has been entered. Format example: (pID)");
		return SQL_INVALID_HANDLE;
	}
	new
		i = SQL::GetFreeUpdatingSlot()
	;
	if(i == SQL_INVALID_HANDLE)
	{
		return SQL_INVALID_HANDLE;
	}
	SQL::upd_type[i] = SQL::UPDATE;
	strcpy(SQL::upd_table[i], table);
	format(SQL::upd_query[i], SQL_MAX_QUERY_LENGTH, "UPDATE `%s` SET ", SQL::upd_table[i]);
	format(SQL::upd_rowidentifier[i], SQL_FORM_LENGTH, " WHERE `%s`='%d'", column_where, row_identifier);
	if(!isnull(column_where2) && row_identifier2 != -1 && isnull(row_identifier3))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%d'", column_where2, row_identifier2);
		strcat(SQL::upd_rowidentifier[i], SQL::upd_form2);
	}
	if(!isnull(column_where2) && !isnull(row_identifier3) && row_identifier2 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%s'", column_where2, row_identifier3);
		strcat(SQL::upd_rowidentifier[i], SQL::upd_form2);
	}
	strcat(SQL::upd_rowidentifier[i], " ");
	SQL::upd_datacount[i] = 0;
	SQL::UsedHandle[i] = true;
	SQL::upd_connectionHandle[i] = connectionHandle;
	return i;
}

static stock SQL::OpenTable_UpdateEx(const table[], const column_where[] = "", const row_identifier[] = "", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable_UpdateEx) Invalid table length.");
		return SQL_INVALID_HANDLE;
	}
	if(column_where[0] == '\0')
	{
		SQL_Error("(SQL::OpenTable_UpdateEx) No format has been entered. Format example: (pID)");
		return SQL_INVALID_HANDLE;
	}
	new
		i = SQL::GetFreeUpdatingSlot()
	;
	if(i == SQL_INVALID_HANDLE)
	{
		return SQL_INVALID_HANDLE;
	}
	SQL::upd_type[i] = SQL::UPDATE;
	strcpy(SQL::upd_table[i], table);
	format(SQL::upd_query[i], SQL_MAX_QUERY_LENGTH, "UPDATE `%s` SET ", SQL::upd_table[i]);
	format(SQL::upd_rowidentifier[i], SQL_FORM_LENGTH, " WHERE `%s`='%s' ", column_where, row_identifier);
	SQL::upd_datacount[i] = 0;
	SQL::UsedHandle[i] = true;
	SQL::upd_connectionHandle[i] = connectionHandle;
	return i;
}

static stock SQL::OpenTable_UpdateEx2(const table[], const column_where[] = "", const row_identifier[] = "", const column_where2[] = "", const row_identifier2[] = "", row_identifier3 = -1, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable_UpdateEx) Invalid table length.");
		return SQL_INVALID_HANDLE;
	}
	if(column_where[0] == '\0')
	{
		SQL_Error("(SQL::OpenTable_UpdateEx) No format has been entered. Format example: (pID)");
		return SQL_INVALID_HANDLE;
	}
	if(column_where2[0] == '\0')
	{
		SQL_Error("(SQL::OpenTable_UpdateEx) No format has been entered. Format example: (pID)");
		return SQL_INVALID_HANDLE;
	}
	new
		i = SQL::GetFreeUpdatingSlot()
	;
	if(i == SQL_INVALID_HANDLE)
	{
		return SQL_INVALID_HANDLE;
	}
	SQL::upd_type[i] = SQL::UPDATE;
	strcpy(SQL::upd_table[i], table);
	format(SQL::upd_query[i], SQL_MAX_QUERY_LENGTH, "UPDATE `%s` SET ", SQL::upd_table[i]);
	format(SQL::upd_rowidentifier[i], SQL_FORM_LENGTH, " WHERE `%s`='%s'", column_where, row_identifier);
	if(!isnull(column_where2) && !isnull(row_identifier2) && row_identifier3 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%s'", column_where2, row_identifier2);
		strcat(SQL::upd_rowidentifier[i], SQL::upd_form2);
	}
	if(!isnull(column_where2) && row_identifier3 != -1 && isnull(row_identifier2))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%d'", column_where2, row_identifier3);
		strcat(SQL::upd_rowidentifier[i], SQL::upd_form2);
	}
	strcat(SQL::upd_rowidentifier[i], " ");
	SQL::upd_datacount[i] = 0;
	SQL::UsedHandle[i] = true;
	SQL::upd_connectionHandle[i] = connectionHandle;
	return i;
}

static stock SQL::UpdateFloatEntry(handle, const field[], Float:value)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::UpdateFloatEntry) Invalid handle. Make sure you used (SQL::OpenTable) first.");
		return 0;
	}
	if(SQL::upd_type[handle] != SQL::UPDATE && SQL::upd_type[handle] != SQL::TUPDATE) return 0;
	format(SQL::upd_form, sizeof(SQL::upd_form), "`%s`='%f',", field, value);
	strcat(SQL::upd_query[handle], SQL::upd_form);
	SQL::upd_datacount[handle]++;
	return 1;
}

static stock SQL::UpdateStringEntry(handle, const field[], const value[], bool:use_real_escape = true)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::UpdateStringEntry) Invalid handle. Make sure you used (SQL::OpenTable) first.");
		return 0;
	}
	if(SQL::upd_type[handle] != SQL::UPDATE && SQL::upd_type[handle] != SQL::TUPDATE) return 0;
	if(use_real_escape == true)
	{
		new escape[SQL_FORM_LENGTH];
		mysql_escape_string(value, escape);
		format(SQL::upd_form, sizeof(SQL::upd_form), "`%s`='%s',", field, escape);
	}
	else
	{
		format(SQL::upd_form, sizeof(SQL::upd_form), "`%s`='%s',", field, value);
	}
	strcat(SQL::upd_query[handle], SQL::upd_form);
	SQL::upd_datacount[handle]++;
	return 1;
}
static stock SQL::OpenTable_Insert(const table[], bool:use_autoincrement = false, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable_Insert) Invalid table length.");
		return SQL_INVALID_HANDLE;
	}
	new
		i = SQL::GetFreeUpdatingSlot()
	;
	if(i == SQL_INVALID_HANDLE)
	{
		return SQL_INVALID_HANDLE;
	}
	SQL::upd_type[i] = SQL::INSERT;
	strcpy(SQL::upd_table[i], table);
	format(SQL::upd_query[i], SQL_MAX_QUERY_LENGTH, "INSERT INTO `%s` (", SQL::upd_table[i]);
	strcpy(SQL::upd_query_2[i], " VALUES (");
	SQL::upd_datacount[i] = 0;
	if(use_autoincrement == true)
	{
		SQL::upd_useautoincrement[i] = true;
	}
	else
	{
		SQL::upd_useautoincrement[i] = false;
	}
	SQL::upd_connectionHandle[i] = connectionHandle;
	SQL::UsedHandle[i] = true;
	return i;
}

static stock SQL::InsertIntEntry(handle, const field[], value)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::UpdateIntEntry) Invalid handle. Make sure you used (SQL::OpenTable) first.");
		return 0;
	}
	if(SQL::upd_type[handle] != SQL::INSERT) return 0;
	format(SQL::upd_form, sizeof(SQL::upd_form), "`%s`,", field);
	strcat(SQL::upd_query[handle], SQL::upd_form);
	format(SQL::upd_form, sizeof(SQL::upd_form), "'%i',", value);
	strcat(SQL::upd_query_2[handle], SQL::upd_form);
	SQL::upd_datacount[handle]++;
	return 1;
}

static stock SQL::InsertFloatEntry(handle, const field[], Float:value)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::UpdateIntEntry) Invalid handle. Make sure you used (SQL::OpenTable) first.");
		return 0;
	}
	if(SQL::upd_type[handle] != SQL::INSERT) return 0;
	format(SQL::upd_form, sizeof(SQL::upd_form), "`%s`,", field);
	strcat(SQL::upd_query[handle], SQL::upd_form);
	format(SQL::upd_form, sizeof(SQL::upd_form), "'%f',", value);
	strcat(SQL::upd_query_2[handle], SQL::upd_form);
	SQL::upd_datacount[handle]++;
	return 1;
}

static stock SQL::InsertStringEntry(handle, const field[], const value[], bool:use_real_escape = true)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::UpdateIntEntry) Invalid handle. Make sure you used (SQL::OpenTable) first.");
		return 0;
	}
	if(SQL::upd_type[handle] != SQL::INSERT) return 0;
	format(SQL::upd_form, sizeof(SQL::upd_form), "`%s`,", field);
	strcat(SQL::upd_query[handle], SQL::upd_form);
	if(use_real_escape == true)
	{
		new escape[SQL_FORM_LENGTH];
		mysql_escape_string(value, escape);
		format(SQL::upd_form, sizeof(SQL::upd_form), "'%s',", escape);
	}
	else
	{
		format(SQL::upd_form, sizeof(SQL::upd_form), "'%s',", value);
	}
	strcat(SQL::upd_query_2[handle], SQL::upd_form);
	SQL::upd_datacount[handle]++;
	return 1;
}

/*==============================================================================
Actual functions
================================================================================*/

#define SQL_TReadRetrievedRows(%0,%1) if(IsMultiReadHandleValid(%0)) if(cache_num_rows()) for(new %1 = 0; %1 < cache_num_rows(); %1++)

stock SQL::cache_get_value_name_int(row_idx, const column_where[])
{
	new int = 0;
	cache_get_value_name_int(row_idx, column_where, int);
	return int;
}
stock Float:SQL::cache_get_value_name_float(row_idx, const column_where[])
{
	new Float:fval = 0;
	cache_get_value_name_float(row_idx, column_where, fval);
	return fval;
}
stock MySQL:SQL::Connect(const host[], const user[], const password[], const database[], debugging = 0, port = 3306, bool:autoreconnect = true, pool_size = 2)
{
	switch(debugging)
	{
		case 0: mysql_log(NONE);
		case 1: mysql_log(ALL);
		case 2: mysql_log(ERROR);
		case 3: mysql_log(WARNING);
		case 4: mysql_log(INFO);
		case 5: mysql_log(DEBUG);
		case 6: mysql_log(ERROR | WARNING);
		case 7: mysql_log(ERROR | DEBUG);
		case 8: mysql_log(ERROR | INFO);
		case 9: mysql_log(WARNING | DEBUG);
		case 10: mysql_log(WARNING | INFO);
		case 11: mysql_log(INFO | DEBUG);
		case 12: mysql_log(ERROR | WARNING | DEBUG);
		case 13: mysql_log(ERROR | WARNING | DEBUG | INFO);
		case 14: mysql_log(WARNING | DEBUG | INFO);
	}
	new MySQLOpt:options = mysql_init_options();
	mysql_set_option(options, SERVER_PORT, port);
	mysql_set_option(options, AUTO_RECONNECT, autoreconnect);
	mysql_set_option(options, POOL_SIZE, pool_size);
	new MySQL:SQL::mc = mysql_connect(host, user, password, database, options);
	if(mysql_errno(SQL::mc) != 0)
	{
		SQL_Warning("Could not connect to database %s on host %s | user %s and password %s", database, host, user, password);
	}
	else
	{
	    SQL_Notice("Successfully connected to database!");
	}
	return SQL::mc;
}

stock SQL::Disconnect(MySQL:handle = MYSQL_DEFAULT_HANDLE)
{
	return mysql_close(handle);
}

stock SQL::Begin(MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	mysql_tquery(connectionHandle, "BEGIN", "", "");
	return 1;
}

stock SQL::Commit(MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	mysql_tquery(connectionHandle, "COMMIT", "", "");
	return 1;
}

stock SQL::DeleteRow(const table[], const column_where[] = "", row_identifier = -1, limit = -1, limit2 = -1, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	format(SQL::upd_form, sizeof(SQL::upd_form), "DELETE FROM `%s` WHERE `%s`='%d'", table, column_where, row_identifier);
	if(limit != -1 && limit2 != -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " LIMIT %d, %d", limit, limit2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(limit != -1 && limit2 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " LIMIT %d", limit);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}

stock SQL::DeleteRow2(const table[], const column_where[] = "", row_identifier = -1, const column_where2[] = "", row_identifier2 = -1, const row_identifier3[] = "", limit = -1, limit2 = -1, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	format(SQL::upd_form, sizeof(SQL::upd_form), "DELETE FROM `%s` WHERE `%s`='%d'", table, column_where, row_identifier);
	if(!isnull(column_where2) && row_identifier2 != -1 && isnull(row_identifier3))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%d'", column_where2, row_identifier2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(column_where2) && !isnull(row_identifier3) && row_identifier2 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%s'", column_where2, row_identifier3);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(limit != -1 && limit2 != -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " LIMIT %d, %d", limit, limit2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(limit != -1 && limit2 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " LIMIT %d", limit);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}

stock SQL::DeleteRow3(const table[], MySQL:connectionHandle, const column_where[] = "", type = 'i', {Float,_}:...)
{
	switch (type)
	{
		case 'd', 'i': mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form),  "DELETE FROM `%s` WHERE `%s`='%d' ", table, column_where, getarg(4));
		case 's':
		{
			new i_ArgLoop = -1;
			new i_Extract[128];
			while (++i_ArgLoop != 0x100)
			{
				if(i_ArgLoop < 128)
				{
					i_Extract[i_ArgLoop] = getarg(4, i_ArgLoop);
				}
			}
			mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form),  "DELETE FROM `%s` WHERE `%s`='%e' ", table, column_where, i_Extract);
		}
		case 'f':  mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form),  "DELETE FROM `%s` WHERE `%s`='%f' ", table, column_where, getarg(4));
	}
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}

stock SQL::Open(SQL::qtypes:type, const table[], const column_where[] = "", row_identifier = -1, const column_where2[] = "", row_identifier2 = -1, const row_identifier3[] = "", limit = -1, limit2 = -1, const desc[] = "", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new handle;
	switch(type)
	{
		case SQL::CREATE:
		{
			handle = SQL::CreateTable(table, connectionHandle);
		}
		case SQL::READ:
		{
			handle = SQL::OpenTable_Read(table, column_where, row_identifier, connectionHandle);
		}
		case SQL::READ2:
		{
			handle = SQL::OpenTable_Read2(table, column_where, row_identifier, column_where2, row_identifier2, row_identifier3, connectionHandle);
		}
		case SQL::TREAD:
		{
			handle = SQL::OpenTable_TableRead(table, connectionHandle);
		}
		case SQL::INSERT:
		{
			handle = SQL::OpenTable_Insert(table, false, connectionHandle);
		}
		case SQL::UPDATE:
		{
			handle = SQL::OpenTable_Update(table, column_where, row_identifier, connectionHandle);
		}
		case SQL::UPDATE2:
		{
			handle = SQL::OpenTable_Update2(table, column_where, row_identifier, column_where2, row_identifier2, row_identifier3, connectionHandle);
		}
		case SQL::TUPDATE:
		{
			handle = SQL::OpenTable_TableUpdate(table, connectionHandle);
		}
		case SQL::MREAD:
		{
			handle = SQL::OpenTable_MultiRead(table, column_where, row_identifier, limit, limit2, desc, connectionHandle);
		}
		case SQL::MREAD2:
		{
			handle = SQL::OpenTable_MultiRead2(table, column_where, row_identifier, column_where2, row_identifier2, row_identifier3, limit, limit2, desc, connectionHandle);
		}
		case SQL::MTREAD:
		{
			handle = SQL::OpenTable_MultiTableRead(table, connectionHandle);
		}
	}
	return handle;
}

stock SQL::OpenEx(SQL::qtypes:type, const table[], const column_where[] = "", const row_identifier[] = "", const column_where2[] = "", const row_identifier2[] = "", row_identifier3 = -1, limit = -1, limit2 = -1, const desc[] = "", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new handle;
	switch(type)
	{
		case SQL::CREATE:
		{
			handle = SQL::CreateTable(table, connectionHandle);
		}
		case SQL::READ:
		{
			handle = SQL::OpenTable_ReadEx(table, column_where, row_identifier, connectionHandle);
		}
		case SQL::READ2:
		{
			handle = SQL::OpenTable_ReadEx2(table, column_where, row_identifier, column_where2, row_identifier2, row_identifier3, connectionHandle);
		}
		case SQL::TREAD:
		{
			handle = SQL::OpenTable_TableRead(table, connectionHandle);
		}
		case SQL::INSERT:
		{
			handle = SQL::OpenTable_Insert(table, false, connectionHandle);
		}
		case SQL::UPDATE:
		{
			handle = SQL::OpenTable_UpdateEx(table, column_where, row_identifier, connectionHandle);
		}
		case SQL::UPDATE2:
		{
			handle = SQL::OpenTable_UpdateEx2(table, column_where, row_identifier, column_where2, row_identifier2, row_identifier3, connectionHandle);
		}
		case SQL::TUPDATE:
		{
			handle = SQL::OpenTable_TableUpdate(table, connectionHandle);
		}
		case SQL::MREAD:
		{
			handle = SQL::OpenTable_MultiReadEx(table, column_where, row_identifier, limit, limit2, desc, connectionHandle);
		}
		case SQL::MREAD2:
		{
			handle = SQL::OpenTable_MultiReadEx2(table, column_where, row_identifier, column_where2, row_identifier2, row_identifier3, limit, limit2, desc, connectionHandle);
		}
		case SQL::MTREAD:
		{
			handle = SQL::OpenTable_MultiTableRead(table, connectionHandle);
		}
	}
	return handle;
}



stock SQL::ToggleAutoIncrement(handle, bool:toggle)
{
	if(!SQL::IsValidUpdatingSlot(handle)) return 0;
	if(SQL::upd_type[handle] != SQL::INSERT) return 0;
	SQL::upd_useautoincrement[handle] = toggle;
	return 1;
}

stock SQL::DeleteRowEx(const table[], const column_where[] = "", const row_identifier[] = "", limit = -1, limit2 = -1, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "DELETE FROM `%s` WHERE `%s`='%e'", table, column_where, row_identifier);
	if(limit != -1 && limit2 != -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " LIMIT %d, %d", limit, limit2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(limit != -1 && limit2 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " LIMIT %d", limit);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}

stock SQL::DeleteRowEx2(const table[], const column_where[] = "", const row_identifier[] = "", const column_where2[] = "", const row_identifier2[] = "", row_identifier3 = -1, limit = -1, limit2 = -1, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	format(SQL::upd_form, sizeof(SQL::upd_form), "DELETE FROM `%s` WHERE `%s`='%e'", table, column_where, row_identifier);
	if(!isnull(column_where2) && !isnull(row_identifier2) && row_identifier3 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%e'", column_where2, row_identifier2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(column_where2) && row_identifier3 != -1 && isnull(row_identifier2))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%d'", column_where2, row_identifier3);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(limit != -1 && limit2 != -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " LIMIT %d, %d", limit, limit2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(limit != -1 && limit2 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " LIMIT %d", limit);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}

stock SQL::GetIntEntry(const table[], const field[], const column_where[] = "", row_identifier = -1, const column_whereo[] = "=", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new Cache:result, int = 0, count;
	format(SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`%s'%d' ", field, table, column_where, column_whereo, row_identifier);
	result = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	if(result)
	{
	    cache_get_row_count(count);
		if(count) cache_get_value_name_int(0, field, int);
		cache_delete(result);
	}
	return int;
}

stock SQL::GetIntEntry2(const table[], const field[], const column_where[] = "", row_identifier = -1, const column_where2[] = "", row_identifier2 = -1, const row_identifier3[] = "", const column_whereo[] = "=", const column_whereo2[] = "=", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new Cache:result, int = 0, count;
	format(SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`%s'%d'", field, table, column_where, column_whereo, row_identifier);
	if(!isnull(column_where2) && row_identifier2 != -1 && isnull(row_identifier3))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`%s'%d'", column_where2, column_whereo2, row_identifier2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(column_where2) && !isnull(row_identifier3) && row_identifier2 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`%s'%s'", column_where2, column_whereo2, row_identifier3);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	result = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	if(result)
	{
	    cache_get_row_count(count);
		if(count) cache_get_value_name_int(0, field, int);
		cache_delete(result);
	}
	return int;
}

stock Float:SQL::GetFloatEntry(const table[], const field[], const column_where[] = "", row_identifier = -1, const column_whereo[] = "=", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new
		Cache:result,
		Float:int,
		count
	;
	format(SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`%s'%d' ", field, table, column_where, column_whereo, row_identifier);
	result = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	if(result)
	{
	    cache_get_row_count(count);
		if(count)
		{
			cache_get_value_name_float(0, field, int);
		}
		else
		{
			int = 0.0;
		}
		cache_delete(result);
	}
	return int;
}

stock Float:SQL::GetFloatEntry2(const table[], const field[], const column_where[] = "", row_identifier = -1, const column_where2[] = "", row_identifier2 = -1, const row_identifier3[] = "", const column_whereo[] = "=", const column_whereo2[] = "=", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new
		Cache:result,
		Float:int,
		count
	;
	format(SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`%s'%d'", field, table, column_where, column_whereo, row_identifier);
	if(!isnull(column_where2) && row_identifier2 != -1 && isnull(row_identifier3))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`%s'%d'", column_where2, column_whereo2, row_identifier2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(column_where2) && !isnull(row_identifier3) && row_identifier2 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`%s'%s'", column_where2, column_whereo2, row_identifier3);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	result = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	if(result)
	{
	    cache_get_row_count(count);
		if(count)
		{
			cache_get_value_name_float(0, field, int);
		}
		else
		{
			int = 0.0;
		}
		cache_delete(result);
	}
	return int;
}

stock SQL::t_GetStringEntry(const table[], const field[], const column_where[] = "", row_identifier = -1, const column_whereo[] = "=", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new Cache:result, count;
	format(SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`%s'%d' ", field, table, column_where, column_whereo, row_identifier);
	result = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	SQL::upd_form[0] = EOS;
	if(result)
	{
	    cache_get_row_count(count);
		if(count == 1)
		{
			cache_get_value_name(0, field, SQL::upd_form, sizeof(SQL::upd_form));
			cache_delete(result);
		}
		else
		{
			cache_delete(result);
			return SQL::upd_form;
		}
	}
	return SQL::upd_form;
}

stock SQL::t_GetStringEntry2(const table[], const field[], const column_where[] = "", row_identifier = -1, const column_where2[] = "", row_identifier2 = -1, const row_identifier3[] = "", const column_whereo[] = "=", const column_whereo2[] = "=", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new Cache:result, count;
	format(SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`%s'%d' ", field, table, column_where, column_whereo, row_identifier);
	result = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	SQL::upd_form[0] = EOS;
	if(result)
	{
	    cache_get_row_count(count);
		if(count == 1)
		{
			cache_get_value_name(0, field, SQL::upd_form, sizeof(SQL::upd_form));
			cache_delete(result);
		}
		else
		{
			cache_delete(result);
			return SQL::upd_form;
		}
	}
	return SQL::upd_form;
}

stock SQL::GetStringEntry(const table[], const field[], const column_where[] = "", row_identifier = -1, dest[], len = sizeof(dest), const column_whereo[] = "=", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new Cache:result, count;
	format(SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`%s'%d' ", field, table, column_where, column_whereo, row_identifier);
	result = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	if(result)
	{
	    cache_get_row_count(count);
		if(count == 1)
		{
			cache_get_value_name(0, field, SQL::upd_form, sizeof(SQL::upd_form));
			strcpy(dest, SQL::upd_form, len);
			cache_delete(result);
		}
		else
		{
			cache_delete(result);
			return 0;
		}
	}
	return 1;
}

stock SQL::GetStringEntry2(const table[], const field[], const column_where[] = "", row_identifier = -1, const column_where2[] = "", row_identifier2 = -1, const row_identifier3[] = "", dest[], len = sizeof(dest), const column_whereo[] = "=", const column_whereo2[] = "=", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new Cache:result, count;
	format(SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`%s'%d'", field, table, column_where, column_whereo, row_identifier);
	if(!isnull(column_where2) && row_identifier2 != -1 && isnull(row_identifier3))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`%s'%d'", column_where2, column_whereo2, row_identifier2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(column_where2) && !isnull(row_identifier3) && row_identifier2 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`%s'%s'", column_where2, column_whereo2, row_identifier3);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	result = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	if(result)
	{
	    cache_get_row_count(count);
		if(count == 1)
		{
			cache_get_value_name(0, field, SQL::upd_form, sizeof(SQL::upd_form));
			strcpy(dest, SQL::upd_form, len);
			cache_delete(result);
		}
		else
		{
			cache_delete(result);
			return 0;
		}
	}
	return 1;
}

stock SQL::GetStringEntryEx(const table[], const field[], const column_where[] = "", const scolumn_where[] = "", dest[], len = sizeof(dest), const column_whereo[] = "=", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new Cache:result, count;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`%s'%e' ", field, table, column_where, column_whereo, scolumn_where);
	result = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	if(result)
	{
	    cache_get_row_count(count);
		if(count == 1)
		{
			cache_get_value_name(0, field, SQL::upd_form, sizeof(SQL::upd_form));
			strcpy(dest, SQL::upd_form, len);
			cache_delete(result);
		}
		else
		{
			cache_delete(result);
			return 0;
		}
	}
	return 1;
}

stock SQL::GetStringEntryEx2(const table[], const field[], const column_where[] = "", const scolumn_where[] = "", const column_where2[] = "", const scolumn_where2[] = "", scolumn_where3 = -1, dest[], len = sizeof(dest), const column_whereo[] = "=", const column_whereo2[] = "=", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new Cache:result, count;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`%s'%e'", field, table, column_where, column_whereo, scolumn_where);
	if(!isnull(column_where2) && !isnull(scolumn_where2) && scolumn_where3 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`%s'%e'", column_where2, column_whereo2, scolumn_where2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(column_where2) && scolumn_where3 != -1 && isnull(scolumn_where2))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`%s'%d'", column_where2, column_whereo2, scolumn_where3);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	result = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	if(result)
	{
	    cache_get_row_count(count);
		if(count == 1)
		{
			cache_get_value_name(0, field, SQL::upd_form, sizeof(SQL::upd_form));
			strcpy(dest, SQL::upd_form, len);
			cache_delete(result);
		}
		else
		{
			cache_delete(result);
			return 0;
		}
	}
	return 1;
}

stock SQL::t_GetStringEntryEx(const table[], const field[], const column_where[] = "", const scolumn_where[] = "", const column_whereo[] = "=", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new Cache:result, count;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`%s'%e' ", field, table, column_where, column_whereo, scolumn_where);
	result = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	SQL::upd_form[0] = EOS;
	if(result)
	{
	    cache_get_row_count(count);
		if(count == 1)
		{
			cache_get_value_name(0, field, SQL::upd_form, sizeof(SQL::upd_form));
			cache_delete(result);
		}
		else
		{
			cache_delete(result);
			return SQL::upd_form,;
		}
	}
	return SQL::upd_form,;
}

stock SQL::t_GetStringEntryEx2(const table[], const field[], const column_where[] = "", const scolumn_where[] = "", const column_where2[] = "", const scolumn_where2[] = "", scolumn_where3 = -1, const column_whereo[] = "=", const column_whereo2[] = "=", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new Cache:result, count;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`%s'%e'", field, table, column_where, column_whereo, scolumn_where);
	if(!isnull(column_where2) && !isnull(scolumn_where2) && scolumn_where3 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`%s'%e'", column_where2, column_whereo2, scolumn_where2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(column_where2) && scolumn_where3 != -1 && isnull(scolumn_where2))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`%s'%d'", column_where2, column_whereo2, scolumn_where3);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	result = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	SQL::upd_form[0] = EOS;
	if(result)
	{
	    cache_get_row_count(count);
		if(count == 1)
		{
			cache_get_value_name(0, field, SQL::upd_form, sizeof(SQL::upd_form));
			cache_delete(result);
		}
		else
		{
			cache_delete(result);
			return SQL::upd_form,;
		}
	}
	return SQL::upd_form,;
}

stock SQL::GetIntEntryEx(const table[], const field[], const column_where[] = "", scolumn_where[] = "", const column_whereo[] = "=", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new Cache:result, int, count;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`%s'%e' ", field, table, column_where, column_whereo, scolumn_where);
	result = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	if(result)
	{
	    cache_get_row_count(count);
		if(count == 1)
		{
			cache_get_value_name_int(0, field, int);
		}
		else
		{
			int = -1;
		}
		cache_delete(result);
	}
	return int;
}

stock SQL::GetIntEntryEx2(const table[], const field[], const column_where[] = "", scolumn_where[] = "", const column_where2[] = "", scolumn_where2[] = "", scolumn_where3 = -1, const column_whereo[] = "=", const column_whereo2[] = "=", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new Cache:result, int, count;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`%s'%e'", field, table, column_where, column_whereo, scolumn_where);
	if(!isnull(column_where2) && !isnull(scolumn_where2) && scolumn_where3 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`%s'%e'", column_where2, column_whereo2, scolumn_where2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(column_where2) && scolumn_where3 != -1 && isnull(scolumn_where2))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`%s'%d'", column_where2, column_whereo2, scolumn_where3);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	result = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	if(result)
	{
	    cache_get_row_count(count);
		if(count == 1)
		{
			cache_get_value_name_int(0, field, int);
		}
		else
		{
			int = -1;
		}
		cache_delete(result);
	}
	return int;
}

stock Float:SQL::GetFloatEntryEx(const table[], const field[], const column_where[] = "", scolumn_where[] = "", const column_whereo[] = "=", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new Cache:result, Float:int, count;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`%s'%e' ", field, table, column_where, column_whereo, scolumn_where);
	result = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	if(result)
	{
	    cache_get_row_count(count);
		if(count == 1)
		{
			cache_get_value_name_float(0, field, int);
		}
		else
		{
			int = -1.0;
		}
		cache_delete(result);
	}
	return int;
}

stock Float:SQL::GetFloatEntryEx2(const table[], const field[], const column_where[] = "", scolumn_where[] = "", const column_where2[] = "", scolumn_where2[] = "", scolumn_where3 = -1, const column_whereo[] = "=", const column_whereo2[] = "=", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new Cache:result, Float:int, count;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`%s'%e'", field, table, column_where, column_whereo, scolumn_where);
	if(!isnull(column_where2) && !isnull(scolumn_where2) && scolumn_where3 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`%s'%e'", column_where2, column_whereo2, scolumn_where2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(column_where2) && scolumn_where3 != -1 && isnull(scolumn_where2))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`%s'%d'", column_where2, column_whereo2, scolumn_where3);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	result = mysql_query(connectionHandle, SQL::upd_form);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_form);
	}
	if(result)
	{
	    cache_get_row_count(count);
		if(count == 1)
		{
			cache_get_value_name_int(0, field, int);
		}
		else
		{
			int = -1.0;
		}
		cache_delete(result);
	}
	return int;
}

stock SQL::AddTableColumn(handle, const field_name[], SQL::datatypes: type = SQL_TYPE_INT, maxlength = 11, bool:auto_increment = false, bool:setprimary = false, bool:setindex = false)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::AddTableColumn) Invalid handle. Make sure you used (SQL::Open) first.");
		return 0;
	}
	if(SQL::upd_type[handle] != SQL::CREATE) return 0;
	if(SQL::isset_primarykey[handle] && setprimary == true)
	{
		SQL_Error("(SQL::AddTableColumn) A primary column_where was created already. There can only be one primary column_where! [Key %s]", field_name);
		return 0;
	}
	if(auto_increment == true && type == SQL_TYPE_VCHAR)
	{
		SQL_Error("(SQL::AddTableColumn) You can't set auto increment if the type is varchar! [Key %s]", field_name);
		return 0;
	}
	switch(type)
	{
		case SQL_TYPE_INT:
		{
			if(auto_increment == true && setprimary == false)
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s int NOT NULL AUTO_INCREMENT,", field_name, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;
				SQL::isset_primarykey[handle] = 1;
				strcpy(SQL::primarykey[handle], field_name);
			}
			else if(auto_increment == true && setprimary == true)
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s int NOT NULL AUTO_INCREMENT,", field_name, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;
				SQL::isset_primarykey[handle] = 1;
				strcpy(SQL::primarykey[handle], field_name);
			}
			else if(setprimary == true)
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s int NOT NULL PRIMARY KEY,", field_name, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;
				SQL::isset_primarykey[handle] = 2;
				strcpy(SQL::primarykey[handle], field_name);
			}
			else
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s int(%d) NOT NULL,", field_name, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;
			}
		}
		case SQL_TYPE_VCHAR:
		{
			if(auto_increment == true) return 0;
			if(setprimary == true)
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s varchar(%d) NOT NULL PRIMARY KEY,", field_name, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;
				SQL::isset_primarykey[handle] = 2;
				strcpy(SQL::primarykey[handle], field_name);
			}
			else
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s varchar(%d) NOT NULL,", field_name, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;

			}
		}
		case SQL_TYPE_FLOAT:
		{
			if(auto_increment == true) return 0;
			if(setprimary == true)
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s float(%d) NOT NULL PRIMARY KEY,", field_name, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;
				SQL::isset_primarykey[handle] = 2;
				strcpy(SQL::primarykey[handle], field_name);
			}
			else
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s float(%d) NOT NULL,", field_name, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;
			}
		}
	}
	if(setindex == true)
	{
		new indexslot = SQL::GetFreeIndexSlot(handle);
		if(indexslot == SQL_INVALID_HANDLE) return SQL_Error("(SQL::AddTableColumn) Indexes limit reached. Increase the limit in MAX_INDEXES");
		SQL::index_set[handle][indexslot] = 1;
		strcpy(SQL::index[handle][indexslot], field_name);
	}
	return 1;
}

stock SQL::ReadInt(handle, const field[], &dest, row = 0)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::ReadInt) Invalid handle. Make sure you opened the table first.");
		return 0;
	}
	if(SQL::upd_type[handle] == SQL::READ)
	{
		cache_get_value_name_int(0, field, dest);
		SQL::upd_datacount[handle]++;
	}
	else if(SQL::upd_type[handle] == SQL::CALLBACK)
	{
	    cache_get_value_name_int(row, field, dest);
	    SQL::upd_datacount[handle]++;
	}
	else return 0;
	return 1;
}

stock SQL::ReadFloat(handle, const field[], &Float:dest, row = 0)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::ReadInt) Invalid handle. Make sure you opened the table first.");
		return 0;
	}
	if(SQL::upd_type[handle] == SQL::READ)
	{
		cache_get_value_name_float(0, field, dest);
		SQL::upd_datacount[handle]++;
	}
	else if(SQL::upd_type[handle] == SQL::CALLBACK)
	{
	    cache_get_value_name_float(row, field, dest);
	    SQL::upd_datacount[handle]++;
	}
	else return 0;
	return 1;
}

stock SQL::ReadString(handle, const field[], dest[], len = sizeof(dest), row = 0)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::ReadInt) Invalid handle. Make sure you opened the table first.");
		return 0;
	}
	if(SQL::upd_type[handle] == SQL::READ)
	{
		cache_get_value_name(0, field, dest, len+1);
		SQL::upd_datacount[handle]++;
	}
	else if(SQL::upd_type[handle] == SQL::CALLBACK)
	{
        cache_get_value_name(row, field, dest, len+1);
        SQL::upd_datacount[handle]++;
	}
	else return 0;
	return 1;
}


stock SQL::UpdateIntEntry(handle, const field[], value)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::UpdateIntEntry) Invalid handle. Make sure you used (SQL::OpenTable) first.");
		return 0;
	}
	if(SQL::upd_type[handle] != SQL::UPDATE && SQL::upd_type[handle] != SQL::TUPDATE) return 0;
	format(SQL::upd_form, sizeof(SQL::upd_form), "`%s`='%i',", field, value);
	strcat(SQL::upd_query[handle], SQL::upd_form);
	SQL::upd_datacount[handle]++;
	return 1;
}

stock SQL::WriteInt(handle, const field[], value)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::WriteInt) Invalid handle. Make sure you used (SQL::Open) first.");
		return 0;
	}
	switch(SQL::upd_type[handle])
	{
		case SQL::UPDATE:
		{
			return SQL::UpdateIntEntry(handle, field, value);
		}
		case SQL::TUPDATE:
		{
			return SQL::UpdateIntEntry(handle, field, value);
		}
		case SQL::INSERT:
		{
			return SQL::InsertIntEntry(handle, field, value);
		}
	}
	return 0;
}

stock SQL::WriteFloat(handle, const field[], Float:value)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::WriteFloat) Invalid handle. Make sure you used (SQL::Open) first.");
		return 0;
	}
	switch(SQL::upd_type[handle])
	{
		case SQL::UPDATE:
		{
			return SQL::UpdateFloatEntry(handle, field, value);
		}
		case SQL::TUPDATE:
		{
			return SQL::UpdateFloatEntry(handle, field, value);
		}
		case SQL::INSERT:
		{
			return SQL::InsertFloatEntry(handle, field, value);
		}
	}
	return 0;
}

stock SQL::WriteString(handle, const field[], const value[], bool: use_real_escape = true)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::WriteFloat) Invalid handle. Make sure you used (SQL::Open) first.");
		return 0;
	}
	switch(SQL::upd_type[handle])
	{
		case SQL::UPDATE:
		{
			return SQL::UpdateStringEntry(handle, field, value, use_real_escape);
		}
		case SQL::TUPDATE:
		{
			return SQL::UpdateStringEntry(handle, field, value, use_real_escape);
		}
		case SQL::INSERT:
		{
			return SQL::InsertStringEntry(handle, field, value, use_real_escape);
		}
	}
	return 0;
}


stock SQL::Close(handle)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::Close) Invalid handle. Make sure you opened a handle first.");
		return 0;
	}
	new MySQL:connectionHandle = SQL::upd_connectionHandle[handle];
	if(SQL::upd_type[handle] == SQL::UPDATE)
	{
		if(!SQL::upd_datacount[handle])
		{
			SQL_Error("%s", SQL::upd_query[handle]);
			SQL_Error("(SQL::Close) No data has been updated.");
			SQL::UsedHandle[handle] = false;
			return 0;
		}
		//Remove last comma
		new pos = strlen(SQL::upd_query[handle])-1;
		SQL::upd_query[handle][pos] = ' ';
		strcat(SQL::upd_query[handle], SQL::upd_rowidentifier[handle]);

		mysql_tquery(connectionHandle, SQL::upd_query[handle], "", "");
		SQL::upd_table[handle][0] = '\0';
		SQL::upd_query[handle][0] = '\0';
		SQL::upd_rowidentifier[handle][0] = '\0';
		SQL::upd_datacount[handle] = 0;
		SQL::UsedHandle[handle] = false;
	}
	else if(SQL::upd_type[handle] == SQL::TUPDATE)
	{
		//Remove last comma
		new pos = strlen(SQL::upd_query[handle])-1;
		SQL::upd_query[handle][pos] = ' ';

		mysql_tquery(connectionHandle, SQL::upd_query[handle], "", "");
		SQL::upd_table[handle][0] = '\0';
		SQL::upd_query[handle][0] = '\0';
		SQL::upd_datacount[handle] = 0;
		SQL::UsedHandle[handle] = false;
	}
	else if(SQL::upd_type[handle] == SQL::CREATE)
	{
		if(!SQL::upd_datacount[handle])
		{
			SQL_Error("%s", SQL::upd_query[handle]);
			SQL_Error("(SQL::Close) No data has been added..");
			SQL::UsedHandle[handle] = false;
			return 0;
		}
		new pos = strlen(SQL::upd_query[handle])-1;
		SQL::upd_query[handle][pos] = ' ';
		for(new i = 0; i != SQL_MAX_INDEXES; i++)
		{
			if(SQL::index_set[handle][i])
			{
				format(SQL::upd_form, 128, ",INDEX (`%s`)", SQL::index[handle][i]);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::index_set[handle][i] = 0;
				SQL::index[handle][i][0] = EOS;
			}
		}
		if(SQL::isset_primarykey[handle] == 1)
		{
			format(SQL::upd_form, 128, ",primary key (%s)", SQL::primarykey[handle]);
			strcat(SQL::upd_query[handle], SQL::upd_form);
		}
		strcat(SQL::upd_query[handle], ");");
		mysql_tquery(connectionHandle, SQL::upd_query[handle], "", "");
		SQL::upd_table[handle][0] = '\0';
		//SQL::upd_connectionHandle[handle] = MYSQL_INVALID_HANDLE;
		SQL::primarykey[handle][0] = '\0';
		SQL::isset_primarykey[handle] = false;
		SQL::upd_query[handle][0] = '\0';
		SQL::upd_rowidentifier[handle][0] = '\0';
		SQL::upd_datacount[handle] = 0;
		SQL::UsedHandle[handle] = false;


	}
	else if(SQL::upd_type[handle] == SQL::INSERT)
	{
		if(!SQL::upd_datacount[handle])
		{
			SQL_Error("%s", SQL::upd_query[handle]);
			SQL_Error("(SQL::Close) No data has been inserted..");
			SQL::UsedHandle[handle] = false;
			return 0;
		}
		new pos = strlen(SQL::upd_query[handle])-1;
		SQL::upd_query[handle][pos] = ')';
		pos = strlen(SQL::upd_query_2[handle])-1;
		SQL::upd_query_2[handle][pos] = ' ';
		strcat(SQL::upd_query_2[handle], ");");
		strcat(SQL::upd_query[handle], SQL::upd_query_2[handle]);
		SQL::upd_table[handle][0] = '\0';
		SQL::upd_query_2[handle][0] = '\0';
		SQL::upd_rowidentifier[handle][0] = '\0';
		SQL::upd_datacount[handle] = 0;
		if(SQL::upd_useautoincrement[handle] == true)
		{
			new Cache:result;
			result = mysql_query(connectionHandle, SQL::upd_query[handle]);
			if(mysql_errno() != 0)
			{
				return SQL_Warning("Query could not be completed due to error: %s", SQL::upd_query[handle]);
			}
			if(result)
			{
				new id = cache_insert_id();
				cache_delete(result);
				SQL::upd_query[handle][0] = '\0';
				SQL::UsedHandle[handle] = false;
				return id;
			}
		}
		else
		{
			mysql_tquery(connectionHandle, SQL::upd_query[handle], "", "");
			SQL::UsedHandle[handle] = false;
		}
	}
	else if(SQL::upd_type[handle] == SQL::READ)
	{
		if(!SQL::upd_datacount[handle])
		{
			SQL_Error("%s", SQL::upd_query[handle]);
			SQL_Error("(SQL::Close) No data has been read..");
			SQL::UsedHandle[handle] = false;
			return 0;
		}
		if(cache_is_valid(SQL::ReadCache[handle]))
		{
			cache_delete(SQL::ReadCache[handle]);
		}
		SQL::UsedHandle[handle] = false;
	}
	else if(SQL::upd_type[handle] == SQL::CALLBACK)
	{
	    new count;
		cache_get_row_count(count);
		if(cache_is_valid(SQL::ReadCache[handle]))
		{
			cache_delete(SQL::ReadCache[handle]);
		}
		SQL::UsedHandle[handle] = false;
		return count;
	}
	return -1;
}

stock SQL::SetIntEntry(const table[], const field[], value, const column_where[] = "", row_identifier = -1, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	format(SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%d' WHERE `%s`='%d' ", table, field, value, column_where, row_identifier);
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}

stock SQL::SetIntEntry2(const table[], const field[], value, const column_where[] = "", row_identifier = -1, const column_where2[] = "", row_identifier2 = -1, const row_identifier3[] = "", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	format(SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%d' WHERE `%s`='%d'", table, field, value, column_where, row_identifier);
	if(!isnull(column_where2) && row_identifier2 != -1 && isnull(row_identifier3))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%d'", column_where2, row_identifier2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(column_where2) && !isnull(row_identifier3) && row_identifier2 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%s'", column_where2, row_identifier3);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}

stock SQL::SetIntEntryEx(const table[], const field[], value, const column_where[] = "", const row_identifier[] = "", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%d' WHERE `%s`='%e' ", table, field, value, column_where, row_identifier);
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}

stock SQL::SetIntEntryEx2(const table[], const field[], value, const column_where[] = "", const row_identifier[] = "", const column_where2[] = "", const row_identifier2[] = "", row_identifier3 = -1, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%d' WHERE `%s`='%e'", table, field, value, column_where, row_identifier);
	if(!isnull(column_where2) && !isnull(row_identifier2) && row_identifier3 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%e'", column_where2, row_identifier2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(column_where2) && row_identifier3 != -1 && isnull(row_identifier2))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%d'", column_where2, row_identifier3);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}

stock SQL::SetStringEntry(const table[], const field[], const value[], const column_where[] = "", row_identifier = -1, bool:use_real_escape = true, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	if(use_real_escape == true)
	{
		//Not using mysql_escape_string as the %e specifier is faster.
		mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%e' WHERE `%s`='%d' ", table, field, value, column_where, row_identifier);
	}
	else
	{
		format(SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%s' WHERE `%s`='%d' ", table, field, value, column_where, row_identifier);
	}
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}

stock SQL::SetStringEntry2(const table[], const field[], const value[], const column_where[] = "", row_identifier = -1, const column_where2[] = "", row_identifier2 = -1, const row_identifier3[] = "", bool:use_real_escape = true, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	if(use_real_escape == true)
	{
		//Not using mysql_escape_string as the %e specifier is faster.
 		mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%e' WHERE `%s`='%d'", table, field, value, column_where, row_identifier);
	}
	else
	{
 		format(SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%e' WHERE `%s`='%d'", table, field, value, column_where, row_identifier);
	}
	if(!isnull(column_where2) && row_identifier2 != -1 && isnull(row_identifier3))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%d'", column_where2, row_identifier2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(column_where2) && !isnull(row_identifier3) && row_identifier2 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%s'", column_where2, row_identifier3);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}

stock SQL::SetStringEntryEx(const table[], const field[], const value[], const column_where[] = "", const row_identifier[] = "", bool:use_real_escape = true, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	if(use_real_escape == true)
	{
		//Not using mysql_escape_string as the %e specifier is faster.
		mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%e' WHERE `%s`='%e' ", table, field, value, column_where, row_identifier);
	}
	else
	{
		mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%s' WHERE `%s`='%e' ", table, field, value, column_where, row_identifier);
	}
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}

stock SQL::SetStringEntryEx2(const table[], const field[], const value[], const column_where[] = "", const row_identifier[] = "", const column_where2[] = "", const row_identifier2[] = "", row_identifier3 = -1, bool:use_real_escape = true, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	if(use_real_escape == true)
	{
		//Not using mysql_escape_string as the %e specifier is faster.
 		format(SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%e' WHERE `%s`='%e'", table, field, value, column_where, row_identifier);
	}
	else
	{
  		format(SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%s' WHERE `%s`='%e'", table, field, value, column_where, row_identifier);
	}
	if(!isnull(column_where2) && !isnull(row_identifier2) && row_identifier3 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%e'", column_where2, row_identifier2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(column_where2) && row_identifier3 != -1 && isnull(row_identifier2))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%d'", column_where2, row_identifier3);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}

stock SQL::SetFloatEntry(const table[], const field[], Float:value, const column_where[] = "", row_identifier = -1, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	format(SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%f' WHERE `%s`='%d' ", table, field, value, column_where, row_identifier);
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}

stock SQL::SetFloatEntry2(const table[], const field[], Float:value, const column_where[] = "", row_identifier = -1, const column_where2[] = "", row_identifier2 = -1, const row_identifier3[] = "", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	format(SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%f' WHERE `%s`='%d'", table, field, value, column_where, row_identifier);
	if(!isnull(column_where2) && row_identifier2 != -1 && isnull(row_identifier3))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%d'", column_where2, row_identifier2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(column_where2) && !isnull(row_identifier3) && row_identifier2 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%s'", column_where2, row_identifier3);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}

stock SQL::SetFloatEntryEx(const table[], const field[], Float:value, const column_where[] = "", const row_identifier[] = "", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%f' WHERE `%s`='%e' ", table, field, value, column_where, row_identifier);
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}

stock SQL::SetFloatEntryEx2(const table[], const field[], Float:value, const column_where[] = "", const row_identifier[] = "", const column_where2[] = "", const row_identifier2[] = "", row_identifier3 = -1, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	format(SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%f' WHERE `%s`='%e'", table, field, value, column_where, row_identifier);
	if(!isnull(column_where2) && !isnull(row_identifier2) && row_identifier3 == -1)
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%e'", column_where2, row_identifier2);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	if(!isnull(column_where2) && row_identifier3 != -1 && isnull(row_identifier2))
	{
		format(SQL::upd_form2, sizeof(SQL::upd_form2), " AND `%s`='%d'", column_where2, row_identifier3);
		strcat(SQL::upd_form, SQL::upd_form2);
	}
	strcat(SQL::upd_form, " ");
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}


stock SQL::TableExists(const tablename[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new query[SQL_FORM_LENGTH], Cache:result, rows;
	mysql_format(connectionHandle, query, sizeof(query),"SELECT * FROM `%s` ", tablename);
	result = mysql_query(connectionHandle, query);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", query);
	}
	cache_get_row_count(rows);
	cache_delete(result);
	if(rows > 0)
	{
		return 1;
	}
	return 0;
}

stock SQL::ExistsTable(const tablename[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new query[85], Cache:result, rows;
	format(query, sizeof(query), "SHOW TABLES LIKE '%s' ", tablename);
	result = mysql_query(connectionHandle, query);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", query);
	}
	cache_get_row_count(rows);
	cache_delete(result);
	if(rows > 0)
	{
		return 1;
	}
	return 0;
}

stock SQL::CountRows(const tablename[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new query[SQL_FORM_LENGTH], Cache:result, rows;
	format(query, sizeof(query), "SELECT * FROM `%s` ", tablename);
	result = mysql_query(connectionHandle, query);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", query);
	}
	cache_get_row_count(rows);
	cache_delete(result);
	return rows;
}

stock SQL::RowExistsEx(const table[], const column_where[] = "", const row_identifier[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new query[SQL_FORM_LENGTH], Cache:result, rows;
	mysql_format(connectionHandle, query, sizeof(query),"SELECT * FROM `%s` WHERE `%s`='%e' ", table, column_where, row_identifier);
	result = mysql_query(connectionHandle, query);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", query);
	}
	cache_get_row_count(rows);
	cache_delete(result);
	if(rows > 0)
	{
		return rows;
	}
	return 0;
}

stock SQL::RowExistsEx2(const table[], const column_where[] = "", const row_identifier[], const column_where2[] = "", const row_identifier2[] = "", row_identifier3 = -1, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new query[SQL_FORM_LENGTH], query2[SQL_FORM_LENGTH], Cache:result, rows;
	format(query, sizeof(query), "SELECT * FROM `%s` WHERE `%s`='%e'", table, column_where, row_identifier);
	if(!isnull(column_where2) && !isnull(row_identifier2) && row_identifier3 == -1)
	{
		format(query2, sizeof(query2), " AND `%s`='%e'", column_where2, row_identifier2);
		strcat(query, query2);
	}
	if(!isnull(column_where2) && row_identifier3 != -1 && isnull(row_identifier2))
	{
		format(query2, sizeof(query2), " AND `%s`='%d'", column_where2, row_identifier3);
		strcat(query, query2);
	}
	strcat(query, " ");
	result = mysql_query(connectionHandle, query);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", query);
	}
	cache_get_row_count(rows);
	cache_delete(result);
	if(rows > 0)
	{
		return rows;
	}
	return 0;
}

stock SQL::DropTable(const table[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new query[SQL_FORM_LENGTH];
	format(query, sizeof(query), "DROP TABLE %s;", table);
	mysql_tquery(connectionHandle, query, "", "");
	return 0;
}

stock SQL::RowExists(const table[], const column_where[] = "", row_identifier = -1, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new query[SQL_FORM_LENGTH], Cache:result, rows;
	mysql_format(connectionHandle, query, sizeof(query),"SELECT * FROM `%s` WHERE `%s`='%d' ", table, column_where, row_identifier);
	result = mysql_query(connectionHandle, query);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", query);
	}
	cache_get_row_count(rows);
	cache_delete(result);
	if(rows > 0)
	{
		return rows;
	}
	return 0;
}

stock SQL::RowExists2(const table[], const column_where[] = "", row_identifier = -1, const column_where2[] = "", row_identifier2 = -1, const row_identifier3[] = "", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new query[SQL_FORM_LENGTH], query2[SQL_FORM_LENGTH], Cache:result, rows;
	format(query, sizeof(query), "SELECT * FROM `%s` WHERE `%s`='%d'", table, column_where, row_identifier);
	if(!isnull(column_where2) && row_identifier2 != -1 && isnull(row_identifier3))
	{
		format(query2, sizeof(query2), " AND `%s`='%d'", column_where2, row_identifier2);
		strcat(query, query2);
	}
	if(!isnull(column_where2) && !isnull(row_identifier3) && row_identifier2 == -1)
	{
		format(query2, sizeof(query2), " AND `%s`='%s'", column_where2, row_identifier3);
		strcat(query, query2);
	}
	strcat(query, " ");
	result = mysql_query(connectionHandle, query);
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", query);
	}
	cache_get_row_count(rows);
	cache_delete(result);
	if(rows > 0)
	{
		return rows;
	}
	return 0;
}

stock SQL::CountTables(MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new Cache:result, rows;
	result = mysql_query(connectionHandle, "SHOW TABLES");
	if(mysql_errno() != 0)
	{
		return SQL_Warning("Query could not be completed due to error: %s", "SHOW TABLES");
	}
	cache_get_row_count(rows);
	cache_delete(result);
	return rows;
}

stock SQL::AddForeignKey(const o_table[], const f_table[], const o_key[], const f_key[], const db[], SQL::ftypes:on_delete, SQL::ftypes:on_update, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new query[256];
	format(query, sizeof(query), "ALTER TABLE `%s` ADD FOREIGN KEY (`%s`) REFERENCES `%s`.`%s`(`%s`) ON DELETE %s ON UPDATE %s;",
	o_table, o_key, db, f_table, f_key, SQL::RetFType(on_delete), SQL::RetFType(on_update));
	mysql_tquery(connectionHandle, query, "", "");
	return 0;
}

#undef SQL_INVALID_HANDLE
#undef SQL_MAX_TABLE_NAME
#undef SQL_MAX_HANDLES
#undef SQL_Warning
#undef SQL_Error
