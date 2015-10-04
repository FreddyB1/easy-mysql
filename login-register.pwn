
/* 
 *  Simple register/login system using easy_mysql.inc! 
*/ 
#include <a_samp> 
#include <easy-mysql> 

main() 
{ 
     
} 
#define mysql_host    "localhost" 
#define mysql_user    "root" 
#define mysql_db        "server" 
#define mysql_pass         "" 
#define mysql_debugging_enabled             (true) 

#define DIALOG_LOGIN 0 
#define DIALOG_REGISTER 1 

enum p_info 
{ 
    p_id, 
    p_name[24], 
    p_password[64], 
    p_score, 
    Float:p_posx, 
    Float:p_posy, 
    Float:p_posz, 
    p_loggedin 
}; 
new UserInfo[MAX_PLAYERS][p_info]; 
     
stock ret_pName(playerid) 
{ 
    new name[24]; 
    GetPlayerName(playerid, name, sizeof(name)); 
    return name; 
} 
public OnGameModeInit() 
{ 
    //Connecting to the MySQL database 
    SQL::Connect(mysql_host, mysql_user, mysql_db, mysql_pass); 
    //Checking if the table 'players' exists 
     
    //Checking if the table 'players' exists 
    if(!SQL::ExistsTable("players")) 
    { 
        //If not, then create a table called 'players'. 
        new handle = SQL::Open(SQL::CREATE, "players"); //Opening a valid handle to create a table called 'players' 
        SQL::AddTableEntry(handle, "p_id", SQL_TYPE_INT, 11, true); 
        SQL::AddTableEntry(handle, "p_name", SQL_TYPE_VCHAR, 24); 
        SQL::AddTableEntry(handle, "p_password", SQL_TYPE_VCHAR, 64); 
        SQL::AddTableEntry(handle, "p_score", SQL_TYPE_INT); 
        SQL::AddTableEntry(handle, "p_posx", SQL_TYPE_FLOAT); 
        SQL::AddTableEntry(handle, "p_posy", SQL_TYPE_FLOAT); 
        SQL::AddTableEntry(handle, "p_posz", SQL_TYPE_FLOAT); 
        SQL::Close(handle);//Closing the previous opened handle. 
    } 
    return 1; 
} 
public OnPlayerConnect(playerid) 
{ 
    UserInfo[playerid][p_loggedin] = 0; UserInfo[playerid][p_score] = 0;  UserInfo[playerid][p_posx] = 1958.3783; 
    UserInfo[playerid][p_posy] = 1343.1572; UserInfo[playerid][p_posz] = 15.3746;  
    if(SQL::RowExistsEx("players", "p_name", ret_pName(playerid))) //Check if the name is registered in the database 
    { 
        //Get the player password and unique ID. 
        new handle = SQL::OpenEx(SQL::READ, "players", "p_name", ret_pName(playerid)); 
        SQL::ReadString(handle, "p_password", UserInfo[playerid][p_password], 64); 
        SQL::ReadInt(handle, "p_id", UserInfo[playerid][p_id]); 
        SQL::Close(handle); 
        //Show the login dialog 
        ShowPlayerDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_PASSWORD, "{0080FF}Login", "Please input your password below to log in.", "Login", "Exit"); 
    } 
    else 
    { 
        //If not registered, then show the register DIALOG. 
        ShowPlayerDialog(playerid, DIALOG_REGISTER, DIALOG_STYLE_PASSWORD, "{0080FF}Register", "Please input a password below to register in.", "Login", "Exit"); 
    } 
    return 1; 
} 
public OnPlayerSpawn(playerid) 
{ 
    SetPlayerPos(playerid, UserInfo[playerid][p_posx], UserInfo[playerid][p_posy], UserInfo[playerid][p_posz]); 
    return 1; 
} 
public OnPlayerDisconnect(playerid, reason) 
{ 
    if(UserInfo[playerid][p_loggedin] == 1) 
    { 
        //Save the player data. 
        GetPlayerPos(playerid, UserInfo[playerid][p_posx], UserInfo[playerid][p_posy], UserInfo[playerid][p_posz]); 
        new handle = SQL::Open(SQL::UPDATE, "players", "p_id", UserInfo[playerid][p_id]); 
        SQL::WriteInt(handle, "p_score", GetPlayerScore(playerid)); 
        SQL::WriteFloat(handle, "p_posx", UserInfo[playerid][p_posx]); 
        SQL::WriteFloat(handle, "p_posy", UserInfo[playerid][p_posy]); 
        SQL::WriteFloat(handle, "p_posz", UserInfo[playerid][p_posz]); 
        SQL::Close(handle); 
    } 
    return 1; 
} 
public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[]) 
{ 
    switch(dialogid) 
    { 
        case DIALOG_REGISTER: 
        { 
            if(!response) return Kick(playerid); 
            if(strlen(inputtext) < 5) 
            { 
                ShowPlayerDialog(playerid, DIALOG_REGISTER, DIALOG_STYLE_PASSWORD, "{0080FF}Register", "Please input a password below to register in.", "Login", "Exit"); 
                return 1; 
            } 
            SHA256_PassHash(inputtext, "", UserInfo[playerid][p_password], 64); 
            new handle = SQL::Open(SQL::INSERT, "players"); 
            SQL::ToggleAutoIncrement(handle, true);//Toggles auto increment, SQL::Close will return cache_insert_id(); 
            SQL::WriteString(handle, "p_name", ret_pName(playerid)); 
            SQL::WriteString(handle, "p_password", UserInfo[playerid][p_password]); 
            SQL::WriteInt(handle, "p_score", 0); 
            SQL::WriteFloat(handle, "p_posx", 0.0); 
            SQL::WriteFloat(handle, "p_posy", 0.0); 
            SQL::WriteFloat(handle, "p_posz", 0.0); 
            UserInfo[playerid][p_id] = SQL::Close(handle);  
            SendClientMessage(playerid, -1, "Successfully registered in!"); 
            UserInfo[playerid][p_loggedin] = 1; 
        } 
        case DIALOG_LOGIN: 
        { 
            if(!response) Kick(playerid);  
            new hash[64]; 
            SHA256_PassHash(inputtext, "", hash, 64); 
            if(!strcmp(hash, UserInfo[playerid][p_password])) 
            {  
                //Load player data 
                new handle = SQL::Open(SQL::READ, "players", "p_id", UserInfo[playerid][p_id]); 
                SQL::ReadInt(handle, "p_score", UserInfo[playerid][p_score]); 
                SQL::ReadFloat(handle, "p_posx", UserInfo[playerid][p_posx]); 
                SQL::ReadFloat(handle, "p_posy", UserInfo[playerid][p_posy]); 
                SQL::ReadFloat(handle, "p_posz", UserInfo[playerid][p_posz]); 
                SQL::Close(handle);//You must close the handle. 
                SetPlayerScore(playerid, UserInfo[playerid][p_score]); 
                UserInfo[playerid][p_loggedin] = 1; 
                SendClientMessage(playerid, -1, "Successfully logged in!"); 
                 
            } 
            else  
            { 
                ShowPlayerDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_PASSWORD, "{0080FF}Login", "Please input your password below to log in.", "Login", "Exit"); 
            } 
        } 
    } 
    return 1; 
}  
