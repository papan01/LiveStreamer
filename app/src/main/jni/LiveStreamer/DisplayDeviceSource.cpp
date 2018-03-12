#include "DisplayDeviceSource.h"

#include <iomanip>
#include <sstream>
#include "log.h"


int DisplayDeviceSource::bufferedSize = 200000;

int  DisplayDeviceSource::Stats::notify(int tv_sec, int framesize)
{
    m_fps++;
    m_size+=framesize;
    if (tv_sec != m_fps_sec)
    {
        //LOGI("m_msg:%ld, fps: %d, bandwidth: %d kbps",tv_sec, m_size/128);
        m_fps_sec = tv_sec;
        m_fps = 0;
        m_size = 0;
    }
    return m_fps;
}


DisplayDeviceSource* DisplayDeviceSource::createNew(UsageEnvironment& env, unsigned int queueSize, bool useThread)
{
    DisplayDeviceSource* source = new DisplayDeviceSource(env, queueSize, useThread);
    return source;
}

// Constructor
DisplayDeviceSource::DisplayDeviceSource(UsageEnvironment& env, unsigned int queueSize, bool useThread)
        : FramedSource(env),
          m_in("in"),
          m_out("out") ,
          m_queueSize(queueSize)
{
    m_eventTriggerId = envir().taskScheduler().createEventTrigger(DisplayDeviceSource::deliverFrameStub);
    memset(&m_thid, 0, sizeof(m_thid));
    memset(&m_mutex, 0, sizeof(m_mutex));
    memset(&m_mutex_raw,0,sizeof(m_mutex_raw));

    // start thread
    pthread_mutex_init(&m_mutex, NULL);
    pthread_mutex_init(&m_mutex_raw,NULL);
    pthread_create(&m_thid, NULL, threadStub, this);
}

// Destructor
DisplayDeviceSource::~DisplayDeviceSource()
{
    envir().taskScheduler().deleteEventTrigger(m_eventTriggerId);
    pthread_join(m_thid, NULL);
    pthread_mutex_destroy(&m_mutex);
    pthread_mutex_destroy(&m_mutex_raw);
}

// thread mainloop
void* DisplayDeviceSource::thread()
{
    while (true)
    {
        this->getNextFrame();
    }
}

// getting FrameSource callback
void DisplayDeviceSource::doGetNextFrame()
{
    deliverFrame();
}

// stopping FrameSource callback
void DisplayDeviceSource::doStopGettingFrames()
{
    LOGI("DisplayDeviceSource::doStopGettingFrames");
    FramedSource::doStopGettingFrames();
}

// deliver frame to the sink
void DisplayDeviceSource::deliverFrame()
{
    if (isCurrentlyAwaitingData())
    {
        fDurationInMicroseconds = 0;
        fFrameSize = 0;

        pthread_mutex_lock (&m_mutex);
        if (m_captureQueue.empty())
        {
            // LOGI("Queue is empty");
        }
        else
        {
            timeval curTime;
            gettimeofday(&curTime, NULL);
            Frame * frame = m_captureQueue.front();
            m_captureQueue.pop_front();

            m_out.notify(curTime.tv_sec, frame->m_size);
            if (frame->m_size > fMaxSize)
            {
                fFrameSize = fMaxSize;
                fNumTruncatedBytes = frame->m_size - fMaxSize;
            }
            else
            {
                fFrameSize = frame->m_size;
            }
            timeval diff;
            timersub(&curTime,&(frame->m_timestamp),&diff);

//            LOGI("deliverFrame\ttimestamp:%ld.%ld,\tsize:%d,\tdiff:%d ms\t queue size:%d",
//                 curTime.tv_sec  ,
//                 curTime.tv_usec ,
//                 fFrameSize ,
//                 (diff.tv_sec*1000+diff.tv_usec/1000),
//                 m_captureQueue.size());

            fPresentationTime = frame->m_timestamp;


            //LOGI("PresentationTime:%ld",fPresentationTime);

            memcpy(fTo, frame->m_buffer, fFrameSize);
            delete frame;
        }
        pthread_mutex_unlock (&m_mutex);

        if (fFrameSize > 0)
        {
            // send Frame to the consumer
            FramedSource::afterGetting(this);
        }
    }
}

// FrameSource callback on read event
void DisplayDeviceSource::incomingPacketHandler()
{
    if (this->getNextFrame() <= 0)
    {
        handleClosure(this);
    }
}



void DisplayDeviceSource::pushRawData(char* d,unsigned int dataSize)
{

    //LOGI("DisplayDeviceSource::pushRawData");
    pthread_mutex_lock(&m_mutex_raw);

    RawData* data = (RawData*) malloc(sizeof(RawData));
    memset(data,0,sizeof(RawData));
    data->m_buffer = d;
    data->m_size   = dataSize;
    m_rawDataQueue.push_back(data);

    pthread_mutex_unlock(&m_mutex_raw);
}


// read from device
int DisplayDeviceSource::getNextFrame()
{
    timeval ref;
    gettimeofday(&ref, NULL);

    int frameSize = 0;

    pthread_mutex_lock(&m_mutex_raw);

    if(m_rawDataQueue.empty())
    {
        // LOGI("rawDataQueue is empty");
    }
    else
    {
        //LOGI("Queue pop front RawData");
        RawData* rawData = m_rawDataQueue.front();
        m_rawDataQueue.pop_front();
        timeval tv;
        gettimeofday(&tv, NULL);
        timeval diff;
        timersub(&tv,&ref,&diff);
        frameSize = rawData->m_size;
        m_in.notify(tv.tv_sec, frameSize);
//        LOGI("getNextFrame\ttimestamp:%ld.%ld\tsize:%d\tdiff:%d ms",
//             ref.tv_sec,
//             ref.tv_usec,
//             frameSize,
//             (diff.tv_sec*1000+diff.tv_usec/1000)
//        );
        processFrame(rawData->m_buffer,frameSize,ref);
    }
    pthread_mutex_unlock(&m_mutex_raw);
    return frameSize;
}


void DisplayDeviceSource::processFrame(char * frame, int frameSize, const timeval &ref)
{

    //("processFrame");
    timeval tv;
    gettimeofday(&tv, NULL);
    timeval diff;
    timersub(&tv,&ref,&diff);

    std::list< std::pair<unsigned char*,size_t> > frameList = this->splitFrames((unsigned char*)frame, frameSize);
    while (!frameList.empty())
    {
        //LOGI("processFrame while loop");
        std::pair<unsigned char*,size_t>& frame = frameList.front();
        size_t size = frame.second;
        char* buf = new char[size];
        memcpy(buf, frame.first, size);
        queueFrame(buf,size,ref);

//        LOGI("queueFrame\ttimestamp:%d.%d\tsize:%d\tdiff:%fms",
//             ref.tv_sec,
//             ref.tv_usec,
//             size,
//             diff.tv_sec*1000+diff.tv_usec/1000
//        );
        frameList.pop_front();
    }

    //LOGI("frameList empty");
}

// post a frame to fifo
void DisplayDeviceSource::queueFrame(char * frame, int frameSize, const timeval &tv)
{
    pthread_mutex_lock (&m_mutex);
    while (m_captureQueue.size() >= m_queueSize)
    {
        //LOGI("Queue full size drop frame size:%d",(int)m_captureQueue.size());
        delete m_captureQueue.front();
        m_captureQueue.pop_front();
    }
    m_captureQueue.push_back(new Frame(frame, frameSize, tv));
    pthread_mutex_unlock (&m_mutex);

    // post an event to ask to deliver the frame
    envir().taskScheduler().triggerEvent(m_eventTriggerId, this);
}

// split packet in frames
std::list< std::pair<unsigned char*,size_t> > DisplayDeviceSource::splitFrames(unsigned char* frame, unsigned frameSize)
{
    std::list< std::pair<unsigned char*,size_t> > frameList;
    if (frame != NULL)
    {
        frameList.push_back(std::make_pair(frame, frameSize));
    }
    else
    {
        //LOGI("DisplayDeviceSource::splitFrames  frame empty");
    }
    return frameList;
}

