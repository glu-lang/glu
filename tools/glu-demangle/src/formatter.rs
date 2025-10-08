use crate::types::{DisplayFormat, GluFunction, GluType};

/// Format a demangled function according to the display format
pub fn format_function(func: &GluFunction, format: DisplayFormat, strip_module: bool) -> String {
    match format {
        DisplayFormat::Signature => format_signature(func, strip_module),
        DisplayFormat::Verbose => format_verbose(func, strip_module),
        DisplayFormat::NameOnly => format_name_only(func, strip_module),
        DisplayFormat::TypeOnly => format_type_only(func, strip_module),
    }
}

/// Format function in signature style
///
/// # Examples
/// ```
/// use glu_demangle::formatter::format_signature;
/// use glu_demangle::types::{GluFunction, GluType};
///
/// let func = GluFunction {
///     module_path: vec!["test".to_string()],
///     name: "foo".to_string(),
///     return_type: GluType::Void,
///     parameters: vec![GluType::Int { signed: true, width: 32 }],
/// };
/// let result = format_signature(&func, false);
/// assert_eq!(result, "test::foo(Int32) -> Void");
///
/// // With module stripping
/// let result_stripped = format_signature(&func, true);
/// assert_eq!(result_stripped, "foo(Int32) -> Void");
/// ```
pub fn format_signature(func: &GluFunction, strip_module: bool) -> String {
    let name = format_qualified_name(&func.module_path, &func.name, strip_module);

    let params = func.parameters.iter()
        .map(|t| format_type(t, strip_module))
        .collect::<Vec<_>>()
        .join(", ");

    format!("{}({}) -> {}", name, params, format_type(&func.return_type, strip_module))
}

/// Format function in verbose style
fn format_verbose(func: &GluFunction, strip_module: bool) -> String {
    let name = format_qualified_name(&func.module_path, &func.name, strip_module);

    let params = func.parameters.iter()
        .map(|t| format_type_verbose(t, strip_module))
        .collect::<Vec<_>>()
        .join(", ");

    format!("Function {}:\n  Type: function taking ({}) returning {}",
            name, params, format_type_verbose(&func.return_type, strip_module))
}

/// Format function name only
///
/// # Examples
/// ```
/// use glu_demangle::formatter::format_name_only;
/// use glu_demangle::types::{GluFunction, GluType};
///
/// let func = GluFunction {
///     module_path: vec!["test".to_string(), "module".to_string()],
///     name: "foo".to_string(),
///     return_type: GluType::Void,
///     parameters: vec![],
/// };
///
/// assert_eq!(format_name_only(&func, false), "test::module::foo");
/// assert_eq!(format_name_only(&func, true), "foo");
/// ```
pub fn format_name_only(func: &GluFunction, strip_module: bool) -> String {
    format_qualified_name(&func.module_path, &func.name, strip_module)
}

/// Format function type only
fn format_type_only(func: &GluFunction, strip_module: bool) -> String {
    let params = func.parameters.iter()
        .map(|t| format_type(t, strip_module))
        .collect::<Vec<_>>()
        .join(", ");

    format!("({}) -> {}", params, format_type(&func.return_type, strip_module))
}

/// Helper function to format a qualified name (module path + name)
fn format_qualified_name(module_path: &[String], name: &str, strip_module: bool) -> String {
    if strip_module {
        name.to_string()
    } else {
        format!("{}::{}", module_path.join("::"), name)
    }
}

/// Format a type in short form
///
/// # Examples
/// ```
/// use glu_demangle::formatter::format_type;
/// use glu_demangle::types::GluType;
///
/// assert_eq!(format_type(&GluType::Void, false), "Void");
/// assert_eq!(format_type(&GluType::Bool, false), "Bool");
/// assert_eq!(format_type(&GluType::Int { signed: true, width: 32 }, false), "Int32");
/// assert_eq!(format_type(&GluType::Int { signed: false, width: 64 }, false), "UInt64");
/// assert_eq!(format_type(&GluType::Float { width: 32 }, false), "Float32");
///
/// // Pointer type
/// let ptr_type = GluType::Pointer(Box::new(GluType::Int { signed: true, width: 32 }));
/// assert_eq!(format_type(&ptr_type, false), "*Int32");
///
/// // Array type
/// let array_type = GluType::StaticArray {
///     size: 10,
///     element_type: Box::new(GluType::Int { signed: true, width: 32 })
/// };
/// assert_eq!(format_type(&array_type, false), "Int32[10]");
/// ```
pub fn format_type(ty: &GluType, strip_module: bool) -> String {
    match ty {
        GluType::Void => "Void".to_string(),
        GluType::Bool => "Bool".to_string(),
        GluType::Char => "Char".to_string(),
        GluType::Null => "Null".to_string(),
        GluType::DynamicArray => "[]".to_string(),
        GluType::Int { signed, width } => {
            if *signed { format!("Int{}", width) } else { format!("UInt{}", width) }
        }
        GluType::Float { width } => format!("Float{}", width),
        GluType::Pointer(pointee) => format!("*{}", format_type(pointee, strip_module)),
        GluType::StaticArray { size, element_type } => {
            format!("{}[{}]", format_type(element_type, strip_module), size)
        }
        GluType::Function { return_type, params } => {
            let params_str = params.iter()
                .map(|p| format_type(p, strip_module))
                .collect::<Vec<_>>()
                .join(", ");
            format!("({}) -> {}", params_str, format_type(return_type, strip_module))
        }
        GluType::Struct { module_path, name } | GluType::Enum { module_path, name } => {
            format_qualified_name(module_path, name, strip_module)
        }
    }
}

/// Format a type in verbose form
///
/// # Examples
/// ```
/// use glu_demangle::formatter::format_type_verbose;
/// use glu_demangle::types::GluType;
///
/// assert_eq!(format_type_verbose(&GluType::Bool, false), "boolean");
/// assert_eq!(format_type_verbose(&GluType::Int { signed: true, width: 32 }, false), "signed integer (32 bits)");
/// assert_eq!(format_type_verbose(&GluType::Int { signed: false, width: 64 }, false), "unsigned integer (64 bits)");
/// assert_eq!(format_type_verbose(&GluType::Float { width: 64 }, false), "floating point (64 bits)");
///
/// // Pointer type
/// let ptr_type = GluType::Pointer(Box::new(GluType::Bool));
/// assert_eq!(format_type_verbose(&ptr_type, false), "pointer to boolean");
/// ```
pub fn format_type_verbose(ty: &GluType, strip_module: bool) -> String {
    match ty {
        GluType::Void => "void".to_string(),
        GluType::Bool => "boolean".to_string(),
        GluType::Char => "character".to_string(),
        GluType::Null => "null type".to_string(),
        GluType::DynamicArray => "dynamic array".to_string(),
        GluType::Int { signed, width } => {
            if *signed {
                format!("signed integer ({} bits)", width)
            } else {
                format!("unsigned integer ({} bits)", width)
            }
        }
        GluType::Float { width } => format!("floating point ({} bits)", width),
        GluType::Pointer(pointee) => format!("pointer to {}", format_type_verbose(pointee, strip_module)),
        GluType::StaticArray { size, element_type } => {
            format!("array of {} {}", size, format_type_verbose(element_type, strip_module))
        }
        GluType::Function { return_type, params } => {
            let params_str = params.iter()
                .map(|p| format_type_verbose(p, strip_module))
                .collect::<Vec<_>>()
                .join(", ");
            format!("function taking ({}) returning {}", params_str, format_type_verbose(return_type, strip_module))
        }
        GluType::Struct { module_path, name } => {
            if strip_module {
                format!("struct {}", name)
            } else {
                format!("struct {}::{}", module_path.join("::"), name)
            }
        }
        GluType::Enum { module_path, name } => {
            if strip_module {
                format!("enum {}", name)
            } else {
                format!("enum {}::{}", module_path.join("::"), name)
            }
        }
    }
}
