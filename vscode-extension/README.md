# Glu Language Support for VSCode

This extension provides comprehensive language support for the Glu programming language in Visual Studio Code.

## Features

- **Syntax Highlighting**: Full syntax highlighting for Glu language constructs including:
  - Keywords (`func`, `struct`, `enum`, `let`, `var`, `if`, `while`, etc.)
  - Types (`Int`, `Float`, `Bool`, `String`, custom types)
  - Functions and identifiers
  - Comments, strings, and numbers
  - Operators and punctuation

- **Language Server**: Integrated Language Server Protocol (LSP) support providing:
  - Real-time error and warning diagnostics
  - Document symbol navigation
  - Hover information
  - Auto-completion (coming soon)

- **Editor Features**:
  - Bracket matching and auto-closing
  - Comment toggling
  - Indentation rules
  - Word pattern recognition

## Installation

### From Source

1. Clone the Glu repository
2. Build the LSP server:
   ```bash
   mkdir build && cd build
   cmake ..
   make -j8
   ```

3. Build the extension:
   ```bash
   cd vscode-extension
   npm install
   npm run compile
   ```

4. Install in VSCode:
   ```bash
   code --install-extension ./vscode-extension/
   ```

### Package the Extension

To create a `.vsix` package:

```bash
npm install -g vsce
cd vscode-extension
vsce package
```

## Configuration

The extension can be configured through VSCode settings:

```json
{
  "glu.languageServer.enabled": true,
  "glu.languageServer.path": "/path/to/glulsp"
}
```

### Settings

- `glu.languageServer.enabled`: Enable/disable the Glu Language Server (default: `true`)
- `glu.languageServer.path`: Path to the `glulsp` executable (auto-detected if not specified)

## Usage

1. Open any `.glu` file in VSCode
2. The extension will automatically:
   - Apply syntax highlighting
   - Start the language server
   - Provide real-time diagnostics
   - Enable document symbol navigation

## File Association

The extension automatically associates with `.glu` files and provides:

- Language ID: `glu`
- File extensions: `.glu`
- Icon: Glu language icon (if available)

## Commands

The extension provides the following commands:

- **Glu: Restart Language Server**: Restart the LSP server
- **Glu: Show Output**: Show the language server output logs

## Troubleshooting

### Language Server Not Starting

1. Check that `glulsp` is in your PATH or configured correctly
2. Verify the LSP server builds correctly
3. Check the Output panel for error messages

### Syntax Highlighting Issues

1. Ensure the file has a `.glu` extension
2. Check that the language mode is set to "Glu"
3. Try reloading the window

### Performance Issues

1. Close unused `.glu` files
2. Check if the language server is consuming too much memory
3. Consider disabling the language server for large files

## Sample Glu Code

```glu
// Function definition
func main() {
    let x: Int = 42;
    let message: String = "Hello, Glu!";
    
    let point = Point { x: 10, y: 20 };
    
    if x > 0 {
        return;
    }
}

// Struct definition
struct Point {
    x: Int,
    y: Int,
}

// Enum definition
enum Color {
    Red,
    Green,
    Blue,
}
```

## Development

### Building from Source

```bash
# Install dependencies
npm install

# Compile TypeScript
npm run compile

# Watch for changes
npm run watch
```

### Directory Structure

```
vscode-extension/
├── src/
│   └── extension.ts        # Main extension code
├── syntaxes/
│   └── glu.tmLanguage.json # TextMate grammar
├── out/                    # Compiled JavaScript
├── package.json           # Extension manifest
├── tsconfig.json          # TypeScript configuration
└── language-configuration.json # Language configuration
```

## Contributing

To contribute to the extension:

1. Fork the repository
2. Make your changes
3. Test thoroughly
4. Submit a pull request

## License

This extension is part of the Glu language project and follows the same license terms.

## Links

- [Glu Language](https://glu-lang.org)
- [Repository](https://github.com/glu-lang/glu)
- [Issues](https://github.com/glu-lang/glu/issues)
- [Language Server Protocol](https://microsoft.github.io/language-server-protocol/)