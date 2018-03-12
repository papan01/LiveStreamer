#ifndef LIVESTREAMER_SERVERMEDIASUBSESSION_H
#define LIVESTREAMER_SERVERMEDIASUBSESSION_H

#include <string>
#include <iomanip>

// live555
#include <liveMedia.hh>

// forward declaration
class DisplayDeviceSource;

// ---------------------------------
//   BaseServerMediaSubsession
// ---------------------------------
class BaseServerMediaSubsession
{
public:
    BaseServerMediaSubsession(StreamReplicator* replicator): m_replicator(replicator) {};

public:
    static FramedSource* createSource(UsageEnvironment& env, FramedSource * videoES);
    static RTPSink* createSink(UsageEnvironment& env, Groupsock * rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic);
    char const* getAuxLine(DisplayDeviceSource* source,unsigned char rtpPayloadType);

protected:
    StreamReplicator* m_replicator;
};

// -----------------------------------------
//    ServerMediaSubsession for Multicast
// -----------------------------------------
class MulticastServerMediaSubsession : public PassiveServerMediaSubsession , public BaseServerMediaSubsession
{
public:
    static MulticastServerMediaSubsession* createNew(UsageEnvironment& env
            , struct in_addr destinationAddress
            , Port rtpPortNum, Port rtcpPortNum
            , int ttl
            , StreamReplicator* replicator
    );

protected:
    MulticastServerMediaSubsession(StreamReplicator* replicator, RTPSink* rtpSink, RTCPInstance* rtcpInstance)
            : PassiveServerMediaSubsession(*rtpSink, rtcpInstance), BaseServerMediaSubsession(replicator), m_rtpSink(rtpSink) {};

    virtual char const* sdpLines() ;
    virtual char const* getAuxSDPLine(RTPSink* rtpSink,FramedSource* inputSource);

protected:
    RTPSink* m_rtpSink;
    std::string m_SDPLines;
};

// -----------------------------------------
//    ServerMediaSubsession for Unicast
// -----------------------------------------
class UnicastServerMediaSubsession : public OnDemandServerMediaSubsession , public BaseServerMediaSubsession
{
public:
    static UnicastServerMediaSubsession* createNew(UsageEnvironment& env, StreamReplicator* replicator);

protected:
    UnicastServerMediaSubsession(UsageEnvironment& env, StreamReplicator* replicator)
            : OnDemandServerMediaSubsession(env, False), BaseServerMediaSubsession(replicator) {};

    virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,  unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);
    virtual char const* getAuxSDPLine(RTPSink* rtpSink,FramedSource* inputSource);
};
#endif //LIVESTREAMER_SERVERMEDIASUBSESSION_H
