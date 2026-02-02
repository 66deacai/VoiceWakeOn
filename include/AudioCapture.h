#ifndef __AUDIO_CAPTURE_H__
#define __AUDIO_CAPTURE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "RingBuffer.h"

uint32_t u32_g_StartAudioCapture(ring_buffer_t *stp_a_rBuffer);

#ifdef __cplusplus
}
#endif

#endif // __AUDIO_CAPTURE_H__
