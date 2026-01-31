#ifndef GLU_DEMO_SFML_HOST_H
#define GLU_DEMO_SFML_HOST_H

#include <stdint.h>

#if defined(__GNUC__)
    #define GLU_WEAK __attribute__((weak))
#else
    #define GLU_WEAK
#endif

GLU_WEAK void glu_update_motion(
    float *pos_xy, float *vel_xy, float *bounds_xy, float radius, float dt
)
{
}
GLU_WEAK void glu_color_from_frame(uint32_t frame, uint8_t *out_rgba) { }
GLU_WEAK void glu_jitter(uint32_t frame, float strength, float *out_xy) { }

#endif
