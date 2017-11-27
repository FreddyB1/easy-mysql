# Easy - MySQL V3.5

This include allows you to handle MySQL queries in a simplified way. This means that you won't need to write queries for the most part.


## Installation:

In order to use this include, you need the latest version of the MySQL plugin by BlueG, which can be obtained through the following link:

https://github.com/pBlueG/SA-MP-MySQL/releases

You simply need to place the latest release of easy-mysql.inc into your includes folder and include it in your script.

```pawn
#include <easy-mysql>
```

## Functions:

### SQL::Open and SQL::OpenEx

The only difference between those two functions is that SQL::Open takes an integer for argument as row_identifier and SQL::OpenEx a string (could be a player's name).

This function allows to execute a defined type of MySQL command (SQL::qtypes).
The following MySQL instructions are available (SQL::qtypes)
```pawn
SQL::UPDATE
SQL::UPDATE2
SQL::TUPDATE
SQL::CREATE
SQL::INSERT
SQL::READ
SQL::READ2
SQL::TREAD
SQL::CALLBACK
SQL::MREAD
SQL::MREAD2
SQL::MTREAD
```

```pawn
SQL::Open(SQL::qtypes:type, const table[], const column_where[] = "", row_identifier = -1, const column_where2[] = "", row_identifier2 = -1, const row_identifier3[] = "", limit = -1, limit2 = -1, const desc[] = "", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
```

#### Parameters
```pawn
SQL::qtypes:type
```
##### Specifies the MySQL instruction you want to perform
```pawn
const table[]
```
##### Specifies the name of the table the instruction will be executed at.
```pawn
const column_where[] = ""
```
##### If you're using the right type of instruction, specifies the name of the column for which you know the right row identifier value, could be the name of the field of player's database ID for instance. ("db_id) 
```pawn
row_identifier 
```
##### Specifies the row identifier, namely: Execute some MySQL instruction (WHERE column_where = row_identifer), if it's a player then it could be the player's database ID for example.

#### Example

Checking if a table exists and if not creating a new table:
```pawn
//Checking if the table '"samp_users"' exists 
if(!SQL::ExistsTable("samp_users")) 
{ 
    //If not, then create a table called '"samp_users"'. 
    new handle = SQL::Open(SQL::CREATE, "samp_users"); //Opening a valid handle to create the table
    SQL::AddTableEntry(handle, "p_id", SQL_TYPE_INT, 11, true); 
    SQL::AddTableEntry(handle, "p_name", SQL_TYPE_VCHAR, 24, .setindex = true); 
    SQL::AddTableEntry(handle, "p_password", SQL_TYPE_VCHAR, 64); 
    SQL::AddTableEntry(handle, "p_score", SQL_TYPE_INT); 
    SQL::AddTableEntry(handle, "p_posx", SQL_TYPE_FLOAT); 
    SQL::AddTableEntry(handle, "p_posy", SQL_TYPE_FLOAT); 
    SQL::AddTableEntry(handle, "p_posz", SQL_TYPE_FLOAT); 
    SQL::Close(handle);//Closing the previous opened handle. 
} 
```

Inserting data:
```pawn
new handle = SQL::Open(SQL::INSERT, "samp_users"); 
SQL::ToggleAutoIncrement(handle, true);//Toggles auto increment, SQL::Close will return cache_insert_id(); 
SQL::WriteString(handle, "p_name", ret_pName(playerid)); 
SQL::WriteString(handle, "p_password", UserInfo[playerid][p_password]); 
SQL::WriteInt(handle, "p_score", 0); 
SQL::WriteFloat(handle, "p_posx", 0.0); 
SQL::WriteFloat(handle, "p_posy", 0.0); 
SQL::WriteFloat(handle, "p_posz", 0.0); 
SQL::Close(handle);
```

Updating data:
```pawn
new handle = SQL::Open(SQL::UPDATE, "samp_users", "p_id", UserInfo[playerid][p_id]); 
SQL::WriteInt(handle, "p_score", GetPlayerScore(playerid)); 
SQL::WriteFloat(handle, "p_posx", UserInfo[playerid][p_posx]); 
SQL::WriteFloat(handle, "p_posy", UserInfo[playerid][p_posy]); 
SQL::WriteFloat(handle, "p_posz", UserInfo[playerid][p_posz]); 
SQL::Close(handle); 
```
Reading data from a SINGLE row:
```pawn
new handle = SQL::Open(SQL::READ, "samp_users", "p_id", UserInfo[playerid][p_id]); 
SQL::ReadInt(handle, "p_score", UserInfo[playerid][p_score]); 
SQL::ReadFloat(handle, "p_posx", UserInfo[playerid][p_posx]); 
SQL::ReadFloat(handle, "p_posy", UserInfo[playerid][p_posy]); 
SQL::ReadFloat(handle, "p_posz", UserInfo[playerid][p_posz]); 
SQL::Close(handle);
```

Reading all rows in a table:
Example: reading all the rows in a faction system:
```pawn
new handle = SQL::Open(SQL::MTREAD, "samp_factions");
SQL_ReadRetrievedRows(handle, i) //i represents the row number that's being read
{
  SQL::ReadInt(handle, "f_dbID", FactionInfo[i][f_dbID], i);
  SQL::ReadInt(handle, "f_hasLeader", FactionInfo[i][f_hasLeader], i);
  SQL::ReadInt(handle, "f_ranks", FactionInfo[i][f_ranks], i);
  SQL::ReadInt(handle, "f_budget", FactionInfo[i][f_budget], i);
  SQL::ReadInt(handle, "f_color", FactionInfo[i][f_color], i);
  SQL::ReadInt(handle, "f_dbID", FactionInfo[i][f_dbID], i);
  SQL::ReadString(handle, "f_acronym", FactionInfo[i][f_acronym], MAX_FAC_ACRO_LEN, i);
  SQL::ReadString(handle, "f_name", FactionInfo[i][f_name], MAX_FAC_LEN, i);
  SQL::ReadString(handle, "f_leaderName", FactionInfo[i][f_leaderName], MAX_PLAYER_NAME, i);
  for(new l = 0; l < MAX_FAC_RANKS; l++)
  {
    format(strf, sizeof(strf), "f_rankname%d", l);
    SQL::ReadString(handle, "f_leaderName", f_rankname[i][l], MAX_FAC_RANK_LEN, i);
  }
  FactionInfo[i][f_valid] = 1;
}
new rows = SQL::Close(handle); //returns the number of rows
printf("[MYSQL] Factions loaded: %d", rows);
```
