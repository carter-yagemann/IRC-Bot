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

  void connectToServer(char* host, char* port);
  void joinRoom(char* room);
  void recieveData();
  bool charSearch(char* toSearch, char* searchFor);

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
