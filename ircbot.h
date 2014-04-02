/*
 * ircbot.h
 *
 * Created..: 4/2/2014
 * By.......: Carter Yagemann
 */

#ifndef IRCBOT_H_
#define IRCBOT_H_

class IrcBot {

public:
  IrcBot(char * _nick, char * _usr);
  virtual ~IrcBot();

  bool setup;

  void start(char* host, char* port, char* chatroom);
  bool charSearch(char *toSearch, char *searchFor);

private:
  char *port;
  int s; //the socket descriptor

  char* nick;
  char* usr;

  char* nick_msg;
  char* usr_msg;

  bool isConnected(char *buf);

  char * timeNow();

  bool sendData(char *msg);

  void sendPong(char *buf);

  void msgHandler(char *buf);
};

#endif /* IRCBOT_H_ */
