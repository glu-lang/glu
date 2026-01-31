import std.random;

/// Fill array with random bytes
extern(C) void fill_random(ubyte* data, size_t len) {
    auto rng = Random(unpredictableSeed);
    foreach (i; 0 .. len) {
        data[i] = cast(ubyte)(rng.front % 256);
        rng.popFront();
    }
}
