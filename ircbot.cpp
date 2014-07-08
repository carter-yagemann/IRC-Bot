/*
 * IRC Bot - A simple C++ IRC library
 * Copyright (c) 2014 Carter Yagemann
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
}

/*
 * Confirms IrcBot is connected
 */
bool IrcBot::isConnected() {
  return connected;
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
  // Construct PONG message
  char* pong_msg = (char*) calloc(strlen(trail) + 6, sizeof(char));
  strcpy(pong_msg, "PONG ");
  strcat(pong_msg, trail);

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

  parseData();

  // Check for ping
  if (strcmp(command, "PING") == 0)
    sendPong();

  // Check for disconnect
  if (numbytes == 0) {
    cout << "-----CONNECTION CLOSED-----" << endl;
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
  if (strcmp(command, "PRIVMSG") == 0)
    return true;

  return false;
}

/*
 * Parses sender of a PRIVMSG
 */
void IrcBot::getSender(char* buffer, int size){

  if (prefix != NULL) {
    strncpy(buffer, prefix, size);
    buffer[size - 1] = '\0';
  } else {
    buffer[0] = '\0';
  }
}

/*
 * Parses the destination of the PRIVMSG
 */
void IrcBot::getDest(char* buffer, int size) {

  if (params != NULL) {
    strncpy(buffer, params, size);
    buffer[size - 1] = '\0';
  } else {
    buffer[0] = '\0';
  }
}

/*
 * Parses sender's message from a PRIVMSG
 */
void IrcBot::getMsg(char* buffer, int size) {

  if (trail != NULL) {
    strncpy(buffer, trail, size);
    // Remove trailing control characters
    if (buffer[size - 1] == '\n' && buffer[size - 2] == '\r') {
      buffer[size - 2] == '\0';
    } else {
      buffer[size - 1] = '\0';
    }
  } else {
    buffer[0] = '\0';
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

/*
 * Parses data into its components
 *
 * IRC messages take the form:
 *   [prefix] command [params][ :trail]
 *
 * Where [*] indicates optional parts. In other words, only the
 * command parameter is neccesary.
 */
void IrcBot::parseData() {
  // Get prefix
  if (recv_buffer[0] == ':') {
    prefix = recv_buffer + 1;
  } else {
    prefix = NULL;
  }

  // Get trail
  trail = strstr(recv_buffer, " :");
  if (trail != NULL) {
    *trail = '\0';
    trail += 2;
  }

  // Get command
  if (prefix != NULL) {
    command = strstr(recv_buffer, " ");
    *command = '\0';
    command++;
  } else {
    command = recv_buffer;
  }

  // Get params
  params = strstr(command, " ");
  if (params != NULL) {
    *params = '\0';
    params++;
  }
}
