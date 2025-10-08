mod args;
mod types;

use std::io::{self, BufRead};
use args::Args;

fn run() -> anyhow::Result<()> {
    let args = Args::parse()?;

    if args.stdin {
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
