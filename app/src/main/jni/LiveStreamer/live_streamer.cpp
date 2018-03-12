#include "live_streamer.h"
#include "DisplayDeviceSource.h"

#include <GroupsockHelper.hh>
#include <BasicUsageEnvironment.hh>

#include "H264_DisplayDeviceSource.h"
#include "ServerMediaSubsession.h"

LiveStreamer::LiveStreamer(unsigned int fps, unsigned int port) {
    queueSize = 40;
    fps = 25;
    rtpPortNum = 20000;
    rtcpPortNum = rtpPortNum+1;
    ttl = 5;
    rtspPort = 8554;
    rtspOverHTTPPort = 0;
    multicast = false;
    useMmap = true;
    url = "papan01";
    murl = "papan01";
    useThread = true;
    maddr = INADDR_NONE;
    repeatConfig = true;
    timeout = 65;
    _authDB = NULL;
    quit = 0;
}


LiveStreamer::~LiveStreamer(){

}

void LiveStreamer::addSession(RTSPServer* rtspServer, const char* sessionName, ServerMediaSubsession *subSession)
{
    UsageEnvironment& env(rtspServer->envir());
    ServerMediaSession* sms = ServerMediaSession::createNew(env, sessionName);
    sms->addSubsession(subSession);
    rtspServer->addServerMediaSession(sms);

    char* url = rtspServer->rtspURL(sms);
    LOGI("Play this stream using the URL : %s",url);
    delete[] url;
}


bool LiveStreamer::init(){
    _scheduler 		= BasicTaskScheduler::createNew();
    _env 			= BasicUsageEnvironment::createNew(*_scheduler);
    _rtspServer 	= RTSPServer::createNew(*_env, rtspPort, _authDB, timeout);

    if(_rtspServer == NULL)
    {
        LOGI("create rtsp server failed");
        return false;
    }
    else
    {
        // if (rtspOverHTTPPort)
        // {
        // 	_rtspServer->setUpTunnelingOverHTTP(rtspOverHTTPPort);
        // }
        _displaySource = H264_DisplayDeviceSource::createNew(*_env, queueSize, useThread, repeatConfig);
        if(_displaySource == NULL)
        {
            LOGI("unable to create source for device");
            return false;
        }
        else
        {
            OutPacketBuffer::maxSize = 600000;//DisplayDeviceSource::bufferedSize;
            StreamReplicator* replicator = StreamReplicator::createNew(*_env, _displaySource, false);
            if (multicast)
            {
                if (maddr == INADDR_NONE) maddr = chooseRandomIPv4SSMAddress(*_env);
                destinationAddress.s_addr = maddr;
                LOGI("RTP  address :%s:%d", inet_ntoa(destinationAddress),rtpPortNum);
                LOGI("RTCP address :%s:%d", inet_ntoa(destinationAddress),rtcpPortNum);
                addSession(_rtspServer, murl.c_str(), MulticastServerMediaSubsession::createNew(*_env,destinationAddress,
                                                                                                Port(rtpPortNum), Port(rtcpPortNum), ttl, replicator));
            }
            addSession(_rtspServer, url.c_str(), UnicastServerMediaSubsession::createNew(*_env,replicator));
        }
    }
    return true;
}

// This function will block current thread
void LiveStreamer::loop()
{
    LOGI("START LOOP");
    _env->taskScheduler().doEventLoop(&quit);
    LOGI("END LOOP");
    Medium::close(_displaySource);
    _env->reclaim();
    delete _scheduler;
}


void LiveStreamer::dataPushed(char* data,unsigned int dataSize)
{
    LOGI("push  raw data\t dataSize:%d",dataSize);
    _displaySource->pushRawData(data,dataSize);
}

void LiveStreamer::stop()
{
    LOGI("STOP....");
    quit = 1;
}
