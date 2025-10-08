use clap::{Arg, Command};
use crate::types::DisplayFormat;

#[derive(Debug)]
pub struct Args {
    pub format: DisplayFormat,
    pub strip_module: bool,
    pub stdin: bool,
    pub mangled_names: Vec<String>,
}

impl Args {
    pub fn parse() -> anyhow::Result<Self> {
        let matches = build_command().get_matches();
        let format: DisplayFormat = matches
            .get_one::<String>("format")
            .ok_or_else(|| anyhow::anyhow!("Format argument is required"))?
            .parse()
            .map_err(|e| anyhow::anyhow!("{}", e))?;

        let strip_module = matches.get_flag("strip-module");
        let stdin = matches.get_flag("stdin");

        let mangled_names = if let Some(names) = matches.get_many::<String>("mangled-names") {
            names.cloned().collect()
        } else {
            Vec::new()
        };

        let stdin = stdin || mangled_names.is_empty();

        Ok(Args {
            format,
            strip_module,
            stdin,
            mangled_names,
        })
    }
}

fn build_command() -> Command {
    Command::new("glu-demangle")
        .version("0.1.0")
        .author("Glu Language Team")
        .about("Demangles Glu language function names")
        .arg(
            Arg::new("format")
                .short('f')
                .long("format")
                .value_name("FORMAT")
                .help("Output format: signature (s), compact (c), verbose (v), name (n), type (t)")
                .default_value("signature")
        )
        .arg(
            Arg::new("strip-module")
                .short('s')
                .long("strip-module")
                .action(clap::ArgAction::SetTrue)
                .help("Strip module information from output")
        )
        .arg(
            Arg::new("mangled-names")
                .value_name("NAMES")
                .help("Mangled function names to demangle")
                .num_args(0..)
        )
        .arg(
            Arg::new("stdin")
                .long("stdin")
                .action(clap::ArgAction::SetTrue)
                .help("Force reading from stdin (default if no arguments provided)")
        )
}
