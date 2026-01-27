
#![no_main]

/// Print byte slice as hex to stdout
pub fn print_hex(data: &[u8]) {
    for byte in data {
        print!("{:02x}", byte);
    }
    println!();
}

/// Print 32-byte hash array as hex to stdout
pub fn print_hash(hash: &[u8; 32]) {
    print_hex(hash);
}
