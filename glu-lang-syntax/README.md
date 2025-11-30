# Glu Language Support for Visual Studio Code

This extension provides syntax highlighting and language support for the [Glu programming language](https://glu-lang.org/).

## Features

- **Syntax Highlighting**: Full syntax highlighting for Glu language constructs including:
  - Keywords (if, else, while, for, return, etc.)
  - Types (Int, String, Bool, custom types, etc.)
  - Functions and function calls
  - Operators (arithmetic, logical, bitwise, comparison, etc.)
  - Literals (numbers, strings, booleans)
  - Comments (line and block)
  - Attributes (@inline, @no_mangling, etc.)
  - Preprocessor directives (#define, #include, etc.)
  
- **Language Configuration**: Auto-closing pairs, bracket matching, and comment toggling

## Installation

### From VSIX File

1. Download the latest `.vsix` file from the releases
2. Open Visual Studio Code
3. Go to Extensions (Ctrl+Shift+X / Cmd+Shift+X)
4. Click on the `...` menu and select "Install from VSIX..."
5. Select the downloaded `.vsix` file

### From Command Line

```bash
code --install-extension glu-syntax-0.1.0.vsix
```

## Building from Source

To build the extension from source:

1. Install `vsce` (Visual Studio Code Extension Manager):
   ```bash
   npm install -g @vscode/vsce
   ```

2. Package the extension:
   ```bash
   vsce package
   ```

3. Install the generated `.vsix` file:
   ```bash
   code --install-extension glu-syntax-0.1.0.vsix
   ```

## About Glu

Glu is a modern systems programming language designed as a "glue" language for connecting and interoperating with other languages and systems via LLVM. Learn more at [glu-lang.org](https://glu-lang.org/).

## Contributing

Contributions are welcome! Please see the main [Glu repository](https://github.com/glu-lang/glu) for contribution guidelines.

## License

This extension is distributed under the Apache License 2.0. See the [LICENSE](../LICENSE) file for more details.