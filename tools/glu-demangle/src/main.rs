mod args;
mod types;

use args::Args;
use clap::Parser;
use std::io::{self, BufRead};

fn run() -> anyhow::Result<()> {
    let args = Args::parse();

    if args.should_read_stdin() {
        let stdin = io::stdin();
        for line in stdin.lock().lines() {
            let line = line?;
            let trimmed = line.trim();
            if !trimmed.is_empty() {
                println!("Processing: {}", trimmed);
            }
        }
    } else {
        for name in &args.mangled_names {
            println!("Processing: {}", name);
        }
    }

    Ok(())
}

fn main() {
    if let Err(e) = run() {
        eprintln!("Error: {}", e);
        std::process::exit(1);
    }
}
