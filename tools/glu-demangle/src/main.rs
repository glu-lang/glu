mod args;

use glu_demangle::types;
use args::Args;
use clap::Parser;
use glu_demangle::{demangle, format_function};
use std::io::{self, BufRead, Write};

fn process_name(name: &str, output: &mut Box<dyn Write>, format: &types::DisplayFormat, strip_module: bool) {
    match demangle(name) {
        Ok(demangled) => {
            let formatted = format_function(&demangled, format.clone(), strip_module);
            if let Err(e) = writeln!(output, "{}", formatted) {
                eprintln!("Error writing output: {}", e);
            }
        }
        Err(e) => {
            eprintln!("Error demangling '{}': {}", name, e);
        }
    }
}

fn run() -> anyhow::Result<()> {
    let args = Args::parse();

    let mut output: Box<dyn Write> = if let Some(output_file) = &args.output {
        Box::new(std::fs::File::create(output_file)?)
    } else {
        Box::new(io::stdout())
    };

    if args.should_read_stdin() {
        let stdin = io::stdin();
        for line in stdin.lock().lines() {
            let line = line?;
            let trimmed = line.trim();
            if !trimmed.is_empty() {
                process_name(trimmed, &mut output, &args.format, args.strip_module);
            }
        }
    } else {
        for name in &args.mangled_names {
            process_name(name, &mut output, &args.format, args.strip_module);
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
