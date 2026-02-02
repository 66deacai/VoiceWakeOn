#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG(fmt, level, ...) \
    do { \
        fprintf(stderr, "[%s] [%s:%d] " fmt "\n", level, __func__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define LOG_LEVEL(level, fmt, ...) \
    LOG(fmt, level, ##__VA_ARGS__);

#define LOG_ERROR(fmt, ...) \
    do { \
        LOG_LEVEL("ERROR", fmt, ##__VA_ARGS__); \
    } while (0)

#if DEBUG >= 1
#define LOG_INFO(fmt, ...) \
    do { \
        LOG_LEVEL("INFO", fmt, ##__VA_ARGS__); \
    } while (0)
#else
#define LOG_INFO(fmt, ...) \
    do { } while (0)
#endif

#if DEBUG >= 2
#define LOG_DEBUG(fmt, ...) \
    do { \
        LOG_LEVEL("DEBUG", fmt, ##__VA_ARGS__); \
    } while (0)
#else
#define LOG_DEBUG(fmt, ...) \
    do { } while (0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __COMMON_H__ */
