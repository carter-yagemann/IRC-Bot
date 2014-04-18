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
#include <ctype.h>
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

#define MAXDATASIZE 512


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

}

/*
 * IrcBot Destructor
 */
IrcBot::~IrcBot() {

  close (s);

  // Free all dynamically allocated memory
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
  changeNick(nick);
  sendUser();
  auth = true;
}

/*
 * Searches a buffer for a string
 */
bool IrcBot::searchData(char* search_str, bool case_sensitive) {

  if (case_sensitive) {

    if (strstr(recv_buffer, search_str) != NULL)
      return true;

  } else {
    // We need to lowercase both strings so the search is case-insensitive
    int i;
    char* lower_buffer = (char*) calloc(strlen(recv_buffer) + 1, sizeof(char));
    for(i = 0; i < strlen(recv_buffer); i++)
      lower_buffer[i] = tolower(recv_buffer[i]);

    char* lower_search = (char*) calloc(strlen(search_str) + 1, sizeof(char));
    for(i = 0; i < strlen(search_str); i++)
      lower_search[i] = tolower(search_str[i]);

    if (strstr(lower_buffer, lower_search) != NULL)
      return true;

    free(lower_buffer);
    free(lower_search);
  }

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
  char* pong_msg = (char*) calloc(strlen(servername) + 6, sizeof(char));
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
  if (searchData(ping, true))
    sendPong();

  // Check for disconnect
  if (numbytes == 0) {
    cout << "-----CONNECTION CLOSED-----" << endl;
    auth = false;
    connected = false;
    close(s);
  }
}

/*
 * Joins a room
 */
void IrcBot::joinChannel(char* channel) {

  char* join_msg = (char*) calloc(strlen(channel) + 8, sizeof(char));
  strcpy(join_msg, "JOIN ");
  strcat(join_msg, channel);
  strcat(join_msg, "\r\n");

  sendData(join_msg);

  free(join_msg);
}

/*
 * Sends a message to the destination
 */
void IrcBot::sendMsg(char* dest, char* msg) {

  // Craft PRIVMSG
  char* privmsg = (char*) calloc(strlen(dest) + strlen(msg) + 13, sizeof(char));
  strcpy(privmsg, "PRIVMSG ");
  strcat(privmsg, dest);
  strcat(privmsg, " :");
  strcat(privmsg, msg);
  strcat(privmsg, "\r\n");

  // Send PRIVMSG
  sendData(privmsg);

  // Free memory
  free(privmsg);
}

/*
 * Checks if last recieved message was a PRIVMSG
 */
bool IrcBot::recievedMsg() {
  if (strstr(recv_buffer, " PRIVMSG ") != NULL)
    return true;

  return false;
}

/*
 * Parses sender of a PRIVMSG
 */
void IrcBot::getSender(char* buffer, int size){
  //Confirm PRIVMSG is in buffer
  if (recievedMsg() && strstr(recv_buffer, "!~") != NULL) {
    char * i;
    char * end = strstr(recv_buffer, "!~");

    // Name has a : in front of it which we want to skip
    for (i = recv_buffer + 1; i < end && size - 1 > 0; i++) {
      buffer[i - recv_buffer - 1] = *i;
      size--;
    }
    buffer[i - recv_buffer - 1] = '\0';
  }
}

/*
 * Parses the destination of the PRIVMSG
 */
void IrcBot::getDest(char* buffer, int size) {
  char* start = strstr(recv_buffer, "PRIVMSG ");
  if (start == NULL) return;
  start+= 8;

  char* end = strstr(recv_buffer, " :");

  char* i;
  for (i = start; i < end && size - 1 > 0; i++) {
    buffer[i - start] = *i;
    size--;
  }

  buffer[i - start] = '\0';
}

/*
 * Parses sender's message from a PRIVMSG
 */
void IrcBot::getMsg(char* buffer, int size) {
  //Confirm PRIVMSG is in buffer
  if (recievedMsg() && strstr(recv_buffer, "!~") != NULL) {
    int i;

    // Second : marks start of message
    char * start = strchr(recv_buffer + 1, ':');
    start++;

    // Msg has a : in front of it which we want to skip
    for (i = 0; start - recv_buffer + i < strlen(recv_buffer) && i < size - 1; i++)
      buffer[i] = start[i];

    // Remove trailing \r\n if present
    if (buffer[i - 2] == '\r' && buffer[i-1] == '\n') {
      buffer[i - 2] = '\0';
    } else {
      buffer[i] = '\0';
    }
  }
}

/*
 * Generates string representing current time
 */
void IrcBot::timestamp(char* buffer, int size) {
  time_t rawtime;
  struct tm* timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer, size, "%m%d%Y%H%M%S", timeinfo);
}

/*
 * Sets an away message
 */
void IrcBot::setAway(char* msg) {
  // Sending an away with no parameters disables away
  if (msg == NULL) {
    char away_msg[] = "AWAY\r\n\0";
    sendData(away_msg);
  } else {
    char* away_msg = (char*) calloc(strlen(msg) + 9, sizeof(char));
    strcpy(away_msg, "AWAY :");
    strcat(away_msg, msg);
    strcat(away_msg, "\r\n");

    sendData(away_msg);

    free(away_msg);
  }
}

/*
 * Removes away message
 */
void IrcBot::removeAway() {
  // According to the IRC standard, AWAY is disabled by sending
  // an AWAY message with no parameters
  setAway(NULL);
}

/*
 * Leaves a channel
 */
void IrcBot::leaveChannel(char* channel) {
  char* part_msg = (char*) calloc(strlen(channel) + 8, sizeof(char));
  strcpy(part_msg, "PART ");
  strcat(part_msg, channel);
  strcat(part_msg, "\r\n");

  sendData(part_msg);

  free(part_msg);
}

/*
 * Sends server USER information
 */
void IrcBot::sendUser() {
  char* user_msg = (char*) calloc(strlen(usr) + 19, sizeof(char));

  strcpy(user_msg, "USER guest 0 * :");
  strcat(user_msg, usr);
  strcat(user_msg, "\r\n");

  sendData(user_msg);

  free(user_msg);
}

/*
 * Changes the bot's nickname and sends NICK message
 */
void IrcBot::changeNick(char* _nick) {

  char* nick_msg = (char*) calloc(strlen(_nick) + 8, sizeof(char));

  strcpy(nick_msg, "NICK ");
  strcat(nick_msg, _nick);
  strcat(nick_msg, "\r\n");

  sendData(nick_msg);

  free(nick_msg);

  // Update bot's name
  free(nick);
  nick = (char*) calloc(strlen(_nick) + 1, sizeof(char));
  strcpy(nick, _nick);
}

/*
 * Sends an OPER request
 */
void IrcBot::becomeOperator(char* user, char* pass) {

  char* oper_msg = (char*) calloc(strlen(user) + strlen(pass) + 9, sizeof(char));

  strcpy(oper_msg, "OPER ");
  strcat(oper_msg, user);
  strcat(oper_msg, " ");
  strcat(oper_msg, pass);
  strcat(oper_msg, "\r\n");

  sendData(oper_msg);

  free(oper_msg);
}

/*
 * Sends MODE message
 */
void IrcBot::setMode(char* target, char* mode, char* filter){
  // Filter parameter is optional
  int payload_size;
  if (filter == NULL) {
    payload_size = strlen(target) + strlen(mode) + 9;
  } else {
    payload_size = strlen(target) + strlen(mode) + strlen(filter) + 10;
  }

  char* mode_msg = (char*) calloc(payload_size, sizeof(char));

  strcpy(mode_msg, "MODE ");
  strcat(mode_msg, target);
  strcat(mode_msg, " ");
  strcat(mode_msg, mode);
  if (filter != NULL) {
    strcat(mode_msg, " ");
    strcat(mode_msg, filter);
  }
  strcat(mode_msg, "\r\n");

  sendData(mode_msg);

  free(mode_msg);
}

/*
 * Sends TOPIC message to change topic
 */
void IrcBot::setTopic(char* channel, char* msg) {

  char* topic_msg = (char*) calloc(strlen(channel) + strlen(msg) + 11, sizeof(char));

  strcpy(topic_msg, "TOPIC ");
  strcat(topic_msg, channel);
  strcat(topic_msg, " :");
  strcat(topic_msg, msg);
  strcat(topic_msg, "\r\n");

  sendData(topic_msg);

  free(topic_msg);
}

/*
 * Sends KICK message to kick user
 */
void IrcBot::kickUser(char* channel, char* user, char* msg) {
  // msg can be null
  int msg_size;
  if (msg == NULL) {
    msg_size = strlen(channel) + strlen(user) + 9;
  } else {
    msg_size = strlen(channel) + strlen(user) + strlen(msg) + 11;
  }
  char* kick_msg = (char*) calloc(msg_size, sizeof(char));

  strcpy(kick_msg, "KICK ");
  strcat(kick_msg, channel);
  strcat(kick_msg, " ");
  strcat(kick_msg, user);
  if (msg != NULL) {
    strcat(kick_msg, " :");
    strcat(kick_msg, msg);
  }
  strcat(kick_msg, "\r\n");

  sendData(kick_msg);

  free(kick_msg);
}

/*
 * Sends INVITE message to invite a user to a channel
 */
void IrcBot::inviteUser(char* user, char* channel) {

  char* invite_msg = (char*) calloc(strlen(user) + strlen(channel) + 11, sizeof(char));

  strcpy(invite_msg, "INVITE ");
  strcat(invite_msg, user);
  strcat(invite_msg, " ");
  strcat(invite_msg, channel);
  strcat(invite_msg, "\r\n");

  sendData(invite_msg);

  free(invite_msg);
}
