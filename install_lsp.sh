#!/bin/bash

# Glu Language Server Installation Script

set -e

echo "Installing Glu Language Server and VSCode Extension..."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ] || [ ! -d "tools/glulsp" ]; then
    print_error "Please run this script from the Glu project root directory"
    exit 1
fi

# Check dependencies
print_status "Checking dependencies..."

# Check for CMake
if ! command -v cmake &> /dev/null; then
    print_error "CMake is required but not installed"
    exit 1
fi

# Check for make
if ! command -v make &> /dev/null; then
    print_error "Make is required but not installed"
    exit 1
fi

# Check for Node.js and npm
if ! command -v node &> /dev/null || ! command -v npm &> /dev/null; then
    print_warning "Node.js and npm are required for VSCode extension"
    print_warning "Install them from https://nodejs.org/"
fi

# Check for nlohmann/json
if ! pkg-config --exists nlohmann_json 2>/dev/null; then
    print_warning "nlohmann/json library not found"
    print_warning "Install it with: sudo apt-get install nlohmann-json3-dev"
fi

# Build the LSP server
print_status "Building LSP server..."

if [ ! -d "build" ]; then
    mkdir build
fi

cd build
cmake ..
make -j$(nproc) glulsp

if [ $? -eq 0 ]; then
    print_status "LSP server built successfully: $(pwd)/tools/glulsp/sources/glulsp"
else
    print_error "Failed to build LSP server"
    exit 1
fi

cd ..

# Build VSCode extension
if command -v node &> /dev/null && command -v npm &> /dev/null; then
    print_status "Building VSCode extension..."
    
    cd vscode-extension
    npm install
    npm run compile
    
    if [ $? -eq 0 ]; then
        print_status "VSCode extension built successfully"
    else
        print_error "Failed to build VSCode extension"
        exit 1
    fi
    
    cd ..
    
    # Install extension if VSCode is available
    if command -v code &> /dev/null; then
        print_status "Installing VSCode extension..."
        code --install-extension ./vscode-extension/
        
        if [ $? -eq 0 ]; then
            print_status "VSCode extension installed successfully"
        else
            print_warning "Failed to install VSCode extension automatically"
            print_warning "You can install it manually from the Extensions view"
        fi
    else
        print_warning "VSCode not found. Install the extension manually:"
        print_warning "1. Open VSCode"
        print_warning "2. Go to Extensions view (Ctrl+Shift+X)"
        print_warning "3. Install from VSIX: $(pwd)/vscode-extension/"
    fi
else
    print_warning "Skipping VSCode extension build (Node.js/npm not found)"
fi

# Test the LSP server
print_status "Testing LSP server..."

if python3 test_lsp.py 2>/dev/null; then
    print_status "LSP server test passed"
else
    print_warning "LSP server test failed or Python3 not available"
fi

# Print usage information
print_status "Installation complete!"
echo ""
echo "Usage:"
echo "  LSP Server: $(pwd)/build/tools/glulsp/sources/glulsp"
echo "  Test files: $(pwd)/examples/lsp-demo/"
echo ""
echo "To test:"
echo "  1. Open VSCode"
echo "  2. Open a .glu file"
echo "  3. Verify syntax highlighting and error diagnostics work"
echo ""
echo "Configuration:"
echo "  Set 'glu.languageServer.path' in VSCode settings if needed"
echo ""
echo "Troubleshooting:"
echo "  - Check VSCode Output panel for error messages"
echo "  - Verify the LSP server path is correct"
echo "  - Make sure the extension is enabled"