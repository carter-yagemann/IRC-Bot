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

> g++ -c ircbot.cpp ircbot.h

Since this project is only intended to be a library, there is no main method in the code. Users must write 
their own main method. More information is included in the "Getting Started" section.

Getting Started
---------------
This library is intended to be easy and simple to use. All a user has to do is include the header file in 
their main program:

> #include ircbot.h

And then they are ready to go.

The following is an example of how to make a IRC bot and have it join a room:

> Coming Soon
