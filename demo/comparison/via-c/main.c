#include <stddef.h>
#include <stdint.h>

// External function declarations from Zig (compute.zig)
extern void hashBytes(uint8_t const *data, size_t len, uint8_t output[32]);

// External function declarations from D (random.d)
extern void fill_random(uint8_t *data, size_t len);

int main()
{
    for (int i = 0; i < 1000000; i++) {
        uint8_t data[100];
        size_t length = 100;
        uint8_t hash[32] = { 0 };

        fill_random(data, length);
        hashBytes(data, length, hash);
    }

    return 0;
}
