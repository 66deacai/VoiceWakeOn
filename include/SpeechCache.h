#ifndef __SPEECH_CACHE_H__
#define __SPEECH_CACHE_H__

#include <vector>
#include <stdlib.h>
#include <string.h>

#include "common.h"


namespace SpeechDetection
{
template <typename T>
class SpeechCache{

public:
    SpeechCache(uint32_t u32_a_BufferNum, uint32_t u32_a_BufferLen);
    ~SpeechCache();
    int32_t s32_m_fillCache(T *t_a_data, uint32_t u32_a_DataLen);
    int32_t s32_m_extractData(T *t_a_Data, uint32_t u32_a_DataLen);
    int32_t s32_m_storeData(T *t_a_Data, uint32_t u32_a_DataLen);

private:
    std::vector<T*> vec_m_Buffers;
    uint32_t u32_m_CacheLen;
    uint32_t u32_m_BufferLen;
    uint32_t u32_m_Index;
};

template <typename T>
SpeechCache<T>::SpeechCache(uint32_t u32_a_BufferNum, uint32_t u32_a_BufferLen)
{
    u32_m_BufferLen = u32_a_BufferLen;
    
    vec_m_Buffers.reserve(u32_a_BufferNum);
    for (uint32_t u32_t_Index = 0; u32_t_Index < u32_a_BufferNum; u32_t_Index++)
    {
        vec_m_Buffers.push_back((T*)malloc(sizeof(T) * u32_a_BufferLen));
    }
    u32_m_CacheLen = u32_a_BufferLen * u32_a_BufferNum;
    u32_m_Index = 0;
}

template <typename T>
SpeechCache<T>::~SpeechCache()
{
    for (auto &it : vec_m_Buffers)
    {
        free(it);
    }
}

template <typename T>
int32_t SpeechCache<T>::s32_m_fillCache(T *t_a_data, uint32_t u32_a_DataLen)
{
    uint32_t u32_t_Index;
    int32_t s32_t_ret;

    u32_t_Index = 0;
    s32_t_ret = 0;

    do
    {
        if (u32_a_DataLen != u32_m_CacheLen)
        {
            LOG_ERROR("SpeechCache::s32_m_extractData: %d does not match cache size %d", u32_a_DataLen, u32_m_CacheLen);
            s32_t_ret = -1;
            break;
        }

        for (u32_t_Index; u32_t_Index < u32_m_CacheLen; u32_t_Index += u32_m_BufferLen)
        {
            s32_t_ret = s32_m_storeData(t_a_data + u32_t_Index, u32_m_BufferLen);
            if (s32_t_ret != 0)
            {
                LOG_ERROR("SpeechCache::s32_m_fillCache: Failed to store data");
                break;
            }
        }
    } while (0);

    return s32_t_ret;
}

template <typename T>
int32_t SpeechCache<T>::s32_m_extractData(T *t_a_Data, uint32_t u32_a_DataLen)
{
    int32_t s32_t_ret;
    uint32_t u32_t_curIndex;
    uint32_t u32_t_offset;

    s32_t_ret = 0;
    u32_t_curIndex = u32_m_Index;

    do
    {
        if (u32_a_DataLen != u32_m_CacheLen)
        {
            LOG_ERROR("SpeechCache::s32_m_extractData: %d does not match cache size %d", u32_a_DataLen, u32_m_CacheLen);
            s32_t_ret = -1;
            break;
        }

        do
        {
            memcpy(t_a_Data + u32_t_offset, vec_m_Buffers[u32_t_curIndex], u32_m_BufferLen * sizeof(T));
            u32_t_offset += u32_m_BufferLen;
            u32_t_curIndex = (u32_t_curIndex + 1) % vec_m_Buffers.size();
        } while (u32_t_curIndex != u32_m_Index);

    } while (0);
    
    return s32_t_ret;
}

template <typename T>
int32_t SpeechCache<T>::s32_m_storeData(T *t_a_Data, uint32_t u32_a_DataLen)
{
    int32_t s32_t_ret;

    s32_t_ret = 0;

    do
    {
        if (u32_a_DataLen != u32_m_BufferLen)
        {
            LOG_ERROR("SpeechCache::s32_m_storeData: Data size does not match buffer size");
            s32_t_ret = -1;
            break;
        }

        memcpy(vec_m_Buffers[u32_m_Index], t_a_Data, u32_m_BufferLen * sizeof(T));
        u32_m_Index = (u32_m_Index + 1) % vec_m_Buffers.size();
    } while(0);

    return s32_t_ret;
}


}

#endif /*!__SPEECH_CACHE_H__ */
