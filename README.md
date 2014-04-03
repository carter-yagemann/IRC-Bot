IRC Bot
=======

Description
-----------
The goal of this project is to make a simple IRC bot library using C++. Users of this library should be 
able to quickly and easily make their own custom IRC bots in C++.

Status
------
This project is currently in its barebone stage. Bots can connect to a server and sit in a chatroom, but 
not much else. This project is currently private, but I plan to open it up to the public if more 
functionality is implimented.

Dependencies
------------
This project currently only utilizes standard C++ libraries and should have good portability across 
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

The following is an example of how to make a IRC bot and have it join a room:

    #include "ircbot.h"
    
    using namespace std;
    
    int main() {
        IrcBot bot = IrcBot("NewBot", "New Bot");
        bot.start("irc.freenode.net", "6667");
        
        bot.joinRoom("#roomName");
        
        while (bot.isConnected()) {
            bot.recieveData();
            
            /* Handle Data Here! */
        }
        
        return 0;
    }

Bot Commands
============

connectToServer(host, port)
---------------------------
Opens up a connection with the given host on a given port.

Example: `bot.connectToServer("irc.freenode.net", "6667");`

joinChannel(channel)
--------------------
Joins the given channel.

Example: `bot.joinChannel("#news");`

recieveData()
-------------
Recieves data from the server and stores it in a buffer. Other methods can then parse this buffer and react 
accordingly. This method will also automatically send PONG responses to the server's PINGs which is 
necessary to prevent the server from disconnecting with the bot. For this reason, this method should be 
called frequently.

Example: `bot.recieveData();`

searchData(search_string)
-------------------------
Searches the last recieved data for the given string. This search is performed on the raw server message 
and is case sensitive. Returns true if the string is found.

Example: `bot.searchData("Tell me a joke, bot.");`

isConnected()
-------------
Returns true if the bot is currently connected to a server.

Example: `bot.isConnected()`

isAuth()
--------
Returns true if the bot is currently authenticated (has a nickname and real name) and is therefore ready to 
join channels and send messages.

Example: `bot.isAuth()`
