IRC Bot
=======

Description
-----------
The goal of this project is to make a simple IRC bot library using C++. Users of this library should be 
able to quickly and easily make their own custom IRC bots.

Status
------
This project is currently open to the public and is still being further developed.

Dependencies
------------
This project currently only utilizes GNU C++ libraries and should have good portability across 
different architectures.

Compilation
-----------
This project has been tested and confirmed to compile with g++. Compiling should be possible with simple 
commands such as:

`g++ -c ircbot.cpp ircbot.h`

Since this project is only intended to be a library, there is no main method in the code. Users must write 
their own main method. More information is included in the "Getting Started" section.

Getting Started
---------------
This library is intended to be easy and simple to use. All a user has to do is include the header file in 
their main program:

`#include ircbot.h`

And then they are ready to go.

The following is an example of how to make a IRC bot and have it join a channel:

    #include "ircbot.h"
    
    using namespace std;
    
    int main() {
        IrcBot bot = IrcBot("NewBot", "New Bot");
        bot.connectToServer("irc.freenode.net", "6667");
        
        bot.joinChannel("#channelName");
        
        while (bot.isConnected()) {
            bot.recieveData();
            
            /* Handle Data Here! */
        }
        
        return 0;
    }

Bot Commands
============

IrcBot(nickname, username)
--------------------------
This method must be called when creating a new bot object. Nickname is the initial nickname the bot 
will use when it joins the server and username is the bot's "real name" as required according to the IRC 
protocol.

Exampe: `IrcBot bot = IrcBot("NewBot", "New Bot");`

connectToServer(host, port)
---------------------------
Opens up a connection with the given host on a given port.

Example: `bot.connectToServer("irc.freenode.net", "6667");`

joinChannel(channel)
--------------------
Joins the given channel.

Example: `bot.joinChannel("#news");`

leaveChannel(channel)
---------------------
Leaves the given channel.

Example: `bot.leaveChannel("#news");`

changeNick(nickname)
--------------------
Changes the bot's nickname to the new nickname and sends this change to the IRC server.

Example: `bot.changeNick("NewNickname");`

recieveData()
-------------
Recieves data from the server and stores it in a buffer. Other methods can then parse this buffer and react 
accordingly. This method will also automatically send PONG responses to the server's PINGs which is 
necessary to prevent the server from disconnecting with the bot. For this reason, this method should be 
called frequently.

Example: `bot.recieveData();`

searchData(search_string, case_sensitive)
-----------------------------------------
Searches the last recieved data for the given string. This search is performed on the raw server message 
and will be case sensitive if the second parameter is true or case insensitive if the second parameter is 
false. Returns true if the string is found.

Example: `bot.searchData("Tell me a joke, bot.", false);`

recievedMsg()
-------------
Checks if the last data recieved by recieveData() was a message. Returns true if it was. Since this method 
only looks for the PRIVMSG command, it will return true for messages your bot sent. Therefore, you should 
always check who the sender of the message is if you don't want your bot to respond to itself.

Example `bot.recievedMsg();`

getSender(buffer, size)
-----------------------
Confirms that the data last recieved was a message and prints the sender's name into the buffer. Size 
should be the size of the buffer and is used to prevent overflows.

Example: `bot.getSender(username_buffer, 50);`

getDest(buffer, size)
---------------------
Confirms that the data last recieved was a message and prints the destination into the buffer. Size should 
be the size of the buffer and is used to prevent overflows. This is a useful way to figure out which 
channel a message came from or if the message was a direct message to the bot.

Example: `bot.getDest(destination_buffer, 50);`

getMsg(buffer, size)
------------------------
Confirms that the data last recieved was a message and prints the sender's message into the buffer. Size 
should be the size of the buffer and is used to prevent overflows.

Example: `bot.getMsg(message_buffer, 100);`

sendMsg(dest, message)
----------------------
Sends the message to the destination. The destination can be a channel, user or mask.

Example: `bot.sendMsg("Steve", "Hi Steve!");`

Example: `bot.sendMsg("#channel", "Hi everybody!");`

setAway(message)
----------------
Sets the bot's away message to the given message.

Example: `bot.setAway("I am away right now");`

removeAway()
------------
Removes the bot's away message. Since away messages are disabled by sending an AWAY 
command with no parameter, as defined in the IRC protocol, this method just calls 
setAway with a null pointer.

Example: `bot.removeAway();`

isConnected()
-------------
Returns true if the bot is currently connected to a server.

Example: `bot.isConnected();`

isAuth()
--------
Returns true if the bot is currently authenticated (has a nickname and real name) and is therefore ready to 
join channels and send messages.

Example: `bot.isAuth();`

Things To Possibly Impliment
============================
* Impliment logging for channels, PMs, etc
* Impliment MODE command
* Impliment admin commands (KICK, etc)
* Impliment response codes and response code handeling
