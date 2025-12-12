#[derive(Debug, Clone)]
pub enum DisplayFormat {
    /// Full signature: `module::name(param1, param2) -> return_type`
    Signature,
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
            "verbose" | "v" => Ok(DisplayFormat::Verbose),
            "name" | "n" | "name-only" => Ok(DisplayFormat::NameOnly),
            "type" | "t" | "type-only" => Ok(DisplayFormat::TypeOnly),
            _ => Err(format!(
                "Invalid display format: {}. Valid options: signature, verbose, name, type",
                s
            )),
        }
    }
}

#[derive(Debug, PartialEq)]
pub struct GluFunction {
    pub module_path: Vec<String>,
    pub name: String,
    pub func_type: GluType,
}

#[derive(Debug, PartialEq)]
pub enum GlobalKind {
    /// Storage variable
    Storage,
    /// Accessor function
    Accessor,
    /// Eager constructor
    Constructor,
    /// Destructor
    Destructor,
    /// Init function
    Init,
    /// Set bit
    SetBit,
}

#[derive(Debug, PartialEq)]
pub struct GluGlobal {
    pub module_path: Vec<String>,
    pub name: String,
    pub kind: GlobalKind,
    pub var_type: GluType,
}

#[derive(Debug, PartialEq)]
pub enum GluSymbol {
    Function(GluFunction),
    Global(GluGlobal),
}

#[derive(Debug, PartialEq)]
pub enum GluType {
    Void,
    Bool,
    Char,
    Int {
        signed: bool,
        width: u32,
    },
    Float {
        width: u32,
    },
    Pointer(Box<GluType>),
    StaticArray {
        size: usize,
        element_type: Box<GluType>,
    },
    DynamicArray,
    Function {
        return_type: Box<GluType>,
        params: Vec<GluType>,
    },
    Struct {
        module_path: Vec<String>,
        name: String,
    },
    Enum {
        module_path: Vec<String>,
        name: String,
    },
    Null,
}
