#Easy-MySQL

Easy-MySQL is a plugin designed to simply the usage of MySQL queries, specially if you want to run long queries and you're not willing to create a lot of strings for it.

#Download

Go to the [releases page](https://github.com/ThreeKingz/easy-mysql/releases/tag/easy-mysql) and download the latest easy-mysql.inc file available.


#Installation

- Place the easy-mysql.inc file you downloaded in your includes folder.
- Include the easy-mysql in your PAWN script.
```pawn
#include <easy-mysql> 
```
#Requirements:

In order to use this include, you require the [MySQL Plugin](https://github.com/pBlueG/SA-MP-MySQL/releases) by BlueG.

#Functions and their usage:

###SQL::Connect

#####Description: 
      Connects your script to a MySQL database with given parameters.
####Parameters:
```pawn
(const host[], const user[], const database[], const password[], bool:debugging = false, port = 3306, bool:autoreconnect = true, pool_size = 2);´
```
####Return values:
    Connection handle retrieved by mysql_connect.

```pawn
#define mysql_host    "localhost" 
#define mysql_user    "root" 
#define mysql_db        "server" 
#define mysql_pass         "" 
new MySQL_Handle;

public OnGameModeInit() 
{ 
	MySQL_Handle = SQL::Connect(mysql_host, mysql_user, mysql_db, mysql_pass); 
	return 1;
}
```

###SQL::DeleteRow

#####Description: 
      Deletes a row in the specified table.
####Parameters:
```pawn
(const table[], const column[], columnID, connectionHandle = 1);
```
* `const table[]`                 The target MySQL table.
* `const column[]`                The column in the MySQL table.
* `columnID`                      The unique ID that identifies the target row you want to delete.
* `connectionHandle`              (optional)The handle returned by SQL::Connect or mysql_connect.


####Return values:
    Always returns 1.

```pawn
//Delete an user from a table called 'players', supposing the column is called 'p_id' and the player's column ID is 5.
SQL::DeleteRow("players", "p_id", 5);
```


###SQL::DeleteRowEx

#####Description: 
      Deletes a row in the specified table.
####Parameters:
```pawn
(const table[], const column[], columnID[], connectionHandle = 1);
```
* `const table[]`                 The target MySQL table.
* `const column[]`                The column in the MySQL table.
* `columnID[]`                    The string that identifies the target row you want to delete(might be his name?).
* `connectionHandle`              (optional)The handle returned by SQL::Connect or mysql_connect.


####Return values:
    Always returns 1.

```pawn
//Delete an user from a table called 'players', supposing the column is called 'p_name' and the player's name is TestName.
SQL::DeleteRowEx("players", "p_name", "TestName");
```


###SQL::GetIntEntry

#####Description: 
      Gets an integer field in a defined row.
####Parameters:
```pawn
(const table[], const field[], const column[], columnID, connectionHandle = 1);
```
* `const table[]`                 The target MySQL table.
* `const field[]`                 The field to retrieve the integer from.
* `const column[]`                The column that identifies the selected row.
* `columnID`                    The ID in the column that identifies the selected row.
* `connectionHandle`              (optional)The handle returned by SQL::Connect or mysql_connect.


####Return values:
    The retrieved integer, 0 if no integer was found.

```pawn
//Get an user's score from a table called 'players', supposing the score column is called 'p_score', the identifier column is called 'p_id' and the player's id is 4.
new player_score = SQL::GetIntEntry("players", "p_score", "p_id", 4);
```
