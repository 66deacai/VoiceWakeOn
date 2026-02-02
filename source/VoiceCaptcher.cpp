#include <stdint.h>
#include <time.h>
#include <stdio.h>

#include "Audio.h"
#include "common.h"
#include "AudioCapture.h"
#include "SpeechDetection.h"

ring_buffer_t st_g_rBuffer;


int main() {
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

    while(1);

    ring_buffer_free(&st_g_rBuffer);
    return 0;
}

