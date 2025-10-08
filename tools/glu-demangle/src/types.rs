#[derive(Debug, Clone)]
pub enum DisplayFormat {
    /// Full signature: `module::name(param1, param2) -> return_type`
    Signature,
    /// Compact format: `module::name: (param1, param2) -> return_type`
    Compact,
    /// Verbose format with detailed type descriptions
    Verbose,
    /// Name only: `module::name`
    NameOnly,
    /// Type only: `(param1, param2) -> return_type`
    TypeOnly,
}

impl std::str::FromStr for DisplayFormat {
    type Err = String;

    fn from_str(s: &str) -> std::result::Result<Self, Self::Err> {
        match s.to_lowercase().as_str() {
            "signature" | "sig" | "s" => Ok(DisplayFormat::Signature),
            "compact" | "c" => Ok(DisplayFormat::Compact),
            "verbose" | "v" => Ok(DisplayFormat::Verbose),
            "name" | "n" | "name-only" => Ok(DisplayFormat::NameOnly),
            "type" | "t" | "type-only" => Ok(DisplayFormat::TypeOnly),
            _ => Err(format!("Invalid display format: {}. Valid options: signature, compact, verbose, name, type", s)),
        }
    }
}
