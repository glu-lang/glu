#![no_main]

#[no_mangle]
pub fn get_rust_count() -> u64
{
    13
}

#[no_mangle]
pub fn get_rust_compiler() -> &'static str
{
    "Rust"
}
