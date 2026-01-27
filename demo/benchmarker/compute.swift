import Foundation
import CryptoKit

/// Compute SHA256 hash of data
public func hashBytes(
    data: UnsafeRawPointer,
    len: Int,
    output: UnsafeMutableRawPointer
) {
    let buffer = UnsafeRawBufferPointer(start: data, count: len)
    let digest = SHA256.hash(data: buffer)
    digest.withUnsafeBytes { bytes in
        output.copyMemory(from: bytes.baseAddress!, byteCount: 32)
    }
}

/// Verify if data matches expected hash
public func verifyHash(
    data: UnsafeRawPointer,
    len: Int,
    expected: UnsafeRawPointer
) -> Bool {
    let buffer = UnsafeRawBufferPointer(start: data, count: len)
    let digest = SHA256.hash(data: buffer)
    
    let expectedBuffer = UnsafeRawBufferPointer(start: expected, count: 32)
    return digest.withUnsafeBytes { actual in
        return actual.elementsEqual(expectedBuffer)
    }
}

/// Compare two hashes for equality
public func compareHashes(
    hash1: UnsafeRawPointer,
    hash2: UnsafeRawPointer
) -> Bool {
    let buffer1 = UnsafeRawBufferPointer(start: hash1, count: 32)
    let buffer2 = UnsafeRawBufferPointer(start: hash2, count: 32)
    return buffer1.elementsEqual(buffer2)
}
