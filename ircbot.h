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
  void recieveData();
  bool searchData(char* search_str);

  bool isConnected();
  bool isAuth();

private:

  char* recv_buffer;

  bool connected;
  bool auth;

  char *port;
  int s; // Socket

  char* nick;
  char* usr;

  char* nick_msg;
  char* usr_msg;

  char * timeNow();

  bool sendData(char *msg);

  void sendPong(char *buf);

  void msgHandler(char *buf);
};

#endif /* IRCBOT_H_ */
