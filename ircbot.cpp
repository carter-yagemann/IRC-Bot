/*
 * ircbot.cpp
 *
 * Created..: 4/2/2014
 * By.......: Carter Yagemann
 */

#include "ircbot.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

using namespace std;

#define MAXDATASIZE 1024


/*
 * IrcBot constructor
 */
IrcBot::IrcBot(char * _nick, char * _usr) {

  // Store Nickname and Username
  nick = (char*) calloc(strlen(_nick) + 1, sizeof(char));
  usr  = (char*) calloc(strlen(_usr)  + 1, sizeof(char));

  nick = strcpy(nick, _nick);
  usr  = strcpy(usr, _usr);

  // Create NICK and USER messages
  nick_msg = (char*) calloc(strlen(_nick) + 7, sizeof(char));
  usr_msg  = (char*) calloc(strlen(_usr) + 22, sizeof(char));

  nick_msg = strcpy(nick_msg, "NICK ");
  nick_msg = strcat(nick_msg, _nick);
  nick_msg = strcat(nick_msg, "\r\n");

  usr_msg = strcpy(usr_msg, "USER guest bot bot :");
  usr_msg = strcat(usr_msg, _usr);
  usr_msg = strcat(usr_msg, "\r\n");
}

/*
 * IrcBot Destructor
 */
IrcBot::~IrcBot() {
  close (s);
  free(nick_msg);
  free(usr_msg);
  free(nick);
  free(usr);
}

/*
 * Starts IrcBot
 */
void IrcBot::start(char* host, char* port, char* chatroom) {

  struct addrinfo hints, *servinfo;

  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

  // Setup the structs
  int res;
  if ((res = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
    fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(res));
  }


  // Setup the socket
  if ((s = socket(servinfo->ai_family,servinfo->ai_socktype,servinfo->ai_protocol)) == -1) {
    perror("client: socket");
  }

  // Connect
  if (connect(s,servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
    close(s);
    perror("Client Connect");
  }

  // Free Memory
  freeaddrinfo(servinfo);

  // Create buffer for recieving data
  int numbytes;
  char buf[MAXDATASIZE];

  // Create JOIN IRC message
  char* join_msg = (char*) calloc(strlen(chatroom) + 7, sizeof(char));
  strcpy(join_msg, "JOIN ");
  strcat(join_msg, chatroom);
  strcat(join_msg, "\r\n");

  // String constants
  char ping[] = "PING";

  int count = 0;
  while (1) {
    count++;

    switch (count) {
      case 3:
        // After 3 recives send data to server (as per IRC protocol)
        sendData(nick_msg);
        sendData(usr_msg);
        break;
      case 4:
        // Join a channel after we connect
        sendData(join_msg);
        // No longer need join message
        free(join_msg);
        break;
      default:
        break;
    }



    // Recieve Data
    numbytes = recv(s,buf,MAXDATASIZE-1,0);
    buf[numbytes]='\0';
    cout << buf;

    // Pass buf to the message handler
    msgHandler(buf);


    // If Ping Recived
    /*
     * must reply to ping overwise connection will be closed
     * see http://www.irchelp.org/irchelp/rfc/chapter4.html
     */
    if (charSearch(buf, ping)) {
      sendPong(buf);
    }

    // Connection closed
    if (numbytes == 0) {
      cout << "-----CONNECTION CLOSED-----"<< endl;
      cout << timeNow() << endl;

      break;
    }
  }
}

/*
 * Searches a buffer for a string
 * (Search is case sensitive)
 */
bool IrcBot::charSearch(char* toSearch, char* searchFor) {
  int len = strlen(toSearch);
  int forLen = strlen(searchFor);

  // Search through each char in toSearch
  for (int i = 0; i < len;i++) {
    // If the active char is equal to the first search item then search toSearch
    if (searchFor[0] == toSearch[i]) {
      bool found = true;
      // Search the char array for search field
      for (int x = 1; x < forLen; x++) {
        if (toSearch[i+x] != searchFor[x]) {
          found = false;
        }
      }

      //if found return true;
      if (found == true)
        return true;
    }
  }

  return 0;
}

/*
 * Confirms IrcBot is connected
 */
bool IrcBot::isConnected(char *buf) {
  // If we find /MOTD then its ok join a channel
  char motd[] = "/MOTD";
  if (charSearch(buf, motd) == true)
    return true;
  else
    return false;
}

/*
 * Returns current time
 */
char * IrcBot::timeNow() {
  time_t rawtime;
  struct tm * timeinfo;

  time ( &rawtime );
  timeinfo = localtime ( &rawtime );

  return asctime (timeinfo);
}

/*
 * Sends data to server
 */
bool IrcBot::sendData(char *msg) {

  int len = strlen(msg);
  int bytes_sent = send(s,msg,len,0);

  if (bytes_sent == 0)
    return false;
  else
    return true;
}

/*
 * Send IRC pong to server (in response to ping)
 */
void IrcBot::sendPong(char *buf) {

  char toSearch[] = "PING ";

  for (int i = 0; i < strlen(buf);i++) {
    // If the active char is equil to the first search item then search toSearch
    if (buf[i] == toSearch[0]) {
      bool found = true;
      // Search the char array for search field
      for (int x = 1; x < 4; x++) {
        if (buf[i+x]!=toSearch[x]) {
          found = false;
        }
      }

      if (found == true) {
        int count = 0;
        // Count the chars
        for (int x = (i+strlen(toSearch)); x < strlen(buf);x++)
          count++;

        // Create the new char array
        char returnHost[count + 5];
        returnHost[0]='P';
        returnHost[1]='O';
        returnHost[2]='N';
        returnHost[3]='G';
        returnHost[4]=' ';

        count = 0;
        // Set the hostname data
        for (int x = (i+strlen(toSearch)); x < strlen(buf);x++) {
          returnHost[count+5]=buf[x];
          count++;
        }

        // Send the pong
        if (sendData(returnHost)) {
          cout << timeNow() <<"  Ping Pong" << endl;
        }


        return;
      }
    }
  }
}

/*
 * Message handling method
 */
void IrcBot::msgHandler(char * buf)
{
  // Response to being Mentioned

	/*
	 * TODO: add you code to respod to commands here
	 * the example below replys to the command hi scooby
	 */
	//if (charSearch(buf,"hi scooby"))
	//{
	//	sendData("PRIVMSG #ubuntu :hi, hows it going\r\n");
	//}

}
