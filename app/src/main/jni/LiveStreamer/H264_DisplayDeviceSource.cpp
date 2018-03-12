#include "H264_DisplayDeviceSource.h"

#include <sstream>

// live555
#include <Base64.hh>
#include <iomanip>

#include "log.h"

H264_DisplayDeviceSource* H264_DisplayDeviceSource::createNew(UsageEnvironment& env,  unsigned int queueSize, bool useThread, bool repeatConfig)
{
    H264_DisplayDeviceSource* source = new H264_DisplayDeviceSource(env, queueSize, useThread, repeatConfig);
    return source;
}

// Constructor
H264_DisplayDeviceSource::H264_DisplayDeviceSource(UsageEnvironment& env, unsigned int queueSize, bool useThread, bool repeatConfig)
        : DisplayDeviceSource(env, queueSize,useThread), m_repeatConfig(repeatConfig)
{
}

// Destructor
H264_DisplayDeviceSource::~H264_DisplayDeviceSource()
{
}

// split packet in frames
std::list< std::pair<unsigned char*,size_t> > H264_DisplayDeviceSource::splitFrames(unsigned char* frame, unsigned frameSize)
{
    std::list< std::pair<unsigned char*,size_t> > frameList;

    size_t bufSize = frameSize;
    size_t size = 0;
    unsigned char* buffer = this->extractFrame(frame, bufSize, size);
    while (buffer != NULL)
    {
        switch (buffer[0]&0x1F)
        {
            case 7: LOGI("SPS size:%d", size); m_sps.assign((char*)buffer,size); break;
            case 8: LOGI("PPS size:%d", size); m_pps.assign((char*)buffer,size); break;
            case 5: LOGI("IDR size:%d" ,size);
                if (m_repeatConfig && !m_sps.empty() && !m_pps.empty())
                {
                    frameList.push_back(std::make_pair((unsigned char*)m_sps.c_str(), m_sps.size()));
                    frameList.push_back(std::make_pair((unsigned char*)m_pps.c_str(), m_pps.size()));
                }
                break;
            default: break;
        }

        if (m_auxLine.empty() && !m_sps.empty() && !m_pps.empty())
        {
            u_int32_t profile_level_id = 0;
            if (m_sps.size() >= 4) profile_level_id = (m_sps[1]<<16)|(m_sps[2]<<8)|m_sps[3];

            char* sps_base64 = base64Encode(m_sps.c_str(), m_sps.size());
            char* pps_base64 = base64Encode(m_pps.c_str(), m_pps.size());

            std::ostringstream os;
            os << "profile-level-id=" << std::hex << std::setw(6) << profile_level_id;
            os << ";sprop-parameter-sets=" << sps_base64 <<"," << pps_base64;
            m_auxLine.assign(os.str());

            free(sps_base64);
            free(pps_base64);
            LOGI("%s",m_auxLine.c_str());
        }
        frameList.push_back(std::make_pair(buffer, size));

        buffer = this->extractFrame(&buffer[size], bufSize, size);
    }
    return frameList;
}

// extract a frame
unsigned char*  H264_DisplayDeviceSource::extractFrame(unsigned char* frame, size_t& size, size_t& outsize)
{
    unsigned char * outFrame = NULL;
    outsize = 0;
    if ( (size>= sizeof(H264marker)) && (memcmp(frame,H264marker,sizeof(H264marker)) == 0) )
    {
        size -=  sizeof(H264marker);
        outFrame = &frame[sizeof(H264marker)];
        outsize = size;
        for (int i=0; i+sizeof(H264marker) < size; ++i)
        {
            if (memcmp(&outFrame[i],H264marker,sizeof(H264marker)) == 0)
            {
                outsize = i;
                break;
            }
        }
        size -=  outsize;
    }
    return outFrame;
}


