#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "Audio.h"
#include "common.h"
#include "SpeechDetection.h"

#define SPEECH_THRESHOLD        (10U)
#define SILENCE_THRESHOLD       (20U)
#define FRAME_MS                (20U)
#define FRAME_LEN               (AUDIO_RATE * FRAME_MS / 1000)
#define SPEECH_BUFFER_LEN       (FRAME_MS * SPEECH_THRESHOLD)
#define KWS_BUFFER_LEN          (AUDIO_RATE * 1)

#define KWS_MAX_COUNT           (2)
#define SLIDING_WINDOW_MS       (250)
#define SLIDING_WINDOW_LEN      (SLIDING_WINDOW_MS * AUDIO_RATE / 1000)
#define SLIDING_WINDOW_COUNT    (1000 / SLIDING_WINDOW_MS)


#ifdef DEBUG_SD
#include <fcntl.h>
void write_wav_header(FILE *fp,
                      uint32_t sample_rate,
                      uint16_t bits_per_sample,
                      uint16_t channels,
                      uint32_t total_sample_count)
{
    uint32_t byte_rate = sample_rate * channels * bits_per_sample / 8;
    uint16_t block_align = channels * bits_per_sample / 8;
    uint32_t data_size = total_sample_count * block_align;
    uint32_t file_size = data_size + 36;   /* 整个文件大小 - 8 */

    /* RIFF 头 */
    fwrite("RIFF", 1, 4, fp);
    fwrite(&file_size, 4, 1, fp);
    fwrite("WAVE", 1, 4, fp);

    /* fmt 子块 */
    fwrite("fmt ", 1, 4, fp);
    uint32_t fmt_size = 16;
    uint16_t audio_format = 1;             /* PCM */
    fwrite(&fmt_size, 4, 1, fp);
    fwrite(&audio_format, 2, 1, fp);
    fwrite(&channels, 2, 1, fp);
    fwrite(&sample_rate, 4, 1, fp);
    fwrite(&byte_rate, 4, 1, fp);
    fwrite(&block_align, 2, 1, fp);
    fwrite(&bits_per_sample, 2, 1, fp);

    /* data 子块 */
    fwrite("data", 1, 4, fp);
    fwrite(&data_size, 4, 1, fp);
}

#define WRITE_WAV(fileNmae, data, len) \
    do { \
        FILE *file = fopen(fileNmae, "wb"); \
        write_wav_header(file, AUDIO_RATE, 16, 1, len); \
        fwrite(data, 1, len, file); \
        fclose(file); \
    } while (0)

#endif

enum SPEECH_DETECT_STATUS
{
    SPEECH_STATUS_SILENCE = 0,
    SPEECH_STATUS_SPEECH,
    SPEECH_STATUS_EXIT = 0x0FFF0000,
    SPEECH_STATUS_ERR,
};

static inline int32_t s32_s_waitForSpeech(ring_buffer_t *stp_a_rBuffer, int16_t *s16_a_handle, uint32_t u32_t_Len)
{
    int32_t s32_t_availLen;
    int32_t s32_t_ret;

    s32_t_availLen = 0;
    s32_t_ret = 0;

    while (s32_t_availLen < u32_t_Len) {
        s32_t_availLen = ring_buffer_available(stp_a_rBuffer);
        usleep((FRAME_MS) / 1000);
    }
    LOG_DEBUG("[Speech] Available samples in capture buffer: %d\n", s32_t_availLen);

    s32_t_ret = ring_buffer_read(stp_a_rBuffer, s16_a_handle, u32_t_Len);
    
    return s32_t_ret;
}

SpeechDetection::SpeechDetector::SpeechDetector(ring_buffer_t *stp_a_rBuffer)
    :stp_m_captureBuffer(stp_a_rBuffer),
    stp_m_vadHandle(NULL),
    stp_m_speechBuffer(NULL),
    stp_m_kwsExecutor(NULL)
{
    /* Initialize vad handle */

    LOG_INFO("---------- Initializing VAD ---------- \n");
    LOG_DEBUG("Creating VAD instance...\n");
    stp_m_vadHandle = WebRtcVad_Create();
    LOG_DEBUG("Initializing VAD instance...\n");
    WebRtcVad_Init(stp_m_vadHandle);
    LOG_DEBUG("Setting VAD mode to 3...\n");
    WebRtcVad_set_mode(stp_m_vadHandle, 3);

    /* Initialize speech buffer */
    LOG_INFO("---------- Initializing Speech Buffer ---------- \n");
    LOG_DEBUG("Creating speech ring buffer...\n");
    stp_m_speechBuffer = new ring_buffer_t();
    LOG_DEBUG("Initializing speech ring buffer...\n");
    ring_buffer_init(stp_m_speechBuffer, SPEECH_BUFFER_LEN); // according to speech_thread hold

    /* Initialize KWS executor */
    LOG_INFO("---------- Initializing KWS Executor ---------- \n");
    LOG_DEBUG("Creating KWS executor...\n");
    stp_m_kwsExecutor = new KWSExecutor();

    /* Initialize Speech Cache */
    LOG_INFO("---------- Initializing Speech Cache ---------- \n");
    LOG_DEBUG("Creating speech cache...\n");
    stp_m_cache = new SpeechCache<int16_t>(SLIDING_WINDOW_COUNT, SLIDING_WINDOW_LEN);
}

SpeechDetection::SpeechDetector::~SpeechDetector()
{
    WebRtcVad_Free(stp_m_vadHandle);
    ring_buffer_free(stp_m_speechBuffer);
    delete stp_m_kwsExecutor;
    delete stp_m_cache;
}

void SpeechDetection::SpeechDetector::run()
{
    int32_t s32_t_ret;
    
    s32_t_ret = 0;

    s32_t_ret = s32_m_StatusMachine();
    LOG_ERROR("Speech detection thread exit with status %d\n", s32_t_ret);
}

int32_t SpeechDetection::SpeechDetector::s32_m_StatusMachine()
{
    int32_t s32_t_status;

    s32_t_status = SPEECH_STATUS_SILENCE;

    do
    {
        switch (s32_t_status)
        {
        case SPEECH_STATUS_SILENCE:
            s32_t_status = s32_m_silenceStatus();
            break;

        case SPEECH_STATUS_SPEECH:
            s32_t_status = s32_m_speechStatus();
            break;

        case SPEECH_STATUS_EXIT:
            LOG_INFO("Speech detection exit\n");
            break;

        default:
            LOG_ERROR("Unknown speech detection status %d\n", s32_t_status);
            break;
        }
    } while(s32_t_status < SPEECH_STATUS_EXIT);

    return s32_t_status;
}


int32_t SpeechDetection::SpeechDetector::s32_m_loadFrame(int16_t *s16p_a_frame, uint32_t s32_a_frameSize)
{
    uint32_t u32_t_size;
    int32_t s32_t_ret;

    u32_t_size = 0;
    s32_t_ret = 0;


    while(s32_a_frameSize > ring_buffer_available(stp_m_captureBuffer))
    {
        usleep((FRAME_MS / 2) / 1000);
    }

    u32_t_size = ring_buffer_read(
                        stp_m_captureBuffer,
                        s16p_a_frame,
                        s32_a_frameSize);
    
    if (u32_t_size != s32_a_frameSize)
    {
        s32_t_ret = -1;
        LOG_ERROR("Failed to read enough data from ring buffer\n");
    }


    return s32_t_ret;
}



int32_t SpeechDetection::SpeechDetector::s32_m_silenceStatus()
{
    int16_t s16p_t_frame[FRAME_LEN];
    int32_t s32_t_speechCnt;
    int32_t s32_t_vadRes;
    int32_t s32_t_status;

    s32_t_speechCnt = 0;
    s32_t_vadRes = 0;
    s32_t_status = SPEECH_STATUS_SILENCE;

    do
    {
        /* Load 1 frame from capture buffer */
        s32_t_status = s32_m_loadFrame(s16p_t_frame, FRAME_LEN);
        if (s32_t_status < 0) {
            s32_t_status = SPEECH_STATUS_ERR;
            break;
        }

        /* VAD - check whether speeching */
        s32_t_vadRes = WebRtcVad_Process(
                stp_m_vadHandle,
                AUDIO_RATE,
                s16p_t_frame,
                FRAME_LEN
            );
        
        /* speech more than SPEECH_THRESHOLD frames -> trans into speech status */
        switch (s32_t_vadRes)
        {
        case 0:
            s32_t_speechCnt = 0;
            break;
        
        case 1:
            s32_t_speechCnt++;
            if (s32_t_speechCnt >= SPEECH_THRESHOLD) {
                s32_t_status = SPEECH_STATUS_SPEECH;
                ring_buffer_write(
                    stp_m_speechBuffer,
                    s16p_t_frame,
                    FRAME_LEN
                );
            }
            break;
        
        default:
            LOG_ERROR("[Silence] WebRtcVad_Process return err - %d\n", s32_t_vadRes);
            s32_t_status = SPEECH_STATUS_ERR;
            break;
        }

    } while(SPEECH_STATUS_SILENCE == s32_t_status);

    LOG_INFO("[Silence] Enter status: %d\n", s32_t_status);

    return s32_t_status;
}

int32_t SpeechDetection::SpeechDetector::s32_m_speechStatus()
{
    int16_t s16_t_KWSBuffer[KWS_BUFFER_LEN];
    int16_t s16_t_SlideBuffer[SLIDING_WINDOW_LEN];
    int32_t s32_t_ret = 0;
    int32_t s32_t_status;
    uint32_t u32_t_kwsCnt;
    int16_t *s16p_t_KWSHandle;
    int16_t *s16p_t_SlideHandle;

    s32_t_ret = 0;
    s32_t_status = SPEECH_STATUS_SPEECH;
    u32_t_kwsCnt = 0;
    s16p_t_KWSHandle = s16_t_KWSBuffer;
    s16p_t_SlideHandle = s16_t_SlideBuffer;

    do 
    {
        /* ******************** */
        /* storing 1S voices    */
        /* ******************** */

        /* load speech buffer to array */
        s32_t_ret = ring_buffer_read(stp_m_speechBuffer, s16p_t_KWSHandle, SPEECH_BUFFER_LEN);

        if (s32_t_ret != SPEECH_BUFFER_LEN) {
            LOG_ERROR("[Speech] read %d samples\n", s32_t_ret);
            s32_t_status = SPEECH_STATUS_ERR;
            break;
        }

        /* wait for ringbuffer is enough, read into s16p_t_KWSHandle */
        s32_t_ret = s32_s_waitForSpeech(stp_m_captureBuffer, s16p_t_KWSHandle + SPEECH_BUFFER_LEN, (KWS_BUFFER_LEN - SPEECH_BUFFER_LEN));
        if (s32_t_ret != (KWS_BUFFER_LEN - SPEECH_BUFFER_LEN)) {
            LOG_ERROR("[Speech] Failed to read enough data from capture buffer\n");
            s32_t_status = SPEECH_STATUS_ERR;
            break;
        }

        /* fill stp_m_cache */
        stp_m_cache->s32_m_fillCache(s16p_t_KWSHandle, KWS_BUFFER_LEN);

        while (u32_t_kwsCnt < KWS_MAX_COUNT) {
            /* KWS processing */
            stp_m_kwsExecutor->vd_m_setPcm(s16p_t_KWSHandle, KWS_BUFFER_LEN);

            #ifdef DEBUG_SD
            char fileName[32];
            static int cnt = 0;
            int fd = 0;
            int ret = 0;

            if (stp_m_kwsExecutor->s32_m_isTriggered()) 
            {
                sprintf(fileName, "tmp/pcm%d_triggerd.wav", cnt++);
                s32_t_status = SPEECH_STATUS_EXIT;
            }
            else
            {
                sprintf(fileName, "tmp/pcm%d_untriggerd.wav", cnt++);
            }
            LOG_DEBUG("writing pcm to %s\n", fileName);
            WRITE_WAV(fileName, s16p_t_KWSHandle, KWS_BUFFER_LEN * sizeof(*s16p_t_KWSHandle));
            #endif

            if (stp_m_kwsExecutor->s32_m_isTriggered())
            {
                float f_t_confidence;
                f_t_confidence = stp_m_kwsExecutor->f_m_getConfidence();
                LOG_INFO("KWS triggered! confidence: %.3f\n", f_t_confidence);
                s32_t_status = SPEECH_STATUS_SILENCE;
                break;
            }

            u32_t_kwsCnt ++;
            /* update cache */
            if (u32_t_kwsCnt == KWS_MAX_COUNT)
            {
                s32_t_status = SPEECH_STATUS_SILENCE;
                break;
            }
            s32_t_ret = s32_s_waitForSpeech(stp_m_captureBuffer, s16p_t_SlideHandle, SLIDING_WINDOW_LEN);
            if (s32_t_ret != SLIDING_WINDOW_LEN)
            {
                s32_t_status = SPEECH_STATUS_EXIT;
            }
            
            s32_t_ret = stp_m_cache->s32_m_storeData(s16p_t_SlideHandle, SLIDING_WINDOW_LEN);
            if (0 != s32_t_ret)
            {
                LOG_ERROR("store %d into stp_m_cache faild\n", SLIDING_WINDOW_LEN);
            }
            s32_t_ret = stp_m_cache->s32_m_extractData(s16p_t_KWSHandle, KWS_BUFFER_LEN);
        }

    } while(0);

    return s32_t_status;
}
