use clap::Parser;
use glu_demangle::DisplayFormat;

#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
pub struct Args {
    /// Output format: signature (s), compact (c), verbose (v), name (n), type (t)
    #[arg(short, long, default_value = "signature")]
    pub format: DisplayFormat,

    /// Strip module information from output
    #[arg(short, long)]
    pub strip_module: bool,

    /// Read mangled names from stdin
    #[arg(long)]
    pub stdin: bool,

    /// Mangled function names to demangle
    pub mangled_names: Vec<String>,

    /// Output file (default: stdout)
    #[arg(short, long)]
    pub output: Option<String>,
}

impl Args {
    pub fn should_read_stdin(&self) -> bool {
        self.stdin || self.mangled_names.is_empty()
    }
}
