/*

Copyright [2017] [ThreeKingz - Freddy Borja]

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


//Updated to MySQL R41-4 by madinnat0r - BlueG


//Establish connection
native MySQL:SQL::Connect(const host[], const user[], const database[], const password[], bool:debugging = false, port = 3306, bool:autoreconnect = false, pool_size = 0);
native SQL::Disconnect(MySQL:handle = MYSQL_DEFAULT_HANDLE);


//Open a new type of handle
//qtypes {SQL::UPDATE, SQL::CREATE, SQL::INSERT, SQL_TYPE_DELETE, SQL::READ}
native SQL::Open(SQL::qtypes:type, const table[], const rowid_column[] = "", rowID = -1, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
native SQL::OpenEx(SQL::qtypes:type, const table[], const rowid_column[] = "", rowID[] = "", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
native SQL::ToggleAutoIncrement(handle, bool:toggle);


//Getting a single entry from a single row - don't use this if you want to read data from multiple rows

native SQL::GetIntEntry(const table[], const field[], const rowid_column[], rowID, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
native Float:SQL::GetFloatEntry(const table[], const field[], const rowid_column[], rowID, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE);
native SQL::GetStringEntry(const table[], const field[], const rowid_column[], rowID, dest[], len = sizeof(dest), MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE);

native SQL::GetIntEntryEx(const table[], const field[], const rowid_column[], sRowID[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE);
native Float:SQL::GetFloatEntryEx(const table[], const field[], const rowid_column[], sRowID[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE);
native SQL::GetStringEntryEx(const table[], const field[], const rowid_column[], const scolumn[], dest[], len = sizeof(dest), MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE);

//updates the value of a specific field for a single row: - don't use this for multiple rows
native SQL::SetIntEntry(const table[], const field[], value, const rowid_column[], rowID, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
native SQL::SetStringEntry(const table[], const field[], const string[], const rowid_column[], rowID, bool:use_real_escape = true, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE);
native SQL::SetFloatEntry(const table[], const field[], Float:value, const rowid_column[], rowID, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE);

native SQL::SetFloatEntryEx(const table[], const field[], Float:value, const rowid_column[], rowID[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE);
stock SQL::SetIntEntryEx(const table[], const field[], value, const rowid_column[], rowID[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
stock SQL::SetStringEntryEx(const table[], const field[], const string[], const rowid_column[], rowID[], bool:use_real_escape = true, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)

//adds a new column to a table
native SQL::AddTableColumn(handle, const column_name[], SQL::datatypes: type = SQL_TYPE_INT, maxlength = 11, bool:auto_increment = false, bool:setprimary = false, bool:setindex = false);

//read data, SQL::Open or SQL::OpenEx must return a valid handle
native SQL::ReadInt(handle, const field[], &dest);
native SQL::ReadFloat(handle, const field[], &Float:dest);
native SQL::ReadString(handle, const field[], dest[], len = sizeof(dest));
//"write" data (insert and update)
native SQL::WriteInt(handle, const field[], value);
native SQL::WriteFloat(handle, const field[], Float:value);
native SQL::WriteString(handle, const field[], const value[]);

//Closes any opened handle
native SQL::Close(handle);


//Table functions

native bool:SQL::ExistsTable(const tablename[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE);
native SQL::CountRows(const tablename[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE);
native SQL::AddForeignKey(const o_table[], const f_table[], const o_key[], const f_key[], const db[], SQL::ftypes:on_delete, SQL::ftypes:on_update, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE);
native SQL::CountTables(MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE);
native SQL::AddIndex(const table[], const key[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE);
native SQL::DropTable(const table[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE);

//Check if a row exists
native bool:SQL::RowExistsEx(const table[], const rowid_column[], const rowID[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE);
native bool:SQL::RowExists(const table[], const rowid_column[], rowID, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE);

//delete a row
native SQL::DeleteRow(const table[], const rowid_column[], rowID, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE);
native SQL::DeleteRowEx(const table[], const column[], rowID[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE);
native SQL::DeleteRowExArgs(MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE, const table[], const rowid_column[], type = 'i', {Float,_}:...);


//Transactions
native SQL::Begin(MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE);
native SQL::Commit(MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE);

*/

#if defined _tksql_included
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



#if !defined SQL_MAX_HANDLES
	#define SQL_MAX_HANDLES              	(3)//Maximun concurrent handles running.
#endif
#define SQL_MAX_TABLE_NAME 					(64)
#define SQL_MAX_INDEXES						(4)
#define SQL_MAX_QUERY_LENGTH				(8192)
#define SQL_INVALID_HANDLE					(-1)
#define SQL_FORM_LENGTH						(128)
#define SQL:: 								SQL_T //class like syntax
#define SQL_Warning(%0)      		(printf("[easy-mysql] - WARNING: " %0))
#define SQL_Error(%0)        		(printf("[easy-mysql] - ERROR: " %0))
#define SQL_Notice(%0)				(printf("[easy-mysql] - NOTICE: " %0))



/*==============================================================================
Internal variables and functions
================================================================================*/

enum SQL::datatypes {SQL_TYPE_INT, SQL_TYPE_VCHAR, SQL_TYPE_FLOAT} 
enum SQL::qtypes {SQL::UPDATE, SQL::CREATE, SQL::INSERT, SQL::READ} //query commands
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
	SQL::upd_datacount[SQL_MAX_HANDLES],
	SQL::qtypes:SQL::upd_type[SQL_MAX_HANDLES],
	SQL::upd_pos[SQL_MAX_HANDLES],
	Cache:SQL::ReadCache[SQL_MAX_HANDLES],
	Cache:SQL::ReadAllCache,
	bool:SQL::upd_useautoincrement[SQL_MAX_HANDLES],
	SQL::primarykey[SQL_MAX_HANDLES][64],
	SQL::index[SQL_MAX_HANDLES][SQL_MAX_INDEXES][64],
	SQL::index_set[SQL_MAX_HANDLES][SQL_MAX_INDEXES],
	SQL::isset_primarykey[SQL_MAX_HANDLES]
	
;


//Internal functions:
static stock SQL::GetFreeUpdatingSlot()
{
	new i = 0;
	while (i < sizeof (SQL::UsedHandle) && SQL::UsedHandle[i] == true)
	{
		i++;
	}
	if (i == sizeof (SQL::UsedHandle)) return SQL_INVALID_HANDLE;
	return i;
}

static stock SQL::GetFreeIndexSlot(handle)
{
	new i = 0;
	while (i < sizeof (SQL::index_set) && SQL::index_set[handle][i] == 1)
	{
		i++;
	}
	if (i == sizeof (SQL::index_set)) return SQL_INVALID_HANDLE;
	return i;
}

static stock SQL::IsValidUpdatingSlot(handle)
{
	return SQL::UsedHandle[handle];

}
static stock SQL::CreateTable(const tablename[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE) //used internally
{
	if(connectionHandle == MYSQL_INVALID_HANDLE)
	{
		SQL_Error("(SQL::CreateTable) No active connection.");
		return SQL_INVALID_HANDLE;
	} 
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
	strcpy(SQL::upd_table[i], tablename);
	SQL::upd_connectionHandle[i] = connectionHandle;
	format(SQL::upd_query[i], SQL_MAX_QUERY_LENGTH, "CREATE TABLE %s (", SQL::upd_table[i]);
	SQL::upd_datacount[i] = 0;
	SQL::isset_primarykey[i] = 0;
	SQL::UsedHandle[i] = true;
	return i;
}
static stock SQL::OpenTable_Read(const table[], const column[], rowID, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(connectionHandle == MYSQL_INVALID_HANDLE)
	{
		SQL_Error("(SQL::OpenTable_Read) No active connection.");
		return SQL_INVALID_HANDLE;
	} 
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable_Read) Invalid table length.");
		return SQL_INVALID_HANDLE;
	} 
	if(column[0] == '\0')
	{
		SQL_Error("(SQL::OpenTable_Read) No format has been entered. Format example: (pID)");
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
	SQL::upd_connectionHandle[i] = connectionHandle;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT * FROM `%s` WHERE `%s` = '%d' ", table, column, rowID);
	SQL::ReadCache[i] = mysql_query(connectionHandle, SQL::upd_form);
	SQL::upd_datacount[i] = 0;
	if(cache_is_valid(SQL::ReadCache[i]))
	{
		SQL::UsedHandle[i] = true;
		return i;
	}
	return SQL_INVALID_HANDLE;
}
static stock SQL::OpenTable_ReadEx(const table[], const column[], rowID[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(connectionHandle == MYSQL_INVALID_HANDLE)
	{
		SQL_Error("(SQL::OpenTable_ReadEx) No active connection.");
		return SQL_INVALID_HANDLE;
	} 
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable_ReadEx) Invalid table length.");
		return SQL_INVALID_HANDLE;
	} 
	if(column[0] == '\0')
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
	SQL::upd_connectionHandle[i] = connectionHandle;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT * FROM `%s` WHERE `%s` = '%s' ", table, column, rowID);
	SQL::ReadCache[i] = mysql_query(connectionHandle, SQL::upd_form);
	SQL::upd_datacount[i] = 0;
	if(cache_is_valid(SQL::ReadCache[i]))
	{
		SQL::UsedHandle[i] = true;
		return i;
	}
	return SQL_INVALID_HANDLE;
}

static stock SQL::OpenTable_Update(const table[], const column[], rowID, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(connectionHandle == MYSQL_INVALID_HANDLE)
	{
		SQL_Error("(SQL::OpenTable_Update) No active connection.");
		return SQL_INVALID_HANDLE;
	} 
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable_Update) Invalid table length.");
		return SQL_INVALID_HANDLE;
	} 
	if(column[0] == '\0')
	{
		SQL_Error("(SQL::OpenTable_Update) No format has been entered. Format example: (pID)");
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
	SQL::upd_connectionHandle[i] = connectionHandle;
	format(SQL::upd_query[i], SQL_MAX_QUERY_LENGTH, "UPDATE `%s` SET ", SQL::upd_table[i]);
	format(SQL::upd_rowidentifier[i], SQL_FORM_LENGTH, " WHERE `%s` = '%d' ", column, rowID);
	SQL::upd_datacount[i] = 0;
	SQL::UsedHandle[i] = true;
	return i;
}
static stock SQL::OpenTable_UpdateEx(const table[], const column[], rowID[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(connectionHandle == MYSQL_INVALID_HANDLE)
	{
		SQL_Error("(SQL::OpenTable_UpdateEx) No active connection.");
		return SQL_INVALID_HANDLE;
	} 
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable_UpdateEx) Invalid table length.");
		return SQL_INVALID_HANDLE;
	} 
	if(column[0] == '\0')
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
	SQL::upd_connectionHandle[i] = connectionHandle;
	format(SQL::upd_query[i], SQL_MAX_QUERY_LENGTH, "UPDATE `%s` SET ", SQL::upd_table[i]);
	format(SQL::upd_rowidentifier[i], SQL_FORM_LENGTH, " WHERE `%s` = '%s' ", column, rowID);
	SQL::upd_datacount[i] = 0;
	SQL::UsedHandle[i] = true;
	return i;
}


/*==============================================================================
Actual functions
================================================================================*/
stock SQL::cache_get_value_name_int(row_idx, const column_name[])
{
	new int = 0;
	cache_get_value_name_int(row_idx, column_name, int);
	return int;
}
stock Float:SQL::cache_get_value_name_float(row_idx, const column_name[])
{
	new Float:fval = 0;
	cache_get_value_name_float(row_idx, column_name, fval);
	return fval;
}
stock MySQL:SQL::Connect(const host[], const user[], const database[], const password[], bool:debugging = false, port = 3306, bool:autoreconnect = false, pool_size = 0)
{
	if(debugging == true) mysql_log(ERROR | DEBUG);
	new MySQL:SQL::mc;
	new MySQLOpt:options = mysql_init_options();
	mysql_set_option(options, AUTO_RECONNECT, autoreconnect);
	mysql_set_option(options, POOL_SIZE, pool_size);
	mysql_set_option(options, SERVER_PORT, port);
	SQL::mc = mysql_connect(host, user, password, database, options);
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
	return mysql_close(handle;
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
stock SQL::DeleteRow(const table[], const rowid_column[], rowID, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	format(SQL::upd_form, sizeof(SQL::upd_form), "DELETE FROM `%s` WHERE `%s`='%d' ", table, rowid_column, rowID);
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}
stock SQL::DeleteRowEx(const table[], const rowid_column[], rowID[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form),  "DELETE FROM `%s` WHERE `%s`='%e' ", table, rowid_column, rowID);
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}
stock SQL::DeleteRowExArgs(MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE, const table[], const rowid_column[], type = 'i', {Float,_}:...)
{
	switch (type)
	{
		case 'd', 'i': mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form),  "DELETE FROM `%s` WHERE `%s`='%d' ", table, column, getarg(4));
		case 's':
		{
			new i_ArgLoop = -1;
			new i_Extract[128];
			while (++i_ArgLoop != 0x100)
			{
				if (i_ArgLoop < 128)
				{
					i_Extract[i_ArgLoop] = getarg(4, i_ArgLoop);
				}
			}
			mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form),  "DELETE FROM `%s` WHERE `%s`='%e' ", table, rowid_column, i_Extract);
		}
		case 'f':  mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form),  "DELETE FROM `%s` WHERE `%s`='%f' ", table, rowid_column, getarg(4));
	}
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}
stock SQL::Open(SQL::qtypes:type, const table[], const rowid_column[] = "", rowID = -1, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
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
			handle = SQL::OpenTable_Read(table, rowid_column, rowID, connectionHandle);
		}
		case SQL::INSERT:
		{
			handle = SQL::OpenTable_Insert(table, false, connectionHandle);
		}
		case SQL::UPDATE:
		{
			handle = SQL::OpenTable_Update(table, rowid_column, rowID, connectionHandle);
		}
	}
	return handle;
}
stock SQL::OpenEx(SQL::qtypes:type, const table[], const rowid_column[] = "", rowID[] = "", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
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
			handle = SQL::OpenTable_ReadEx(table, rowid_column, rowID, connectionHandle);
		}
		case SQL::INSERT:
		{
			handle = SQL::OpenTable_Insert(table, false, connectionHandle);
		}
		case SQL::UPDATE:
		{
			handle = SQL::OpenTable_UpdateEx(table, rowid_column, rowID, connectionHandle);
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
stock SQL::GetIntEntry(const table[], const field[], const rowid_column[], rowID, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new Cache:result, int = 0;
	format(SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`='%d' ", field, table, rowid_column, rowID);
	result = mysql_query(connectionHandle, SQL::upd_form);
	if(!cache_is_valid(result)) return SQL_Error("(SQL::GetIntEntry) Invalid cache ID.");
	if (result) 
	{
		if (cache_num_rows()) int = SQL::cache_get_value_name_int(0, field);
		cache_delete(result);
	}
	return int;
}
stock Float:SQL::GetFloatEntry(const table[], const field[], const rowid_column[], rowID, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new 
		Cache:result, 
		Float:int
	;
	format(SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`='%d' ", field, table, rowid_column, rowID);
	
	
	result = mysql_query(connectionHandle, SQL::upd_form);
	if(!cache_is_valid(result)) return SQL_Error("(SQL::GetFloatEntry) Invalid cache ID.");
	if (cache_num_rows())
	{
		int = SQL::cache_get_value_name_float(0, field);
	}
	else
	{
		int = 0.0;
	}
	cache_delete(result);
	return int;
}	
stock SQL::GetStringEntry(const table[], const field[], const rowid_column[], rowID, dest[], len = sizeof(dest), MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new Cache:result;
	format(SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`='%d' ", field, table, rowid_column, rowID);
	result = mysql_query(connectionHandle, SQL::upd_form);
	if (result) 
	{
		if (cache_num_rows() == 1)
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
stock SQL::GetStringEntryEx(const table[], const field[], const rowid_column[], const scolumn[], dest[], len = sizeof(dest), MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new Cache:result;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`='%e' ", field, table, rowid_column, scolumn);
	result = mysql_query(connectionHandle, SQL::upd_form);
	if (result) 
	{
		if (cache_num_rows() == 1)
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

stock SQL::GetIntEntryEx(const table[], const field[], const rowid_column[], sRowID[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new Cache:result, int;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`='%e' ", field, table, rowid_column, sRowID);
	result = mysql_query(connectionHandle, SQL::upd_form);
	if (result) 
	{
		if (cache_num_rows() == 1)
		{
			int = SQL::cache_get_value_name_int(0, field);
		}
		else
		{
			int = -1;
		}
		cache_delete(result);
	}
	return int;
}
stock Float:SQL::GetFloatEntryEx(const table[], const field[], const rowid_column[], sRowID[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new Cache:result, Float:int;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`='%e' ", field, table, rowid_column, sRowID);
	result = mysql_query(connectionHandle, SQL::upd_form);
	if (result) 
	{
		if (cache_num_rows() == 1)
		{
			int = SQL::cache_get_value_name_int(0, field);
		}
		else
		{
			int = -1.0;
		}
		cache_delete(result);
	}
	return int;
}

stock SQL::AddTableColumn(handle, const column_name[], SQL::datatypes: type = SQL_TYPE_INT, maxlength = 11, bool:auto_increment = false, bool:setprimary = false, bool:setindex = false)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::AddTableColumn) Invalid handle. Make sure you used (SQL::Open) first.");
		return 0;
	}
	if(SQL::upd_type[handle] != SQL::CREATE) return 0;
	if(SQL::isset_primarykey[handle] && setprimary == true)
	{
		SQL_Error("(SQL::AddTableColumn) A primary column was created already. There can only be one primary column! [Key %s]", column_name);
		return 0;
	}
	if(auto_increment == true && type == SQL_TYPE_VCHAR)
	{
		SQL_Error("(SQL::AddTableColumn) You can't set auto increment if the type is varchar! [Key %s]", column_name);
		return 0;
	}
	switch(type)
	{
		case SQL_TYPE_INT:
		{
			if(auto_increment == true && setprimary == false)
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s int NOT NULL AUTO_INCREMENT,", column_name, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;
				SQL::isset_primarykey[handle] = 1;
				strcpy(SQL::primarykey[handle], column_name);
			}
			else if(auto_increment == true && setprimary == true)
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s int NOT NULL AUTO_INCREMENT,", column_name, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;
				SQL::isset_primarykey[handle] = 1;
				strcpy(SQL::primarykey[handle], column_name);
			}
			else if(setprimary == true)
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s int NOT NULL PRIMARY KEY,", column_name, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;
				SQL::isset_primarykey[handle] = 2;
				strcpy(SQL::primarykey[handle], column_name);
			}
			else
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s int(%d) NOT NULL,", column_name, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;
			}
		}
		case SQL_TYPE_VCHAR:
		{
			if(auto_increment == true) return 0;
			if(setprimary == true)
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s varchar(%d) NOT NULL PRIMARY KEY,", column_name, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;
				SQL::isset_primarykey[handle] = 2;
				strcpy(SQL::primarykey[handle], column_name);
			}
			else
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s varchar(%d) NOT NULL,", column_name, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;

			}
		}
		case SQL_TYPE_FLOAT:
		{
			if(auto_increment == true) return 0;
			if(setprimary == true)
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s float(%d) NOT NULL PRIMARY KEY,", column_name, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;
				SQL::isset_primarykey[handle] = 2;
				strcpy(SQL::primarykey[handle], column_name);
			}
			else
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s float(%d) NOT NULL,", column_name, maxlength);
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
		strcpy(SQL::index[handle][indexslot], column_name);
	}
	return 1;
}

stock SQL::ReadFloat(handle, const field[], &Float:dest)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::ReadFloat) Invalid handle. Make sure you opened the table first.");
		return 0;
	}
	if(SQL::upd_type[handle] != SQL::READ) return 0;
	dest = SQL::cache_get_value_name_float(0, field);
	SQL::upd_datacount[handle]++;
	return 1;
}

stock SQL::ReadString(handle, const field[], dest[], len = sizeof(dest))
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::ReadString) Invalid handle. Make sure you opened the table first.");
		return 0;
	}
	if(SQL::upd_type[handle] != SQL::READ) return 0;
	cache_get_value_name(0, field, dest, len+1);
	SQL::upd_datacount[handle]++;
	return 1;
}

stock SQL::ReadInt(handle, const field[], &dest)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::ReadInt) Invalid handle. Make sure you opened the table first.");
		return 0;
	}
	if(SQL::upd_type[handle] != SQL::READ) return 0;
	dest = SQL::cache_get_value_name_int(0, field);
	SQL::upd_datacount[handle]++;
	return 1;
}

stock SQL::UpdateIntEntry(handle, const field[], value)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::UpdateIntEntry) Invalid handle. Make sure you used (SQL::Open) first.");
		return 0;
	}
	if(SQL::upd_type[handle] != SQL::UPDATE) return 0;
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
		case SQL::INSERT:
		{
			return SQL::InsertFloatEntry(handle, field, value);
		}
	}
	return 0;
}
stock SQL::WriteString(handle, const field[], const value[], bool:use_real_escape = true)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::WriteString) Invalid handle. Make sure you used (SQL::Open) first.");
		return 0;
	}
	switch(SQL::upd_type[handle])
	{
		case SQL::UPDATE:
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
stock SQL::UpdateFloatEntry(handle, const field[], Float:value)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::UpdateFloatEntry) Invalid handle. Make sure you used (SQL::Open) first.");
		return 0;
	}
	if(SQL::upd_type[handle] != SQL::UPDATE) return 0;
	format(SQL::upd_form, sizeof(SQL::upd_form), "`%s`='%f',", field, value);
	strcat(SQL::upd_query[handle], SQL::upd_form);
	SQL::upd_datacount[handle]++;
	return 1;
}
stock SQL::UpdateStringEntry(handle, const field[], const value[], bool:use_real_escape = true)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::UpdateStringEntry) Invalid handle. Make sure you used (SQL::Open) first.");
		return 0;
	}
	if(SQL::upd_type[handle] != SQL::UPDATE) return 0;
	if(use_real_escape == true)
	{
		new escape[SQL_FORM_LENGTH];
		mysql_escape_string(value, escape, sizeof(escape), SQL::upd_connectionHandle[handle]);
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
stock SQL::Close(handle)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::Close) Invalid handle. Make sure you opened a handle first.");
		return 0;
	}
	if(!SQL::upd_datacount[handle])
	{
		SQL_Error("(SQL::Close) No data has been updated/inserted/deleted.");
		return 0;
	}
	if(SQL::upd_type[handle] == SQL::UPDATE)
	{
		//Remove last comma 
		new pos = strlen(SQL::upd_query[handle])-1;
		SQL::upd_query[handle][pos] = ' ';
		strcat(SQL::upd_query[handle], SQL::upd_rowidentifier[handle]);

		mysql_tquery(SQL::upd_connectionHandle[handle], SQL::upd_query[handle], "", "");
		SQL::upd_table[handle][0] = '\0';
		SQL::upd_connectionHandle[handle] = MYSQL_INVALID_HANDLE;
		SQL::upd_query[handle][0] = '\0';
		SQL::upd_rowidentifier[handle][0] = '\0';
		SQL::upd_datacount[handle] = 0;
		SQL::UsedHandle[handle] = false;
	}
	else if(SQL::upd_type[handle] == SQL::CREATE)
	{
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
		mysql_tquery(SQL::upd_connectionHandle[handle], SQL::upd_query[handle], "", "");
		SQL::upd_table[handle][0] = '\0';
		SQL::upd_connectionHandle[handle] = MYSQL_INVALID_HANDLE;
		SQL::primarykey[handle][0] = '\0';
		SQL::isset_primarykey[handle] = false;
		SQL::upd_query[handle][0] = '\0';
		SQL::upd_rowidentifier[handle][0] = '\0';
		SQL::upd_datacount[handle] = 0;
		SQL::UsedHandle[handle] = false;
		
	}
	else if(SQL::upd_type[handle] == SQL::INSERT)
	{
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
			result = mysql_query(SQL::upd_connectionHandle[handle], SQL::upd_query[handle]);
			if(result)
			{
				new id = cache_insert_id();
				cache_delete(result);
				SQL::upd_connectionHandle[handle] = MYSQL_INVALID_HANDLE;
				SQL::upd_query[handle][0] = '\0';
				SQL::UsedHandle[handle] = false;
				return id;
			}
		}
		else
		{
			mysql_tquery(SQL::upd_connectionHandle[handle], SQL::upd_query[handle], "", "");
			SQL::UsedHandle[handle] = false;
		}
		
	}
	else if(SQL::upd_type[handle] == SQL::READ)
	{
		if(cache_is_valid(SQL::ReadCache[handle]))
		{
			cache_delete(SQL::ReadCache[handle]);
		}
		SQL::UsedHandle[handle] = false;
	}
	return -1;
}
stock SQL::SetIntEntry(const table[], const field[], value, const rowid_column[], rowID, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(!connectionHandle)
	{
		SQL_Error("(SQL::SetIntEntry) No active connection.");
		return 0;
	} 
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	format(SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%d' WHERE `%s`='%d' ", table, field, value, rowid_column, rowID);
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}
stock SQL::SetIntEntryEx(const table[], const field[], value, const rowid_column[], rowID[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(!connectionHandle)
	{
		SQL_Error("(SQL::SetIntEntryEx) No active connection.");
		return 0;
	} 
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%d' WHERE `%s`='%e' ", table, field, value, rowid_column, rowID);
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}
stock SQL::SetStringEntry(const table[], const field[], const value[], const rowid_column[], rowID, bool:use_real_escape = true, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(!connectionHandle)
	{
		SQL_Error("(SQL::SetStringEntry) No active connection.");
		return 0;
	} 
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	if(use_real_escape == true)
	{
		//Not using mysql_real_escape_string as the %e specifier is faster.
		mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%e' WHERE `%s`='%d' ", table, field, value, rowid_column, rowID);
	}
	else
	{
		format(SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%s' WHERE `%s`='%d' ", table, field, value, rowid_column, rowID);
	}
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}
stock SQL::SetStringEntryEx(const table[], const field[], const value[], const rowid_column[], rowID[], bool:use_real_escape = true, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(!connectionHandle)
	{
		SQL_Error("(SQL::SetStringEntryEx) No active connection.");
		return 0;
	} 
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	if(use_real_escape == true)
	{
		//Not using mysql_real_escape_string as the %e specifier is faster.
		mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%e' WHERE `%s`='%e' ", table, field, value, rowid_column, rowID);
	}
	else
	{
		mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%s' WHERE `%s`='%e' ", table, field, value, rowid_column, rowID);
	}
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}
stock SQL::SetFloatEntry(const table[], const field[], Float:value, const rowid_column[], rowID, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(!connectionHandle)
	{
		SQL_Error("(SQL::SetFloatEntry) No active connection.");
		return 0;
	} 
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	format(SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%f' WHERE `%s`='%d' ", table, field, value, rowid_column, rowID);
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}
stock SQL::SetFloatEntryEx(const table[], const field[], Float:value, const rowid_column[], rowID[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(!connectionHandle)
	{
		SQL_Error("(SQL::SetFloatEntryEx) No active connection.");
		return 0;
	} 
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%f' WHERE `%s`='%e' ", table, field, value, rowid_column, rowID);
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}
static stock SQL::OpenTable_Insert(const table[], bool:use_autoincrement = false, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	if(!connectionHandle)
	{
		SQL_Error("(SQL::OpenTable_Insert) No active connection.");
		return SQL_INVALID_HANDLE;
	} 
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
	SQL::upd_connectionHandle[i] = connectionHandle;
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
	SQL::UsedHandle[i] = true;
	return i;
}
static stock SQL::InsertIntEntry(handle, const field[], value)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::InsertIntEntry) Invalid handle. Make sure you used (SQL::Open) first.");
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
		SQL_Error("(SQL::InsertFloatEntry) Invalid handle. Make sure you used (SQL::Open) first.");
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
		SQL_Error("(SQL::InsertStringEntry) Invalid handle. Make sure you used (SQL::Open) first.");
		return 0;
	}
	if(SQL::upd_type[handle] != SQL::INSERT) return 0;
	format(SQL::upd_form, sizeof(SQL::upd_form), "`%s`,", field);
	strcat(SQL::upd_query[handle], SQL::upd_form);
	if(use_real_escape == true)
	{
		new escape[SQL_FORM_LENGTH];
		mysql_escape_string(value, escape, sizeof(escape), SQL::upd_connectionHandle[handle]);
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

stock bool:SQL::ExistsTable(const tablename[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new query[85], Cache:result;
	format(query, sizeof(query), "SHOW TABLES LIKE '%s'", tablename);
	result = mysql_query(connectionHandle, query);
	new rows = cache_num_rows();
	cache_delete(result);
	if(rows > 0) 
	{
		return true;
	}
	return false;
}
stock SQL::CountRows(const tablename[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new query[SQL_FORM_LENGTH], Cache:result;
	format(query, sizeof(query), "SELECT * FROM %s", tablename);
	result = mysql_query(connectionHandle, query);
	if(!cache_is_valid(result)) return SQL_Error("(SQL::CountRows) Invalid cache ID.");
	new rows = cache_num_rows();
	cache_delete(result);
	return rows;
}
stock SQL::RowExistsEx(const table[], const rowid_column[], const rowID[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new query[SQL_FORM_LENGTH], Cache:result;
	mysql_format(connectionHandle, query, sizeof(query),"SELECT * FROM `%s` WHERE `%s` = '%e' ", table, rowid_column, rowID);
	result = mysql_query(connectionHandle, query);
	if(!cache_is_valid(result)) return SQL_Error("(SQL::RowExistsEx) Invalid cache ID.");
	new rows = cache_num_rows();
	cache_delete(result);
	if(rows > 0) 
	{
		return 1;
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
stock SQL::AddIndex(const table[], const key[], MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new query[SQL_FORM_LENGTH];
	format(query, sizeof(query), "ALTER TABLE `%s` ADD INDEX(`%s`);", table, key);
	mysql_tquery(connectionHandle, query, "", "");
	return 0;
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
stock SQL::AddForeignKey(const o_table[], const f_table[], const o_key[], const f_key[], const db[], SQL::ftypes:on_delete, SQL::ftypes:on_update, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new query[256];
	format(query, sizeof(query), "ALTER TABLE `%s` ADD FOREIGN KEY (`%s`) REFERENCES `%s`.`%s`(`%s`) ON DELETE %s ON UPDATE %s;", 
	o_table, o_key, db, f_table, f_key, SQL::RetFType(on_delete), SQL::RetFType(on_update));
	mysql_tquery(connectionHandle, query, "", "");
	return 0;
}

stock SQL::RowExists(const table[], const rowid_column[], rowID, MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new query[SQL_FORM_LENGTH], Cache:result;
	mysql_format(connectionHandle, query, sizeof(query),"SELECT * FROM `%s` WHERE `%s` = '%d' ", table, rowid_column, rowID);
	result = mysql_query(connectionHandle, query);
	if(!cache_is_valid(result)) return SQL_Error("(SQL::RowExists) Invalid cache ID.");
	new rows = cache_num_rows();
	cache_delete(result);
	if(rows > 0) 
	{
		return 1;
	}
	return 0;
}
stock SQL::CountTables(MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
{
	new Cache:result;
	result = mysql_query(connectionHandle, "SHOW TABLES");
	if(!cache_is_valid(result)) return SQL_Error("(SQL::CountTables) Invalid cache ID.");
	new rows = cache_num_rows();
	cache_delete(result);
	return rows;
}


//SQL::CacheReadAll(row, table[], MySQL:handle);
#define CacheReadAll(%0,%1,%2); { new __str[128]; mysql_format(%2, __str, sizeof(__str), "SELECT * FROM %s", %1); \
	new Cache:__cache = mysql_query(%2, __str); \
	if(cache_is_valid(__cache)) { new __rows = cache_num_rows(); if(__rows) { for(new %0; %0 != __rows; %0++) { 

#define CacheCloseReadAll(); } cache_delete(__cache);}}}


//SQL::CacheReadInt(row, column[], dest);
stock SQL::CacheReadInt(row, column[], &dest)
{
	dest = SQL::cache_get_value_name_int(row, column);
	return 1;
}
//SQL::CacheReadInt(row, column[], dest);
stock SQL::CacheReadFloat(row, column[], &Float:dest)
{
	dest = SQL::cache_get_value_name_float(row, column);
	return 1;
}
stock SQL::CacheReadString(row, const column[], dest[], len = sizeof(dest))
{
	return cache_get_value_name(row, column, dest, len);
}



#undef SQL_INVALID_HANDLE
#undef SQL_MAX_TABLE_NAME
#undef SQL_MAX_HANDLES
#undef SQL_Warning
#undef SQL_Error
