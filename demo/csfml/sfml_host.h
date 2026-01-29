#ifndef GLU_DEMO_SFML_HOST_H
#define GLU_DEMO_SFML_HOST_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void glu_update_motion(
    float *pos_xy, float *vel_xy, float *bounds_xy, float radius, float dt
);
void glu_color_from_frame(uint32_t frame, uint8_t *out_rgba);
void glu_jitter(uint32_t frame, float strength, float *out_xy);

#ifdef __cplusplus
}
#endif

#endif
