
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#include "common.h"
#include "KwsExecutor.h"
#include "logmel/logmel_trans.h"

#define RKNN_MODEL_PATH "/root/res/kws.rknn"

#define MEL_BINS 40
#define NUM_FRAMES 98

#ifdef DEBUG_KWS
#include "md5.h"
#define LOG_MD5(data, size) \
    do { \
        char ac_md5[33]; \
        md5_digest(data, size, ac_md5); \
        LOG_INFO("md5: %s\n", ac_md5); \
    } while (0)
#endif

static int32_t s32_s_loadRknnModel(rknn_context *stp_a_rknnCtx)
{
    int32_t s32_t_fd;
    int32_t s32_t_modelSize;
    int32_t s32_t_ret;
    void *vdp_t_rknnModel;

    s32_t_fd = 0;
    s32_t_modelSize = 0;
    s32_t_ret = 0;

    do
    {
        s32_t_fd = open(RKNN_MODEL_PATH, O_RDONLY);
        if (s32_t_fd < 0) {
            LOG_ERROR("Failed to open rknn model file\n");
            s32_t_ret = -1;
            break;
        }

        s32_t_modelSize = lseek(s32_t_fd, 0, SEEK_END);
        lseek(s32_t_fd, 0, SEEK_SET);

        vdp_t_rknnModel = malloc(s32_t_modelSize);
        memset(vdp_t_rknnModel, 0, s32_t_modelSize);

        if (NULL == vdp_t_rknnModel) {
            LOG_ERROR("Failed to allocate memory for rknn model\n");
            s32_t_ret = -1;
            break;
        }

        if (read(s32_t_fd, vdp_t_rknnModel, s32_t_modelSize) != s32_t_modelSize) {
            LOG_ERROR("Failed to read rknn model file\n");
            free(vdp_t_rknnModel);
            vdp_t_rknnModel = NULL;
            s32_t_ret = -1;
            break;
        }

        rknn_init(stp_a_rknnCtx, vdp_t_rknnModel, s32_t_modelSize, 0, NULL);
    } while (0);
    
    if (s32_t_fd > 0) {
        close(s32_t_fd);
    }
    if (vdp_t_rknnModel) {
        free(vdp_t_rknnModel);
    }
    return s32_t_ret;
}

SpeechDetection::KWSExecutor::KWSExecutor()
    :st_m_rknnCtx(0)
{
    s32_s_loadRknnModel(&st_m_rknnCtx);
}

SpeechDetection::KWSExecutor::~KWSExecutor()
{
    rknn_destroy(st_m_rknnCtx);
}

void SpeechDetection::KWSExecutor::vd_m_setPcm(
    const int16_t *s16p_a_pcm,
    int32_t s32_a_pcmLen)
{

    vd_m_execKws(s16p_a_pcm, s32_a_pcmLen, fp_m_confidence, RKNN_OUTPUT_SIZE);
    
    LOG_INFO("KWS confidence: [%.6f, %.6f]\n", fp_m_confidence[0], fp_m_confidence[1]);

    s32_m_triggered = fp_m_confidence[1] > 0.5f ? 1 : 0;

    LOG_INFO("KWS triggered: %d\n", s32_m_triggered);
}

int32_t SpeechDetection::KWSExecutor::s32_m_isTriggered()
{
    return s32_m_triggered;
}

float SpeechDetection::KWSExecutor::f_m_getConfidence()
{
    return fp_m_confidence[1];
}

static float softmax_2(float a, float b)
{
    float ea = expf(a);
    float eb = expf(b);
    return eb / (ea + eb);  // keyword prob
}


void SpeechDetection::KWSExecutor::vd_m_execKws(
    const int16_t *s16p_a_pcm,
    int32_t s32_a_pcmLen,
    float *fp_a_score,
    int32_t s32_a_scoreLen)
{
    int32_t s32_t_ret;
    float *fp_t_logmelInput;
    float f_t_outputData[2];
    rknn_input st_t_rknnInput;
    rknn_output st_t_output;

    s32_t_ret = 0;

    do
    {
        if (s32_a_pcmLen <= 0) {
            LOG_ERROR("Invalid pcm length %d\n", s32_a_pcmLen);
            break;
        }

        fp_t_logmelInput = (float *)malloc(MEL_BINS * NUM_FRAMES * sizeof(float));
        memset(fp_t_logmelInput, 0, MEL_BINS * NUM_FRAMES * sizeof(float));
        if (fp_t_logmelInput == NULL) {
            LOG_ERROR("Failed to allocate memory for logmel input\n");
            break;
        }

        /* trans pcm into logmel formation */
        kws_extract_logmel_cmvn(s16p_a_pcm, s32_a_pcmLen, fp_t_logmelInput, MEL_BINS * NUM_FRAMES * sizeof(float));
        #ifdef DEBUG_KWS
        LOG_INFO("kws_extract_logmel_cmvn(%p, %d, %p, %d) done\n", s16p_a_pcm, s32_a_pcmLen, fp_t_logmelInput, NUM_FRAMES);
        LOG_MD5((char*)s16p_a_pcm, s32_a_pcmLen * 2);
        LOG_MD5((char*)fp_t_logmelInput, MEL_BINS * NUM_FRAMES * sizeof(float));
        #endif

        /* rknn */

        /* set input */
        st_t_rknnInput.index = 0;
        st_t_rknnInput.type = RKNN_TENSOR_FLOAT32;
        st_t_rknnInput.fmt = RKNN_TENSOR_NHWC;
        st_t_rknnInput.size = (MEL_BINS * NUM_FRAMES * sizeof(float));
        st_t_rknnInput.buf = (void *)fp_t_logmelInput;

        LOG_DEBUG("rknn input size: %d \t format: %d \n", st_t_rknnInput.size, st_t_rknnInput.fmt);

        s32_t_ret = rknn_inputs_set(st_m_rknnCtx, 1, &st_t_rknnInput);
        LOG_DEBUG("rknn_inputs_set returned %d\n", s32_t_ret);
        if (s32_t_ret < 0) {
            LOG_ERROR("rknn_inputs_set failed - %d\n", s32_t_ret);
            break;
        }

        /* run rknn */
        s32_t_ret = rknn_run(st_m_rknnCtx, NULL);
        if (s32_t_ret < 0) {
            LOG_ERROR("rknn_run failed - %d\n", s32_t_ret);
            break;
        }
        LOG_DEBUG("rknn run returned %d\n", s32_t_ret);

        /* get rknn output */
        st_t_output.index = 0;
        st_t_output.is_prealloc = 1;
        st_t_output.buf = (void *)f_t_outputData;
        st_t_output.size = sizeof(f_t_outputData);
        st_t_output.want_float = 1;

        rknn_outputs_get(st_m_rknnCtx, 1, &st_t_output, NULL);
        LOG_DEBUG("rknn output success\n");
        if (st_t_output.buf == NULL) {
            LOG_ERROR("rknn_outputs_get failed\n");
            break;
        }
        
        memcpy(fp_a_score, st_t_output.buf, s32_a_scoreLen * sizeof(float));
        
        LOG_DEBUG("rknn output data: %f \t %f \n", fp_a_score[0], fp_a_score[1]);
    } while (0);

    rknn_outputs_release(st_m_rknnCtx, 1, &st_t_output);
}

        