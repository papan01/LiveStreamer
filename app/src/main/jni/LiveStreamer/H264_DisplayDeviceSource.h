#ifndef LIVESTREAMER_H264_DISPLAYDEVICESOURCE_H
#define LIVESTREAMER_H264_DISPLAYDEVICESOURCE_H

#include "DisplayDeviceSource.h"


const char H264marker[] = {0,0,0,1};
class H264_DisplayDeviceSource : public DisplayDeviceSource
{
public:
    static H264_DisplayDeviceSource* createNew(UsageEnvironment& env, unsigned int queueSize, bool useThread, bool repeatConfig) ;

protected:
    H264_DisplayDeviceSource(UsageEnvironment& env, unsigned int queueSize, bool useThread, bool repeatConfig);
    virtual ~H264_DisplayDeviceSource();

    unsigned char* extractFrame(unsigned char* frame, size_t& size, size_t& outsize);

    // overide V4L2DeviceSource
    virtual std::list< std::pair<unsigned char*,size_t> > splitFrames(unsigned char* frame, unsigned frameSize);

private:
    std::string m_sps;
    std::string m_pps;
    bool        m_repeatConfig;
};
#endif //LIVESTREAMER_H264_DISPLAYDEVICESOURCE_H
