# Installation Guide for Glu VSCode Extension

This guide provides detailed instructions for installing and using the Glu language support extension for Visual Studio Code.

## Prerequisites

- Visual Studio Code version 1.85.0 or higher
- (Optional) Node.js and npm for building from source

## Installation Methods

### Method 1: Install Pre-built Extension (Recommended)

1. **Download the Extension**
   
   Download the latest `glu-syntax-0.1.0.vsix` file from the `glu-lang-syntax` directory of the repository.

2. **Install via VSCode GUI**
   
   a. Open Visual Studio Code
   
   b. Go to the Extensions view by clicking the Extensions icon in the Activity Bar or pressing `Ctrl+Shift+X` (Windows/Linux) or `Cmd+Shift+X` (macOS)
   
   c. Click on the `...` (More Actions) button at the top of the Extensions view
   
   d. Select "Install from VSIX..."
   
   e. Navigate to and select the downloaded `glu-syntax-0.1.0.vsix` file
   
   f. Click "Install"

3. **Install via Command Line**
   
   ```bash
   code --install-extension /path/to/glu-syntax-0.1.0.vsix
   ```

4. **Verify Installation**
   
   - Open a `.glu` file in VSCode
   - The language mode in the bottom-right corner should show "Glu"
   - Syntax highlighting should be applied automatically

### Method 2: Build and Install from Source

If you want to build the extension from source or make modifications:

1. **Install Dependencies**
   
   ```bash
   npm install -g @vscode/vsce
   ```

2. **Navigate to Extension Directory**
   
   ```bash
   cd /path/to/glu/glu-lang-syntax
   ```

3. **Package the Extension**
   
   ```bash
   vsce package
   ```
   
   Or using npx (no installation required):
   
   ```bash
   npx @vscode/vsce package
   ```
   
   This will create a `glu-syntax-0.1.0.vsix` file in the current directory.

4. **Install the Extension**
   
   ```bash
   code --install-extension glu-syntax-0.1.0.vsix
   ```

## Testing the Extension

1. **Open a Glu File**
   
   Create or open a file with the `.glu` extension.

2. **Verify Syntax Highlighting**
   
   The following should be highlighted:
   
   - **Keywords**: `func`, `struct`, `enum`, `if`, `else`, `while`, `for`, `return`, etc.
   - **Types**: `Int`, `String`, `Bool`, `Float32`, custom types
   - **Attributes**: `@inline`, `@no_mangling`
   - **Operators**: `+`, `-`, `*`, `/`, `==`, `!=`, `&&`, `||`, etc.
   - **Literals**: Numbers, strings, booleans
   - **Comments**: `//` and `/* */`
   - **Preprocessor**: `#define`, `#include`

3. **Test Language Features**
   
   - Try typing `//` and verify comment highlighting
   - Type `func` and verify keyword highlighting
   - Try auto-closing pairs: `{`, `[`, `(`

## Troubleshooting

### Extension Not Activated

If the extension doesn't activate:

1. Check that the file extension is `.glu`
2. Reload VSCode: Press `Ctrl+Shift+P` (Windows/Linux) or `Cmd+Shift+P` (macOS), type "Reload Window", and press Enter
3. Check the Extension view to ensure the extension is enabled

### No Syntax Highlighting

1. Verify the language mode in the bottom-right corner shows "Glu"
2. Manually select the language: Click the language mode indicator and select "Glu"
3. Try reinstalling the extension

### Colors Look Wrong

Syntax highlighting colors are controlled by your VSCode theme. Try different themes to see which works best:

1. Press `Ctrl+K Ctrl+T` (Windows/Linux) or `Cmd+K Cmd+T` (macOS)
2. Select a different color theme

Recommended themes for Glu:
- Dark+
- Monokai
- One Dark Pro

## Uninstalling

To uninstall the extension:

1. Go to the Extensions view (`Ctrl+Shift+X` / `Cmd+Shift+X`)
2. Find "Glu Language Support" in the list
3. Click the gear icon and select "Uninstall"

Or via command line:

```bash
code --uninstall-extension glu-lang.glu-syntax
```

## Development and Contributing

To develop or contribute to the extension:

1. Clone the Glu repository
2. Navigate to `glu-lang-syntax` directory
3. Make your changes to:
   - `glu.tmLanguage.json` - Syntax highlighting rules
   - `language-configuration.json` - Language configuration
   - `package.json` - Extension metadata
4. Test your changes by packaging and installing the extension
5. Submit a pull request to the Glu repository

## Additional Resources

- [Glu Language Website](https://glu-lang.org/)
- [Glu Documentation](https://glu-lang.org/reference/)
- [VSCode Extension API](https://code.visualstudio.com/api)
- [TextMate Grammar Guide](https://macromates.com/manual/en/language_grammars)

## Support

For issues or questions:

- [GitHub Issues](https://github.com/glu-lang/glu/issues)
- [Glu Community](https://glu-lang.org/)
