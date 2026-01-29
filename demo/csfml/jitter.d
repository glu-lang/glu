module jitter;

extern(C):

void jitter(uint seed, float strength, float* out_xy) {
    if (out_xy is null) {
        return;
    }

    uint s = seed * 1664525u + 1013904223u;
    float dx = (cast(float)(s & 0xFF) / 255.0f - 0.5f) * strength;
    s = s * 1664525u + 1013904223u;
    float dy = (cast(float)((s >> 8) & 0xFF) / 255.0f - 0.5f) * strength;

    out_xy[0] = dx;
    out_xy[1] = dy;
}
