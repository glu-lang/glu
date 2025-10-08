pub mod types;
pub mod parser;
pub mod formatter;

pub use types::{DisplayFormat, GluFunction, GluType};
pub use formatter::format_function;

use parser::Parser;

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
/// assert_eq!(result.return_type, GluType::Void);
/// assert_eq!(result.parameters, vec![]);
///
/// // Function with integer parameters and return type
/// let result = demangle("$GLU$4math3addFi32i32i32R").unwrap();
/// assert_eq!(result.module_path, vec!["math"]);
/// assert_eq!(result.name, "add");
/// assert_eq!(result.return_type, GluType::Int { signed: true, width: 32 });
/// assert_eq!(result.parameters, vec![
///     GluType::Int { signed: true, width: 32 },
///     GluType::Int { signed: true, width: 32 }
/// ]);
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
/// if let GluType::Pointer(pointee) = &result.return_type {
///     assert_eq!(**pointee, GluType::Int { signed: true, width: 32 });
/// }
///
/// // Invalid mangled name
/// let result = demangle("INVALID$4main4testFvR");
/// assert!(result.is_err());
/// ```
pub fn demangle(mangled: &str) -> anyhow::Result<GluFunction> {
    if !mangled.starts_with("$GLU$") {
        return Err(anyhow::anyhow!("Invalid GLU mangled name"));
    }

    let mut parser = Parser::new(&mangled[5..]);
    parser.parse_function()
}
