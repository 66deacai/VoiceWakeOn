#ifndef __KWS_EXECUTOR_H__
#define __KWS_EXECUTOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <rknn/rknn_api.h>

namespace SpeechDetection
{

#define RKNN_OUTPUT_SIZE 2

class KWSExecutor
{
public:
    KWSExecutor();
    ~KWSExecutor();

    void vd_m_setPcm(const int16_t *s16p_a_pcm, int32_t s32_a_pcmLen);
    int32_t s32_m_isTriggered();
    float f_m_getConfidence();

private:
    int32_t s32_m_triggered;
    float fp_m_confidence[RKNN_OUTPUT_SIZE];
    rknn_context st_m_rknnCtx;
    
    void vd_m_execKws(const int16_t *s16p_a_pcm, int32_t s32_a_pcmLen, float *fp_a_score, int32_t s32_a_scoreLen);
};

}

#ifdef __cplusplus
}
#endif

#endif /* __KWS_EXECUTOR_H__ */