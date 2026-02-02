#ifndef __LOGMEL_TRANS_H__
#define __LOGMEL_TRANS_H__  
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KWS_SAMPLE_RATE 16000
#define KWS_N_FFT       400
#define KWS_WIN_LENGTH  400
#define KWS_HOP_LENGTH  160
#define KWS_N_MELS      40

int kws_extract_logmel_cmvn(
    const int16_t* pcm,
    int pcm_len,
    float* out_feat,
    uint32_t feat_size
);

#ifdef __cplusplus
}
#endif

#endif /* __LOGMEL_TRANS_H__ */
