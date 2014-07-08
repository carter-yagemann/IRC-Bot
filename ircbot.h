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
