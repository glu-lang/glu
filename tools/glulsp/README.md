# Glu Language Server Protocol (LSP) Support

This directory contains the Language Server Protocol implementation for the Glu programming language, providing rich IDE features through VSCode and other LSP-compatible editors.

## Features

- **Syntax Highlighting**: Full syntax highlighting for Glu language constructs
- **Diagnostics**: Real-time error and warning reporting from the Glu compiler
- **Document Symbols**: Navigate to functions, structs, and enums
- **Hover Information**: Basic type information on hover (framework ready)
- **LSP Server**: Complete LSP server implementation (`glulsp`)

## Components

### 1. LSP Server (`tools/glulsp/`)

The LSP server is implemented in C++ and integrates directly with the Glu compiler infrastructure:

- **JSONRPCHandler**: Handles LSP communication protocol
- **DocumentManager**: Manages open documents and their analysis
- **LSPServer**: Main server implementation with LSP method handlers

### 2. VSCode Extension (`vscode-extension/`)

A complete VSCode extension providing:

- Language configuration (brackets, comments, indentation)
- TextMate grammar for syntax highlighting
- LSP client integration
- Automatic server discovery and management

## Building

### Prerequisites

- CMake 3.29+
- LLVM 18.1+
- nlohmann/json library
- Node.js and npm (for VSCode extension)

### Build LSP Server

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install nlohmann-json3-dev

# Build the project
mkdir build && cd build
cmake ..
make -j8

# LSP server binary will be at: build/tools/glulsp/sources/glulsp
```

### Build VSCode Extension

```bash
cd vscode-extension
npm install
npm run compile
```

## Installation

### Manual Installation

1. Build the LSP server (see above)
2. Build the VSCode extension
3. Install the extension in VSCode:
   ```bash
   code --install-extension vscode-extension/
   ```

### Configuration

The extension can be configured through VSCode settings:

```json
{
  "glu.languageServer.enabled": true,
  "glu.languageServer.path": "/path/to/glulsp"
}
```

If not specified, the extension will search for `glulsp` in:
1. `../../build/tools/glulsp/sources/glulsp` (relative to extension)
2. `bin/glulsp` (in extension directory)
3. `glulsp` (in system PATH)

## Usage

1. Install the VSCode extension
2. Open any `.glu` file
3. The extension will automatically start the LSP server
4. Enjoy syntax highlighting, diagnostics, and other IDE features

## Testing

Test the LSP server manually:

```bash
# Test LSP server
cd build
python3 ../test_lsp.py

# Test syntax highlighting with sample file
code ../test_sample.glu
```

## Architecture

The LSP implementation follows the Language Server Protocol specification and integrates with existing Glu compiler components:

```
VSCode Extension (TypeScript)
         ↓ JSON-RPC over stdio
LSP Server (C++)
    ├── JSONRPCHandler (protocol communication)
    ├── DocumentManager (document tracking)
    └── LSPServer (method handlers)
         ↓ uses existing compiler infrastructure
Glu Compiler Components
    ├── Lexer (tokenization)
    ├── Parser (AST generation)
    ├── Sema (semantic analysis)
    └── Diagnostics (error reporting)
```

## Supported LSP Features

- ✅ **textDocument/publishDiagnostics**: Error/warning reporting
- ✅ **textDocument/documentSymbol**: Navigate to symbols
- ✅ **textDocument/hover**: Type information (framework ready)
- ✅ **initialize/shutdown**: Server lifecycle management
- ✅ **textDocument/didOpen/didChange/didClose**: Document synchronization

## Future Enhancements

- **Go to Definition**: Navigate to symbol definitions
- **Code Completion**: Auto-complete suggestions
- **Formatting**: Code formatting support
- **Rename**: Symbol renaming
- **Find References**: Find all references to a symbol
- **Signature Help**: Function signature information

## Contributing

To add new LSP features:

1. Implement the handler in `LSPServer.cpp`
2. Add document processing logic in `DocumentManager.cpp`
3. Register the method in `LSPServer` constructor
4. Update the capabilities in `handleInitialize`

## License

This LSP implementation is part of the Glu language project and follows the same license terms.