#ifndef __IMAGEDISPLAY_H__
#define __IMAGEDISPLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void drm_init();
int32_t s32_g_staticDisplay();
int32_t s32_g_dynamicDisplay();

#ifdef __cplusplus
}
#endif

#endif /* !__IMAGEDISPLAY_H__ */
