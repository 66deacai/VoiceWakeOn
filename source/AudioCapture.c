
#define _POSIX_C_SOURCE 200809L

#include <time.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <stdio.h>

#define PCM_DEVICE      ("default")
#define AUDIO_CHANNELS  (1U)
#define PERIOD_FRAMES   (480U)
#define BUFFER_SIZE     (AUDIO_CHANNELS * PERIOD_FRAMES)

#include "Audio.h"
#include "AudioCapture.h"


static void vd_s_pcmInit(snd_pcm_t **stp_a_pcmHandle)
{
    int32_t s32_t_direct;
    uint32_t s32_t_rate;
    snd_pcm_uframes_t  s32_t_period;
    snd_pcm_t *stp_t_pcm;
    snd_pcm_hw_params_t *stp_t_pcmParam;

    s32_t_direct = 0;
    s32_t_rate = AUDIO_RATE;
    s32_t_period = PERIOD_FRAMES;
    stp_t_pcm = *stp_a_pcmHandle;
    stp_t_pcmParam = NULL;
    
    // fprintf(stderr, "Opening PCM device %s\n", PCM_DEVICE);
    snd_pcm_open(stp_a_pcmHandle, PCM_DEVICE,
                 SND_PCM_STREAM_CAPTURE, 0);

    stp_t_pcm = *stp_a_pcmHandle;
    // fprintf(stderr, "PCM device opened\n");
    snd_pcm_hw_params_malloc(&stp_t_pcmParam);
    snd_pcm_hw_params_any(stp_t_pcm, stp_t_pcmParam);
    // fprintf(stderr, "snd_pcm_hw_params_any finished\n");

    snd_pcm_hw_params_set_access(
        stp_t_pcm, stp_t_pcmParam, SND_PCM_ACCESS_RW_INTERLEAVED);
    // fprintf(stderr, "snd_pcm_hw_params_set_access finished\n");

    snd_pcm_hw_params_set_format(
        stp_t_pcm, stp_t_pcmParam, SND_PCM_FORMAT_S16_LE);
    // fprintf(stderr, "snd_pcm_hw_params_set_format finished\n");

    snd_pcm_hw_params_set_channels(
        stp_t_pcm, stp_t_pcmParam, AUDIO_CHANNELS);
    // fprintf(stderr, "snd_pcm_hw_params_set_channels finished\n");

    snd_pcm_hw_params_set_rate_near(
        stp_t_pcm, stp_t_pcmParam, &s32_t_rate, &s32_t_direct);
    // fprintf(stderr, "snd_pcm_hw_params_set_rate finished\n");

    snd_pcm_hw_params_set_period_size_near(
        stp_t_pcm, stp_t_pcmParam, &s32_t_period, &s32_t_direct);
    // fprintf(stderr, "snd_pcm_hw_params_set_period_size finished\n");
    printf(" s32_t_direct = %d s32_t_rate = %d s32_t_period = %d period_size = %d\n", s32_t_direct, s32_t_rate, s32_t_period, 1);

    snd_pcm_hw_params(stp_t_pcm, stp_t_pcmParam);
    // fprintf(stderr, "snd_pcm_hw_params finished\n");

    // snd_pcm_uframes_t period_size;
    // snd_pcm_hw_params_get_period_size(stp_t_pcmParam, &period_size, &s32_t_direct);
}

void set_realtime_priority()
{
    struct sched_param param;
    param.sched_priority = 80;   // 1~99

    pthread_setschedparam(
        pthread_self(),
        SCHED_FIFO,
        &param
    );
}

#ifdef DEBUG_AC
#include <fcntl.h>
#define AUDIO_LEN (AUDIO_RATE * 3)
#endif

static void* vd_s_acThread(void* arg) {
    int16_t buffer[BUFFER_SIZE];
    int32_t s32_t_ret;
    ring_buffer_t *stp_t_rBuffer;
    snd_pcm_t *stp_t_pcm;

    s32_t_ret = 0;
    stp_t_rBuffer = (ring_buffer_t *)arg;
    stp_t_pcm = NULL;

    set_realtime_priority();

    vd_s_pcmInit(&stp_t_pcm);
    snd_pcm_prepare(stp_t_pcm);

    while (1) {
        s32_t_ret = snd_pcm_readi(stp_t_pcm,
                                buffer,
                                PERIOD_FRAMES);
        
        #ifdef DEBUG_AC
        uint32_t u32_t_avali = ring_buffer_available(stp_t_rBuffer);
        static uint32_t s32_t_writeFlg = 0;
        static int32_t idx = 0;
        // printf("%d\n", idx++);
        if (u32_t_avali > 60000)
        {
            if (!s32_t_writeFlg)
            {
                s32_t_writeFlg = 1;
                int s32_t_fd = open("tmp/voice_file.pcm", O_WRONLY | O_CREAT , 0644);
                if (s32_t_fd > 0)
                {
                    write(s32_t_fd, stp_t_rBuffer->buffer, AUDIO_LEN * sizeof(int16_t));
                    printf("write %d\n", AUDIO_LEN);
                }
            }
        }
        #endif

        if (s32_t_ret < 0) {
            snd_pcm_prepare(stp_t_pcm);
            fprintf(stderr, "snd_pcm_readi called error\n");
            continue;
        }

        ring_buffer_write(stp_t_rBuffer,
                          buffer,
                          PERIOD_FRAMES);
        // fprintf(stderr, "Wrote %d frames to ring buffer\n", s32_t_ret);
    }

    snd_pcm_close(stp_t_pcm);
    return NULL;
}


uint32_t u32_g_StartAudioCapture(ring_buffer_t *stp_a_rBuffer)
{
    pthread_t u32_t_tid;

    u32_t_tid = 0;

    pthread_create(&u32_t_tid,
        NULL,
        vd_s_acThread,
        (void *)stp_a_rBuffer);

    return u32_t_tid;
}

