#ifndef LIVESTREAMER_DISPLAYDEVICESOURCE_H
#define LIVESTREAMER_DISPLAYDEVICESOURCE_H

#include <string>
#include <list>
#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <liveMedia.hh>

class DisplayDeviceSource: public FramedSource{

public:
    // ---------------------------------
    // Captured frame
    // ---------------------------------
    struct Frame
    {
        Frame(char* buffer, int size, timeval timestamp) : m_buffer(buffer), m_size(size), m_timestamp(timestamp) {};
        Frame(const Frame&);
        Frame& operator=(const Frame&);
        ~Frame()  { delete m_buffer; };

        char* m_buffer;
        int m_size;
        timeval m_timestamp;
    };


    // ----------------------------------
    // represent the data java pass into
    // ----------------------------------
    struct RawData{

        RawData(char* buffer, int size, timeval timestamp) : m_buffer(buffer), m_size(size), m_timestamp(timestamp) {};
        RawData(const RawData&);
        RawData& operator=(const RawData&);
        ~RawData()  { delete m_buffer; };

        char* m_buffer;
        int m_size;
        timeval m_timestamp;
    };

    // ---------------------------------
    // Compute simple stats
    // ---------------------------------
    class Stats
    {
    public:
        Stats(const std::string & msg) : m_fps(0), m_fps_sec(0), m_size(0), m_msg(msg) {};

    public:
        int notify(int tv_sec, int framesize);

    protected:
        int m_fps;
        int m_fps_sec;
        int m_size;
        const std::string m_msg;
    };

public:
    static DisplayDeviceSource* createNew(UsageEnvironment& env,  unsigned int queueSize, bool useThread) ;
    std::string getAuxLine() { return m_auxLine; };
    void pushRawData(char* data,unsigned int dataSize);

    static int bufferedSize;

protected:
    DisplayDeviceSource(UsageEnvironment& env, unsigned int queueSize, bool useThread);
    virtual ~DisplayDeviceSource();

protected:
    static void* threadStub(void* clientData) { return ((DisplayDeviceSource*) clientData)->thread();};
    void* thread();
    static void deliverFrameStub(void* clientData) {((DisplayDeviceSource*) clientData)->deliverFrame();};
    void deliverFrame();
    static void incomingPacketHandlerStub(void* clientData, int mask) { ((DisplayDeviceSource*) clientData)->incomingPacketHandler(); };
    void incomingPacketHandler();
    int getNextFrame();
    void processFrame(char * frame, int frameSize, const timeval &ref);
    void queueFrame(char * frame, int frameSize, const timeval &tv);

    // split packet in frames
    virtual std::list< std::pair<unsigned char*,size_t> > splitFrames(unsigned char* frame, unsigned frameSize);

    // overide FramedSource
    virtual void doGetNextFrame();
    virtual void doStopGettingFrames();

protected:
    std::list<Frame*> m_captureQueue;
    std::list<RawData*> m_rawDataQueue;
    Stats m_in;
    Stats m_out;
    EventTriggerId m_eventTriggerId;
    int m_outfd;
    unsigned int m_queueSize;
    pthread_t m_thid;
    pthread_mutex_t m_mutex;
    pthread_mutex_t m_mutex_raw;
    std::string m_auxLine;
};



#endif //LIVESTREAMER_DISPLAYDEVICESOURCE_H
