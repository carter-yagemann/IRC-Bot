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

#ifndef IRCBOT_H_
#define IRCBOT_H_

class IrcBot {

public:
  IrcBot(char * _nick, char * _usr);
  virtual ~IrcBot();

  void connectToServer(char* host, char* port);
  void joinChannel(char* channel);
  void leaveChannel(char* channel);

  void changeNick(char* _nick);

  void recieveData();

  void sendMsg(char* dest, char* msg);

  void setAway(char* msg);
  void removeAway();

  void setMode(char* target, char* mode, char* filter);

  void becomeOperator(char* user, char* pass);

  void setTopic(char* channel, char* msg);

  void kickUser(char* channel, char* user, char* msg);
  void inviteUser(char* user, char* channel);

  bool recievedMsg();
  void getSender(char* buffer, int size);
  void getDest(char* buffer, int size);
  void getMsg(char* buffer, int size);

  bool isConnected();

private:

  void timestamp(char* buffer, int size);

  char* recv_buffer;

  bool connected;

  char *port;
  int s; // Socket

  char* nick;
  char* usr;

  // For parsing messages
  char* prefix;
  char* command;
  char* params;
  char* trail;

  void sendUser();

  bool sendData(char *msg);

  void parseData();

  void sendPong();
};

#endif /* IRCBOT_H_ */
