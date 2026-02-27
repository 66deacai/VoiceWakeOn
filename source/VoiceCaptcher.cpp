#include <stdint.h>
#include <time.h>
#include <stdio.h>

#include "Audio.h"
#include "common.h"
#include "AudioCapture.h"
#include "SpeechDetection.h"
#include "ProcessExecutor.h"
#include "ImgDisp/ImageDisplay.h"


ring_buffer_t st_g_rBuffer;

static int32_t s32_s_SpeechDetect();
static int32_t s32_s_StartDisp();

int main() {
    drm_init();
    s32_g_staticDisplay();
    s32_s_SpeechDetect();
    s32_g_dynamicDisplay();
}

static int32_t s32_s_SpeechDetect()
{
    uint32_t u32_t_audioTid;
    
    struct timespec ts;
    clock_gettime(1, &ts);
    LOG_DEBUG("starting at %ld.%06ld\n", ts.tv_sec, ts.tv_nsec / 1000);

    ring_buffer_init(&st_g_rBuffer,
        AUDIO_RATE * 4);

    LOG_DEBUG("Starting audio capture...\n");
    u32_t_audioTid = u32_g_StartAudioCapture(&st_g_rBuffer);

    // main thread: VAD / KWS
    LOG_DEBUG("Starting speech detector...\n");
    SpeechDetection::SpeechDetector st_t_SpeechDetector(&st_g_rBuffer);
    st_t_SpeechDetector.run();

    ring_buffer_free(&st_g_rBuffer);
    return 0;
}

// static int32_t s32_s_StartDisp()
// {
//     ProcessManagement::ProcessExecutor st_t_procExec;
//     st_t_procExec.s32_m_excuteFile(DISPLAYER, )
// }
