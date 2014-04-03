/*
 * IRC Bot - A simple C++ IRC library
 * Copyright (C) 2014  Carter Yagemann
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

  // Allocate space for recieve buffer
  recv_buffer = (char*) malloc(MAXDATASIZE);

  // Bot isn't connected to a server yet
  connected = false;
  auth = false;

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

  // Free all dynamically allocated memory
  free(nick_msg);
  free(usr_msg);
  free(nick);
  free(usr);
  free(recv_buffer);
}

/*
 * Connects IRC bot to server
 */
void IrcBot::connectToServer(char* host, char* port) {

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
  connected = true;

  // Free Memory
  freeaddrinfo(servinfo);

  // Recieve three messages as mentioned in IRC protocol
  int count;
  for (count = 0; count < 3; count++)
    recieveData();

  // Send NICK and USER messages to IRC server
  sendData(nick_msg);
  sendData(usr_msg);
  auth = true;
}

/*
 * Searches a buffer for a string
 * (Search is case sensitive)
 */
bool IrcBot::searchData(char* search_str) {

  if (strstr(recv_buffer, search_str) != NULL)
    return true;

  return false;
}

/*
 * Confirms IrcBot is connected
 */
bool IrcBot::isConnected() {
  return connected;
}

/*
 * Confirms IrcBot is authenticated
 */
bool IrcBot::isAuth() {
  return auth;
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

  cout << ">>" << msg;

  if (bytes_sent == 0)
    return false;
  else
    return true;
}


/*
 * Send IRC pong to server (in response to ping)
 */
void IrcBot::sendPong() {

  char* ping = strstr(recv_buffer, "PING ");
  char* servername = ping + 5;

  if (ping == NULL) return;

  // Construct PONG message
  char* pong_msg = (char*) calloc(strlen(servername) + 5, sizeof(char));
  strcpy(pong_msg, "PONG ");
  strcat(pong_msg, servername);

  // Send PONG message
  sendData(pong_msg);

  free(pong_msg);

}

/*
 * Recieves data from server and responds to ping if necessary
 */
void IrcBot::recieveData() {

  int numbytes = recv(s, recv_buffer, MAXDATASIZE - 1, 0);
  recv_buffer[numbytes] = '\0';
  cout << recv_buffer;

  // Check for ping
  char ping[] = "PING";
  if (searchData(ping))
    sendPong();

  // Check for disconnect
  if (numbytes == 0) {
    cout << "-----CONNECTION CLOSED-----" << endl;
    cout << timeNow() << endl;
    auth = false;
    connected = false;
    close(s);
  }
}

/*
 * Joins a room
 */
void IrcBot::joinChannel(char* channel) {

  if (connected && auth) {
    // Create JOIN IRC message
    char* join_msg = (char*) calloc(strlen(channel) + 7, sizeof(char));
    strcpy(join_msg, "JOIN ");
    strcat(join_msg, channel);
    strcat(join_msg, "\r\n");

    sendData(join_msg);

    free(join_msg);
  }
}
