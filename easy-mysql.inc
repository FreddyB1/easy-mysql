/*
	Author: (creator)
 	* ThePhenix AKA ThreeKingz

	Credits:
	* BlueG - MySQL
	
	Version: 2.0
	
	(c) Copyright 2015
 	* This file is provided as is (no warranties).

	native SQL::Connect(const host[], const user[], const database[], const password[], bool:debugging = false, port = 3306, bool:autoreconnect = true, pool_size = 2);
	native SQL::DeleteRow(const table[], const column[], columnID, connectionHandle = 1);
	native SQL::DeleteRowEx(const table[], const column[], columnID[], connectionHandle = 1);
	native SQL::GetIntEntry(const table[], const field[], const column[], columnID, connectionHandle = 1);
	native Float:SQL::GetFloatEntry(const table[], const field[], const column[], columnID, connectionHandle = 1);
	native SQL::GetStringEntry(const table[], const field[], const column[], columnID, dest[], len = sizeof(dest), connectionHandle = 1);
	native SQL::GetStringEntryEx(const table[], const field[], const column[], const scolumn[], dest[], len = sizeof(dest), connectionHandle = 1)
	native SQL::GetIntEntryEx(const table[], const field[], const column[], scolumn[], connectionHandle = 1);
	native Float:SQL::GetFloatEntryEx(const table[], const field[], const column[], scolumn[], connectionHandle = 1);
	native SQL::CreateTable(const tablename[], connectionHandle = 1);
	native SQL::AddTableEntry(handle, const field[], SQL::datatypes: type = SQL_TYPE_INT, maxlength = 11, bool:auto_increment = false, bool:setprimary = false);
	native SQL::OpenTable_Read(const table[], const column[], columnID, connectionHandle = 1);
	native SQL::OpenTable_ReadEx(const table[], const column[], columnID[], connectionHandle = 1);
	native SQL::ReadInt(handle, const field[], &dest);
	native SQL::ReadFloat(handle, const field[], &Float:dest);
	native SQL::ReadString(handle, const field[], dest[], len = sizeof(dest));
	native SQL::OpenTable_Update(const table[], const column[], columnID, connectionHandle = 1);
	native SQL::OpenTable_UpdateEx(const table[], const column[], columnID[], connectionHandle = 1);
	native SQL::Open(SQL::qtypes:type, const table[], const column[] = "", columnID = -1, connectionHandle = 1);
	native SQL::ToggleAutoIncrement(handle, bool:toggle);
	native SQL::OpenEx(SQL::qtypes:type, const table[], const column[] = "", columnID[] = "", connectionHandle = 1)
	native SQL::UpdateIntEntry(handle, const field[], value);
	native SQL::UpdateFloatEntry(handle, const field[], Float:value);
	native SQL::UpdateStringEntry(handle, const field[], const value[], bool:use_real_escape = true);
	native SQL::Close(handle);
	native SQL::SetIntEntry(const table[], const field[], value, const column[], columnID, connectionHandle = 1);
	native SQL::SetIntEntryEx(const table[], const field[], value, const column[], columnID[], connectionHandle = 1);
	native SQL::SetFloatEntry(const table[], const field[], Float:value, const column[], columnID, connectionHandle = 1);
	native SQL::SetFloatEntryEx(const table[], const field[], Float:value, const column[], columnID[], connectionHandle = 1);
	native SQL::SetStringEntry(const table[], const field[], const value[], const column[], columnID, bool:use_real_escape = true, connectionHandle = 1);
	native SQL::SetStringEntryEx(const table[], const field[], const value[], const column[], columnID[], bool:use_real_escape = true, connectionHandle = 1);
	native SQL::OpenTable_Insert(const table[], connectionHandle = 1);
	native SQL::InsertIntEntry(handle, const field[], value);
	native SQL::InsertFloatEntry(handle, const field[], Float:value);
	native SQL::InsertStringEntry(handle, const field[], const value[], bool:use_real_escape = true);
	native SQL::ExistsTable(const tablename[], connectionHandle = 1);
	native SQL::CountRows(const tablename[], connectionHandle = 1);
	native SQL::CountTables(connectionHandle = 1);
	native SQL::DropTable(const tablename[], connectionHandle = 1);
	
	v2.0
	
	native SQL::Open(SQL::qtypes:type, const table[], const column[] = "", columnID = -1, connectionHandle = 1);
	native SQL::ToggleAutoIncrement(handle, bool:toggle);
	native SQL::OpenEx(SQL::qtypes:type, const table[], const column[] = "", columnID[] = "", connectionHandle = 1)
	
	
	The following functions replace Insert and Update functions:
	
	SQL::WriteFloat(handle, const field[], Float:value);
	SQL::WriteInt(handle, const field[], Float:value);
	SQL::WriteString(handle, const field[], const value[]);
	
	
	This include is mainly for people without experience with MySQL as it simplifies its usage.
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
#if !defined SQL_MAX_HANDLES
	#define SQL_MAX_HANDLES              (3)//Maximun concurrent handles running.
#endif

#define SQL_MAX_TABLE_NAME 					(64)
#define SQL_MAX_QUERY_LENGTH				(4096)
#define SQL_INVALID_HANDLE					(-1)
#define SQL_FORM_LENGTH						(128)
#define SQL:: 	SQL_
#define SQL_Warning(%0)      		(printf("[MYSQL] - WARNING: " %0))
#define SQL_Error(%0)        		(printf("[MYSQL] - ERROR: " %0))
#define SQL_Notice(%0)				(printf("[MYSQL] - NOTICE: " %0))

enum SQL::datatypes {SQL_TYPE_INT, SQL_TYPE_VCHAR, SQL_TYPE_FLOAT} 
enum SQL::qtypes {SQL::UPDATE, SQL::CREATE, SQL::INSERT, SQL_TYPE_DELETE, SQL::READ}

static stock
	bool:SQL::UsedHandle[SQL_MAX_HANDLES],
	SQL::upd_connectionHandle[SQL_MAX_HANDLES],
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
	bool:SQL::upd_useautoincrement[SQL_MAX_HANDLES],
	SQL::primarykey[SQL_MAX_HANDLES][64],
	SQL::isset_primarykey[SQL_MAX_HANDLES]
	
;

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
static stock SQL::IsValidUpdatingSlot(handle)
{
	return SQL::UsedHandle[handle];
}
stock SQL::Connect(const host[], const user[], const database[], const password[], bool:debugging = false, port = 3306, bool:autoreconnect = true, pool_size = 2)
{
	if(debugging == true) mysql_log(LOG_ERROR | LOG_WARNING | LOG_DEBUG);
	new SQL::mc;
	SQL::mc = mysql_connect(host, user, database, password, port, autoreconnect, pool_size);
	if(mysql_errno(SQL::mc) != 0)
	{
		SQL_Warning("Could not connect to database %s on host %s | user %s and password %s", database, host, user, password);
		return 0;
	
	}
	else
	{
	    SQL_Notice("Successfully connected to database!");
	}
	return SQL::mc;
}
stock SQL::DeleteRow(const table[], const column[], columnID, connectionHandle = 1)
{
	format(SQL::upd_form, sizeof(SQL::upd_form), "DELETE FROM `%s` WHERE `%s`='%d' ", table, column, columnID);
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}
stock SQL::Open(SQL::qtypes:type, const table[], const column[] = "", columnID = -1, connectionHandle = 1)
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
			handle = SQL::OpenTable_Read(table, column, columnID, connectionHandle);
		}
		case SQL::INSERT:
		{
			handle = SQL::OpenTable_Insert(table, false, connectionHandle);
		}
		case SQL::UPDATE:
		{
			handle = SQL::OpenTable_Update(table, column, columnID, connectionHandle);
		}
	}
	return handle;
}
stock SQL::OpenEx(SQL::qtypes:type, const table[], const column[] = "", columnID[] = "", connectionHandle = 1)
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
			handle = SQL::OpenTable_ReadEx(table, column, columnID, connectionHandle);
		}
		case SQL::INSERT:
		{
			handle = SQL::OpenTable_Insert(table, false, connectionHandle);
		}
		case SQL::UPDATE:
		{
			handle = SQL::OpenTable_UpdateEx(table, column, columnID, connectionHandle);
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
stock SQL::DeleteRowEx(const table[], const column[], columnID[], connectionHandle = 1)
{
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form),  "DELETE FROM `%s` WHERE `%s`='%e' ", table, column, columnID);
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}
stock SQL::GetIntEntry(const table[], const field[], const column[], columnID, connectionHandle = 1)
{
	new Cache:result, int;
	format(SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`='%d' ", field, table, column, columnID);
	result = mysql_query(connectionHandle, SQL::upd_form);
	if (result) 
	{
		if (cache_get_row_count() == 1)
		{
			int = cache_get_field_content_int(0, field, connectionHandle);
		}
		else
		{
			int = -1;
		}
		cache_delete(result);
	}
	return int;
}
stock Float:SQL::GetFloatEntry(const table[], const field[], const column[], columnID, connectionHandle = 1)
{
	new 
		Cache:result, 
		Float:int
	;
	format(SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`='%d' ", field, table, column, columnID);
	
	
	result = mysql_query(connectionHandle, SQL::upd_form);
	
	
	if (result) 
	{
		if (cache_get_row_count())
		{
			int = cache_get_field_content_float(0, field, connectionHandle);
		}
		else
		{
			int = 0.0;
		}
		cache_delete(result);
	}
	return int;
}	
stock SQL::GetStringEntry(const table[], const field[], const column[], columnID, dest[], len = sizeof(dest), connectionHandle = 1)
{
	new Cache:result;
	format(SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`='%d' ", field, table, column, columnID);
	result = mysql_query(connectionHandle, SQL::upd_form);
	if (result) 
	{
		if (cache_get_row_count() == 1)
		{
			cache_get_field_content(0, field, SQL::upd_form, connectionHandle, sizeof(SQL::upd_form));
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
stock SQL::GetStringEntryEx(const table[], const field[], const column[], const scolumn[], dest[], len = sizeof(dest), connectionHandle = 1)
{
	new Cache:result;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`='%e' ", field, table, column, scolumn);
	result = mysql_query(connectionHandle, SQL::upd_form);
	if (result) 
	{
		if (cache_get_row_count() == 1)
		{
			cache_get_field_content(0, field, SQL::upd_form, connectionHandle, sizeof(SQL::upd_form));
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

stock SQL::GetIntEntryEx(const table[], const field[], const column[], scolumn[], connectionHandle = 1)
{
	new Cache:result, int;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`='%e' ", field, table, column, scolumn);
	result = mysql_query(connectionHandle, SQL::upd_form);
	if (result) 
	{
		if (cache_get_row_count() == 1)
		{
			int = cache_get_field_content_int(0, field, connectionHandle);
		}
		else
		{
			int = -1;
		}
		cache_delete(result);
	}
	return int;
}
stock Float:SQL::GetFloatEntryEx(const table[], const field[], const column[], scolumn[], connectionHandle = 1)
{
	new Cache:result, Float:int;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`='%e' ", field, table, column, scolumn);
	result = mysql_query(connectionHandle, SQL::upd_form);
	if (result) 
	{
		if (cache_get_row_count() == 1)
		{
			int = cache_get_field_content_int(0, field, connectionHandle);
		}
		else
		{
			int = -1.0;
		}
		cache_delete(result);
	}
	return int;
}
stock SQL::CreateTable(const tablename[], connectionHandle = 1)
{
	if(connectionHandle == 0)
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
stock SQL::AddTableEntry(handle, const field[], SQL::datatypes: type = SQL_TYPE_INT, maxlength = 11, bool:auto_increment = false, bool:setprimary = false)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::AddTableEntry) Invalid handle. Make sure you used (SQL::OpenTable) first.");
		return 0;
	}
	if(SQL::upd_type[handle] != SQL::CREATE) return 0;
	if(SQL::isset_primarykey[handle] && setprimary == true)
	{
		SQL_Error("(SQL::AddTableEntry) A primary field was created already. There can only be one primary field! [Key %s]", field);
		return 0;
	}
	if(auto_increment == true && type == SQL_TYPE_VCHAR)
	{
		SQL_Error("(SQL::AddTableEntry) You can't set auto increment if the type is varchar! [Key %s]", field);
		return 0;
	}
	switch(type)
	{
		case SQL_TYPE_INT:
		{
			if(auto_increment == true && setprimary == false)
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s int AUTO_INCREMENT,", field, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;
				SQL::isset_primarykey[handle] = 1;
				strcpy(SQL::primarykey[handle], field);
			}
			else if(auto_increment == true && setprimary == true)
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s int AUTO_INCREMENT,", field, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;
				SQL::isset_primarykey[handle] = 1;
				strcpy(SQL::primarykey[handle], field);
			}
			else if(setprimary == true)
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s int NOT NULL PRIMARY KEY,", field, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;
				SQL::isset_primarykey[handle] = 2;
				strcpy(SQL::primarykey[handle], field);
			}
			else
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s int(%d),", field, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;
			}
		}
		case SQL_TYPE_VCHAR:
		{
			if(auto_increment == true) return 0;
			if(setprimary == true)
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s varchar(%d) PRIMARY KEY,", field, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;
				SQL::isset_primarykey[handle] = 2;
				strcpy(SQL::primarykey[handle], field);
			}
			else
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s varchar(%d),", field, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;

			}
		}
		case SQL_TYPE_FLOAT:
		{
			if(auto_increment == true) return 0;
			if(setprimary == true)
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s float(%d) PRIMARY KEY,", field, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;
				SQL::isset_primarykey[handle] = 2;
				strcpy(SQL::primarykey[handle], field);
			}
			else
			{
				format(SQL::upd_form, sizeof(SQL::upd_form), "%s float(%d),", field, maxlength);
				strcat(SQL::upd_query[handle], SQL::upd_form);
				SQL::upd_datacount[handle]++;
			}
		}
	}
	return 1;
}
stock SQL::OpenTable_Read(const table[], const column[], columnID, connectionHandle = 1)
{
	if(connectionHandle == 0)
	{
		SQL_Error("(SQL::OpenTable) No active connection.");
		return SQL_INVALID_HANDLE;
	} 
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable) Invalid table length.");
		return SQL_INVALID_HANDLE;
	} 
	if(column[0] == '\0')
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
	SQL::upd_connectionHandle[i] = connectionHandle;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT * FROM `%s` WHERE `%s` = '%d' ", table, column, columnID);
	SQL::ReadCache[i] = mysql_query(connectionHandle, SQL::upd_form);
	SQL::upd_datacount[i] = 0;
	if(cache_is_valid(SQL::ReadCache[i]))
	{
		SQL::UsedHandle[i] = true;
		return i;
	}
	return SQL_INVALID_HANDLE;
}
stock SQL::OpenTable_ReadEx(const table[], const column[], columnID[], connectionHandle = 1)
{
	if(connectionHandle == 0)
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
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "SELECT * FROM `%s` WHERE `%s` = '%s' ", table, column, columnID);
	SQL::ReadCache[i] = mysql_query(connectionHandle, SQL::upd_form);
	SQL::upd_datacount[i] = 0;
	if(cache_is_valid(SQL::ReadCache[i]))
	{
		SQL::UsedHandle[i] = true;
		return i;
	}
	return SQL_INVALID_HANDLE;
}
stock SQL::ReadInt(handle, const field[], &dest)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::ReadInt) Invalid handle. Make sure you opened the table first.");
		return 0;
	}
	if(SQL::upd_type[handle] != SQL::READ) return 0;
	dest = cache_get_field_content_int(0, field, SQL::upd_connectionHandle[handle]);
	SQL::upd_datacount[handle]++;
	return 1;
}
stock SQL::ReadFloat(handle, const field[], &Float:dest)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::ReadInt) Invalid handle. Make sure you opened the table first.");
		return 0;
	}
	if(SQL::upd_type[handle] != SQL::READ) return 0;
	dest = cache_get_field_content_float(0, field, SQL::upd_connectionHandle[handle]);
	SQL::upd_datacount[handle]++;
	return 1;
}
stock SQL::ReadString(handle, const field[], dest[], len = sizeof(dest))
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::ReadInt) Invalid handle. Make sure you opened the table first.");
		return 0;
	}
	if(SQL::upd_type[handle] != SQL::READ) return 0;
	cache_get_field_content(0, field, dest, SQL::upd_connectionHandle[handle], len+1);
	SQL::upd_datacount[handle]++;
	return 1;
}

stock SQL::OpenTable_Update(const table[], const column[], columnID, connectionHandle = 1)
{
	if(connectionHandle == 0)
	{
		SQL_Error("(SQL::OpenTable) No active connection.");
		return SQL_INVALID_HANDLE;
	} 
	if(strlen(table) > SQL_MAX_TABLE_NAME)
	{
		SQL_Error("(SQL::OpenTable) Invalid table length.");
		return SQL_INVALID_HANDLE;
	} 
	if(column[0] == '\0')
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
	SQL::upd_connectionHandle[i] = connectionHandle;
	format(SQL::upd_query[i], SQL_MAX_QUERY_LENGTH, "UPDATE `%s` SET ", SQL::upd_table[i]);
	format(SQL::upd_rowidentifier[i], SQL_FORM_LENGTH, " WHERE `%s` = '%d' ", column, columnID);
	SQL::upd_datacount[i] = 0;
	SQL::UsedHandle[i] = true;
	return i;
}
stock SQL::OpenTable_UpdateEx(const table[], const column[], columnID[], connectionHandle = 1)
{
	if(connectionHandle == 0)
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
	format(SQL::upd_rowidentifier[i], SQL_FORM_LENGTH, " WHERE `%s` = '%s' ", column, columnID);
	SQL::upd_datacount[i] = 0;
	SQL::UsedHandle[i] = true;
	return i;
}
stock SQL::UpdateIntEntry(handle, const field[], value)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::UpdateIntEntry) Invalid handle. Make sure you used (SQL::OpenTable) first.");
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
stock SQL::WriteString(handle, const field[], const value[])
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
			return SQL::UpdateStringEntry(handle, field, value);
		}
		case SQL::INSERT:
		{
			return SQL::InsertStringEntry(handle, field, value);
		}
	}
	return 0;
}
stock SQL::UpdateFloatEntry(handle, const field[], Float:value)
{
	if(!SQL::IsValidUpdatingSlot(handle))
	{
		SQL_Error("(SQL::UpdateFloatEntry) Invalid handle. Make sure you used (SQL::OpenTable) first.");
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
		SQL_Error("(SQL::UpdateStringEntry) Invalid handle. Make sure you used (SQL::OpenTable) first.");
		return 0;
	}
	if(SQL::upd_type[handle] != SQL::UPDATE) return 0;
	if(use_real_escape == true)
	{
		new escape[SQL_FORM_LENGTH];
		mysql_real_escape_string(value, escape, SQL::upd_connectionHandle[handle]);
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
		SQL::upd_connectionHandle[handle] = 0;
		SQL::upd_query[handle][0] = '\0';
		SQL::upd_rowidentifier[handle][0] = '\0';
		SQL::upd_datacount[handle] = 0;
		SQL::UsedHandle[handle] = false;
	}
	else if(SQL::upd_type[handle] == SQL::CREATE)
	{
		new pos = strlen(SQL::upd_query[handle])-1;
		SQL::upd_query[handle][pos] = ' ';
		if(SQL::isset_primarykey[handle] == 1)
		{
			format(SQL::upd_form, 128, ",primary key (%s)", SQL::primarykey[handle]);
			strcat(SQL::upd_query[handle], SQL::upd_form);
		}
		strcat(SQL::upd_query[handle], ");");
		mysql_tquery(SQL::upd_connectionHandle[handle], SQL::upd_query[handle], "", "");
		SQL::upd_table[handle][0] = '\0';
		SQL::upd_connectionHandle[handle] = 0;
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
		strcat(SQL_upd_query[handle], SQL_upd_query_2[handle]);
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
				SQL::upd_connectionHandle[handle] = 0;
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
stock SQL::SetIntEntry(const table[], const field[], value, const column[], columnID, connectionHandle = 1)
{
	if(!connectionHandle)
	{
		SQL_Error("(SQL::SetIntEntry) No active connection.");
		return 0;
	} 
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	format(SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%d' WHERE `%s`='%d' ", table, field, value, column, columnID);
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}
stock SQL::SetIntEntryEx(const table[], const field[], value, const column[], columnID[], connectionHandle = 1)
{
	if(!connectionHandle)
	{
		SQL_Error("(SQL::SetIntEntryEx) No active connection.");
		return 0;
	} 
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%d' WHERE `%s`='%e' ", table, field, value, column, columnID);
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}
stock SQL::SetStringEntry(const table[], const field[], const value[], const column[], columnID, bool:use_real_escape = true, connectionHandle = 1)
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
		mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%e' WHERE `%s`='%d' ", table, field, value, column, columnID);
	}
	else
	{
		format(SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%s' WHERE `%s`='%d' ", table, field, value, column, columnID);
	}
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}
stock SQL::SetStringEntryEx(const table[], const field[], const value[], const column[], columnID[], bool:use_real_escape = true, connectionHandle = 1)
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
		mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%e' WHERE `%s`='%e' ", table, field, value, column, columnID);
	}
	else
	{
		mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%s' WHERE `%s`='%e' ", table, field, value, column, columnID);
	}
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}
stock SQL::SetFloatEntry(const table[], const field[], Float:value, const column[], columnID, connectionHandle = 1)
{
	if(!connectionHandle)
	{
		SQL_Error("(SQL::SetFloatEntry) No active connection.");
		return 0;
	} 
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	format(SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%f' WHERE `%s`='%d' ", table, field, value, column, columnID);
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}
stock SQL::SetFloatEntryEx(const table[], const field[], Float:value, const column[], columnID[], connectionHandle = 1)
{
	if(!connectionHandle)
	{
		SQL_Error("(SQL::SetFloatEntryEx) No active connection.");
		return 0;
	} 
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	mysql_format(connectionHandle, SQL::upd_form, sizeof(SQL::upd_form), "UPDATE `%s` SET `%s`='%f' WHERE `%s`='%e' ", table, field, value, column, columnID);
	mysql_tquery(connectionHandle, SQL::upd_form, "", "");
	return 1;
}
stock SQL::OpenTable_Insert(const table[], bool:use_autoincrement = false, connectionHandle = 1)
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
stock SQL::InsertIntEntry(handle, const field[], value)
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
stock SQL::InsertFloatEntry(handle, const field[], Float:value)
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
stock SQL::InsertStringEntry(handle, const field[], const value[], bool:use_real_escape = true)
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
		mysql_real_escape_string(value, escape, SQL::upd_connectionHandle[handle]);
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
stock SQL::ExistsTable(const tablename[], connectionHandle = 1)
{
	new query[85], Cache:result;
	format(query, sizeof(query), "SHOW TABLES LIKE '%s'", tablename);
	result = mysql_query(connectionHandle, query);
	new rows = cache_get_row_count();
	cache_delete(result);
	if(rows > 0) 
	{
		return 1;
	}
	return 0;
}
stock SQL::CountRows(const tablename[], connectionHandle = 1)
{
	new query[SQL_FORM_LENGTH], Cache:result;
	format(query, sizeof(query), "SELECT * FROM %s", tablename);
	result = mysql_query(connectionHandle, query);
	new rows = cache_get_row_count();
	cache_delete(result);
	return rows;
}
stock SQL::RowExistsEx(const table[], const column[], const columnID[], connectionHandle = 1)
{
	new query[SQL_FORM_LENGTH], Cache:result;
	mysql_format(connectionHandle, query, sizeof(query),"SELECT * FROM `%s` WHERE `%s` = '%e' ", table, column, columnID);
	result = mysql_query(connectionHandle, query);
	new rows = cache_get_row_count();
	cache_delete(result);
	if(rows > 0) 
	{
		return 1;
	}
	return 0;
}
stock SQL::DropTable(const table[], connectionHandle = 1)
{
	new query[SQL_FORM_LENGTH];
	format(query, sizeof(query), "DROP TABLE %s;", table);
	mysql_tquery(connectionHandle, query, "", "");
	return 0;
}
stock SQL::RowExists(const table[], const column[], columnID[], connectionHandle = 1)
{
	new query[SQL_FORM_LENGTH], Cache:result;
	mysql_format(connectionHandle, query, sizeof(query),"SELECT * FROM `%s` WHERE `%s` = '%d' ", table, column, columnID);
	result = mysql_query(connectionHandle, query);
	new rows = cache_get_row_count();
	cache_delete(result);
	if(rows > 0) 
	{
		return 1;
	}
	return 0;
}
stock SQL::CountTables(connectionHandle = 1)
{
	new Cache:result;
	result = mysql_query(connectionHandle, "SHOW TABLES");
	new rows = cache_get_row_count();
	cache_delete(result);
	return rows;
}

#undef SQL_INVALID_HANDLE
#undef SQL_MAX_TABLE_NAME
#undef SQL_MAX_HANDLES
#undef SQL_Warning
#undef SQL_Error
