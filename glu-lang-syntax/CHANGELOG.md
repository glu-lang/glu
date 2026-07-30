# Changelog

All notable changes to the Glu Language Support extension will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2025-10-16

### Added

#### Syntax Highlighting
- Complete keyword support from Glu compiler's TokenKind.def
  - Control flow: `if`, `else`, `while`, `for`, `return`, `break`, `continue`, `in`
  - Declarations: `func`, `struct`, `union`, `enum`, `typealias`, `let`, `var`
  - Module system: `import`, `public`, `private`
  - Type modifiers: `unique`, `shared`
  - Type casting: `as`
  - Literals: `true`, `false`, `null`

- Comprehensive type highlighting
  - Signed integers: `Int`, `Int8`, `Int16`, `Int32`, `Int64`
  - Unsigned integers: `UInt8`, `UInt16`, `UInt32`, `UInt64`
  - Floating point: `Float16`, `Float32`, `Float64`
  - Other primitives: `Char`, `String`, `Bool`, `Void`
  - User-defined types (uppercase identifiers)

- Complete operator support
  - Arithmetic: `+`, `-`, `*`, `/`, `%`
  - Comparison: `==`, `!=`, `<`, `<=`, `>`, `>=`
  - Logical: `&&`, `||`, `!`
  - Bitwise: `&`, `|`, `^`, `~`, `<<`, `>>`
  - Range: `...`, `..<`
  - Pointer: `.*`
  - Scope: `::`, `::<`
  - Arrow: `->`
  - Assignment: `=`
  - Ternary: `?`

- Literal support
  - Decimal, hexadecimal (0x), binary (0b), and octal (0o) integers
  - Floating point with scientific notation
  - Underscore separators in numbers
  - String literals with escape sequences
  - Character literals with escape sequences
  - Boolean literals

- Comment support
  - Line comments (`//`)
  - Block comments (`/* */`)
  - Nested block comments

- Attribute support
  - Pattern matching for attributes like `@inline`, `@no_mangling`, etc.

- Preprocessor directive support
  - `#define`, `#undef`, `#include`
  - Conditional compilation: `#if`, `#ifdef`, `#ifndef`, `#else`, `#elif`, `#endif`
  - Multi-line macro support with backslash continuation

- Function call highlighting
  - Distinguishes function calls from other identifiers

#### Language Configuration
- Auto-closing pairs for brackets, parentheses, and quotes
- Bracket matching
- Comment toggling support
- Word pattern for better word selection

#### Documentation
- Comprehensive README.md with installation instructions
- INSTALL.md with detailed installation guide and troubleshooting
- FEATURES.md describing all syntax highlighting capabilities
- CHANGELOG.md for version tracking
- test-syntax.glu example file demonstrating all language features

#### Build System
- .vscodeignore file to exclude unnecessary files from package
- LICENSE symlink to main repository license
- Updated package.json with proper metadata

### Changed
- Updated package.json version from 0.0.1 to 0.1.0
- Renamed RADME.md to README.md (typo fix)
- Improved package.json with repository, license, and keywords metadata
- Enhanced .gitignore to exclude .vsix files

### Removed
- Old glu-syntax-0.0.1.vsix package (replaced with 0.1.0)

## [0.0.1] - Initial Release

### Added
- Basic syntax highlighting for Glu language
- Basic keyword support
- Basic type support
- Basic operator support
- Simple string and number literal support
- Line and block comment support
- Basic language configuration

---

## Planned for Future Releases

### [0.2.0] - Planned
- Code snippets for common Glu patterns
- Improved scope detection
- Better support for generic syntax
- Folding regions for functions and structures

### [0.3.0] - Planned
- Language server protocol (LSP) support
- IntelliSense and code completion
- Go to definition
- Find all references
- Hover information

### [0.4.0] - Planned
- Debugger support
- Build task integration
- Error diagnostics
- Symbol outline view

### [1.0.0] - Planned
- Stable release with all core features
- Full language server support
- Comprehensive documentation
- Performance optimizations
