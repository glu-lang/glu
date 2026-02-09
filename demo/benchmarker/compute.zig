const std = @import("std");
const sha256 = std.crypto.hash.sha2.Sha256;

/// Compute SHA256 hash of data
export fn hashBytes(data: [*]const u8, len: usize, output: *[32]u8) void {
    sha256.hash(data[0..len], output, .{});
}

/// Verify if data matches expected hash
export fn verifyHash(data: [*]const u8, len: usize, expected: *const [32]u8) bool {
    var actual: [32]u8 = undefined;
    sha256.hash(data[0..len], &actual, .{});
    return std.mem.eql(u8, &actual, expected);
}

/// Compare two hashes for equality
export fn compareHashes(hash1: *const [32]u8, hash2: *const [32]u8) bool {
    return std.mem.eql(u8, hash1, hash2);
}
