#ifndef LIVESTREAMER_LIVE_STREAMER_H
#define LIVESTREAMER_LIVE_STREAMER_H


#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include "log.h"

class DisplayDeviceSource;
class UsageEnvironment;
class TaskScheduler;
class RTSPServer;
class UserAuthenticationDatabase;
class ServerMediaSubsession;

class LiveStreamer
{
public:
     bool init();

     void dataPushed(char* data,unsigned int dataSize);

     void loop();

     void stop();

     LiveStreamer(unsigned int fps,unsigned int port);

     ~LiveStreamer();
protected:

    void addSession(RTSPServer* rtspServer,
                    const char* sessionName,
                    ServerMediaSubsession *subSession);

private:
    DisplayDeviceSource*		_displaySource;
    UsageEnvironment*			_env;
    TaskScheduler* 				_scheduler;
    RTSPServer* 				_rtspServer;
    UserAuthenticationDatabase* _authDB;


    int queueSize ;
    int fps;
    unsigned short rtpPortNum;
    unsigned short rtcpPortNum;
    unsigned char ttl;
    struct in_addr destinationAddress;
    unsigned short rtspPort;
    unsigned short rtspOverHTTPPort;
    bool multicast;
    bool useMmap;
    std::string url;
    std::string murl;
    bool useThread;
    in_addr_t maddr;
    bool repeatConfig;
    int timeout;

    char quit;
};
#endif //LIVESTREAMER_LIVE_STREAMER_H