# Glu VSCode Extension Features

This document describes all the syntax highlighting features provided by the Glu Language Support extension for Visual Studio Code.

## Supported Language Features

### 1. Keywords

#### Control Flow Keywords
- `if`, `else` - Conditional statements
- `while`, `for` - Loop statements
- `return` - Return from function
- `break`, `continue` - Loop control
- `in` - For-in loop syntax

#### Declaration Keywords
- `func` - Function declaration
- `struct` - Structure type declaration
- `union` - Union type declaration
- `enum` - Enumeration type declaration
- `typealias` - Type alias declaration
- `let` - Immutable variable declaration
- `var` - Mutable variable declaration

#### Module Keywords
- `import` - Import module or symbol
- `public` - Public visibility modifier
- `private` - Private visibility modifier

#### Type Keywords
- `unique` - Unique pointer type
- `shared` - Shared pointer type
- `as` - Type cast operator

#### Literal Keywords
- `true`, `false` - Boolean literals
- `null` - Null pointer literal

### 2. Primitive Types

The extension highlights all built-in Glu types:

#### Integer Types
- `Int`, `Int8`, `Int16`, `Int32`, `Int64` - Signed integers
- `UInt8`, `UInt16`, `UInt32`, `UInt64` - Unsigned integers

#### Floating Point Types
- `Float16`, `Float32`, `Float64` - Floating point numbers

#### Other Primitive Types
- `Char` - Character type
- `String` - String type
- `Bool` - Boolean type
- `Void` - Void type (no return value)

#### User-Defined Types
The extension automatically highlights any identifier starting with an uppercase letter as a type (e.g., `Point`, `Color`, `MyStruct`).

### 3. Operators

#### Arithmetic Operators
- `+`, `-`, `*`, `/`, `%` - Basic arithmetic
- Unary `+`, `-` - Positive/negative

#### Comparison Operators
- `==`, `!=` - Equality comparison
- `<`, `<=`, `>`, `>=` - Relational comparison

#### Logical Operators
- `&&` - Logical AND
- `||` - Logical OR
- `!` - Logical NOT

#### Bitwise Operators
- `&` - Bitwise AND
- `|` - Bitwise OR
- `^` - Bitwise XOR
- `~` - Bitwise NOT (complement)
- `<<` - Left shift
- `>>` - Right shift

#### Range Operators
- `...` - Inclusive range (e.g., `0...10`)
- `..<` - Exclusive range (e.g., `0..<10`)

#### Pointer Operators
- `.*` - Pointer dereference

#### Other Operators
- `=` - Assignment
- `->` - Function return type
- `::` - Scope resolution
- `::<` - Generic scope resolution
- `.` - Member access
- `?` - Ternary operator

#### Punctuation
- `,` - Comma separator
- `;` - Statement terminator
- `:` - Type annotation

### 4. Literals

#### Integer Literals
```glu
42              // Decimal
0xFF            // Hexadecimal
0b1010          // Binary
0o755           // Octal
1_000_000       // Underscore separators
```

#### Floating Point Literals
```glu
3.14            // Basic float
1.5e-10         // Scientific notation (lowercase)
2.0E+5          // Scientific notation (uppercase)
3_141.592_653   // Underscore separators
```

#### String Literals
```glu
"Hello, World!"              // Basic string
"Line 1\nLine 2"            // Escape sequences
"Quote: \"test\""           // Escaped quotes
```

#### Character Literals
```glu
'A'             // Basic character
'\n'            // Escape sequence
'\t'            // Tab character
```

#### Boolean Literals
```glu
true
false
```

#### Null Literal
```glu
null
```

### 5. Comments

#### Line Comments
```glu
// This is a single-line comment
```

#### Block Comments
```glu
/* This is a
   multi-line
   block comment */
```

#### Nested Block Comments
```glu
/* Outer comment
   /* Nested comment */
   Still outer comment */
```

### 6. Attributes

Attributes are special annotations that modify declarations:

```glu
@inline                     // Inline function hint
@no_mangling               // Disable name mangling
@deprecated                // Mark as deprecated
```

Attributes are highlighted with the `@` symbol and can be placed before declarations.

### 7. Preprocessor Directives

The extension supports C-style preprocessor directives:

```glu
#define MAX_SIZE 100
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#include "header.glu"
#ifdef DEBUG
#ifndef RELEASE
#else
#elif
#endif
#undef
```

Multi-line macros using backslash continuation are also supported:

```glu
#define MACRO(x) \
    statement1; \
    statement2;
```

### 8. Functions

#### Function Declarations
```glu
func functionName(param: Type) -> ReturnType {
    // body
}
```

#### Function Calls
```glu
functionName(argument)
```

Function names in calls are highlighted distinctly from keywords and variables.

### 9. Advanced Features

#### Type Annotations
```glu
let variable: Int = 10;
var mutable: String = "hello";
```

#### Generic Syntax
```glu
array::<Int>::create(10)
```

#### Pointer Types
```glu
func process(ptr: *Int) -> Void {
    let value = .*ptr;
}
```

#### Member Access
```glu
struct.field
object.method()
```

#### Range-Based Loops
```glu
for i in 0...10 {
    printLine(i);
}

for i in 0..<10 {
    printLine(i);
}
```

## Color Scheme

The extension uses semantic token types that work with all VSCode themes. The typical color mapping is:

- **Keywords**: Purple/Magenta
- **Types**: Green/Cyan
- **Strings**: Orange/Red
- **Numbers**: Light Green
- **Comments**: Gray/Muted
- **Functions**: Yellow
- **Operators**: White/Default
- **Attributes**: Yellow/Orange
- **Preprocessor**: Purple/Magenta

Note: Actual colors depend on your selected VSCode theme.

## Language Configuration

The extension also provides:

- **Auto-closing pairs**: Automatically closes `{}`, `[]`, `()`, `""`, `''`
- **Bracket matching**: Highlights matching brackets
- **Comment toggling**: Use `Ctrl+/` to toggle line comments
- **Block comment toggling**: Use `Shift+Alt+A` to toggle block comments

## Limitations

- The extension provides syntax highlighting only, not semantic analysis
- IntelliSense, code completion, and error checking are not included
- Jump-to-definition and other language server features are not available

## Future Enhancements

Potential future additions:
- Code snippets for common Glu patterns
- Language server support for IntelliSense
- Debugger integration
- Build task integration
- Symbol outline support
