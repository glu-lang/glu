#![no_main]

pub fn speaker() -> &'static str {
    "Rust"
}

pub fn message() -> &'static str {
    "Rust adds safety to the project"
}

pub fn timing(limit: u32) -> u32 {
    if limit > 30 {
        30
    } else {
        limit
    }
}
