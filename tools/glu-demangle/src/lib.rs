pub mod formatter;
pub mod parser;
pub mod types;

pub use formatter::{format_function, format_global};
pub use types::{DisplayFormat, GlobalKind, GluFunction, GluGlobal, GluSymbol, GluType};

use parser::Parser;

fn strip_mangling_prefix(mangled: &str) -> &str {
    // Strip $GLU$ prefix if present, otherwise use as-is
    // Also handle _ prefix used on most platforms
    if mangled.starts_with("_$GLU$") {
        &mangled[6..]
    } else if mangled.starts_with("$GLU$") {
        &mangled[5..]
    } else {
        mangled
    }
}

/// Parse a mangled GLU symbol (function or global variable)
///
/// # Examples
/// ```
/// use glu_demangle::{demangle_symbol, GluSymbol, GluType, GlobalKind};
///
/// // Function
/// let result = demangle_symbol("$GLU$4main4testFvR").unwrap();
/// match result {
///     GluSymbol::Function(func) => {
///         assert_eq!(func.name, "test");
///     },
///     _ => panic!("Expected function"),
/// }
///
/// // Global storage
/// let result = demangle_symbol("$GLU$4main3fooGsi32").unwrap();
/// match result {
///     GluSymbol::Global(global) => {
///         assert_eq!(global.name, "foo");
///         assert_eq!(global.kind, GlobalKind::Storage);
///         assert_eq!(global.var_type, GluType::Int { signed: true, width: 32 });
///     },
///     _ => panic!("Expected global"),
/// }
/// ```
pub fn demangle_symbol(mangled: &str) -> anyhow::Result<GluSymbol> {
    let mut parser = Parser::new(strip_mangling_prefix(mangled));
    parser.parse_symbol()
}

/// Parse a mangled GLU function name
///
/// # Examples
/// ```
/// use glu_demangle::{demangle, GluType};
///
/// // Simple void function with no parameters
/// let result = demangle("$GLU$4main4testFvR").unwrap();
/// assert_eq!(result.module_path, vec!["main"]);
/// assert_eq!(result.name, "test");
/// if let GluType::Function { return_type, params } = &result.func_type {
///     assert_eq!(**return_type, GluType::Void);
///     assert_eq!(params, &vec![]);
/// }
///
/// // Function with integer parameters and return type
/// let result = demangle("$GLU$4math3addFi32i32i32R").unwrap();
/// assert_eq!(result.module_path, vec!["math"]);
/// assert_eq!(result.name, "add");
/// if let GluType::Function { return_type, params } = &result.func_type {
///     assert_eq!(**return_type, GluType::Int { signed: true, width: 32 });
///     assert_eq!(params, &vec![
///         GluType::Int { signed: true, width: 32 },
///         GluType::Int { signed: true, width: 32 }
///     ]);
/// }
///
/// // Nested module path
/// let result = demangle("$GLU$3std2io5printFvR").unwrap();
/// assert_eq!(result.module_path, vec!["std", "io"]);
/// assert_eq!(result.name, "print");
///
/// // Function with pointer types
/// let result = demangle("$GLU$4test3fooFPi32Pi32R").unwrap();
/// assert_eq!(result.module_path, vec!["test"]);
/// assert_eq!(result.name, "foo");
/// if let GluType::Function { return_type, .. } = &result.func_type {
///     if let GluType::Pointer(pointee) = &**return_type {
///         assert_eq!(**pointee, GluType::Int { signed: true, width: 32 });
///     }
/// }
///
/// // Without $GLU$ prefix (for terminal copy-paste)
/// let result = demangle("4main4testFvR").unwrap();
/// assert_eq!(result.module_path, vec!["main"]);
/// assert_eq!(result.name, "test");
///
/// // Invalid mangled name
/// let result = demangle("INVALID$4main4testFvR");
/// assert!(result.is_err());
/// ```
pub fn demangle(mangled: &str) -> anyhow::Result<GluFunction> {
    let mut parser = Parser::new(strip_mangling_prefix(mangled));
    parser.parse_function()
}
