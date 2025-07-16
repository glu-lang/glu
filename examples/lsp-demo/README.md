# Glu LSP Demo Workspace

This directory contains example Glu files to demonstrate the Language Server Protocol features.

## Files

- `demo.glu` - Main demo file showcasing various Glu language features
- `simple.glu` - Simple example for basic testing
- `errors.glu` - File with intentional errors to test diagnostics

## How to Use

1. Build the Glu project and LSP server
2. Install the VSCode extension
3. Open this directory in VSCode
4. Open any `.glu` file to see:
   - Syntax highlighting
   - Document symbols in the outline
   - Error diagnostics
   - Hover information

## Features Demonstrated

- **Syntax Highlighting**: Keywords, types, functions, comments
- **Document Symbols**: Functions, structs, enums appear in outline
- **Diagnostics**: Errors and warnings from the compiler
- **Hover Information**: Type information on hover

## Testing the LSP

To test the LSP server manually:

```bash
# From the build directory
python3 ../test_lsp.py

# Or test with a specific file
./tools/glulsp/sources/glulsp < test_request.json
```

## Expected Behavior

When you open `demo.glu` in VSCode with the extension:

1. **Syntax Highlighting**: All keywords, types, and constructs should be properly colored
2. **Document Symbols**: The outline should show:
   - `calculateArea` function
   - `main` function
   - `Point` struct
   - `Rectangle` struct
   - `Color` enum
   - `Status` enum
   - `processPoint` function
   - `errorExample` function
   - `complexCalculation` function

3. **Diagnostics**: Any syntax errors or type mismatches will be underlined in red
4. **Hover**: Hovering over symbols may show type information (basic framework is ready)

## Troubleshooting

If features aren't working:

1. Check that the LSP server is running (look for `glulsp` process)
2. Check the Output panel for error messages
3. Verify the extension is loaded (`Glu Language Support`)
4. Try restarting VSCode or reloading the window