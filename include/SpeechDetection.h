#ifndef __SPEECH_DETECTION_H__
#define __SPEECH_DETECTION_H__

#include "RingBuffer.h"
#include "KwsExecutor.h"
#include "webrtc/webrtc_vad.h"
#include "SpeechCache.h"

uint32_t u32_g_StartSpeechDetection(ring_buffer_t *stp_a_rBuffer);

namespace SpeechDetection
{

class SpeechDetector 
{
public:
    SpeechDetector(ring_buffer_t *stp_a_rBuffer);
    ~SpeechDetector();

    void run();

private:
    KWSExecutor *stp_m_kwsExecutor;
    VadInst* stp_m_vadHandle;
    ring_buffer_t *stp_m_captureBuffer;
    ring_buffer_t *stp_m_speechBuffer;
    SpeechCache<int16_t> *stp_m_cache;

    int32_t s32_m_StatusMachine();
    int32_t s32_m_loadFrame(int16_t *s16p_a_frame, uint32_t s32_a_frameSize);
    int32_t s32_m_silenceStatus();
    int32_t s32_m_speechStatus();
};

}


#endif // __SPEECH_DETECTION_H__
