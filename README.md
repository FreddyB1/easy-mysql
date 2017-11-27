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

### SQL::Open
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
SQL::Open(SQL::qtypes:type, const table[], const column[] = "", row_identifier = -1, const column2[] = "", columnID2 = -1, const columnID3[] = "", limit = -1, limit2 = -1, const desc[] = "", MySQL:connectionHandle = MYSQL_DEFAULT_HANDLE)
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
const column[] = ""
```
##### If you're using the right type of instruction, specifies the name of the column for which you know the right row identifier value (row 
```pawn
row_identifier 
```
##### Specifies the row identifier, namely: Execute some MySQL instruction (WHERE column = row_identifer)
