#ifndef GLU_COMPILER_DRIVER_HPP
#define GLU_COMPILER_DRIVER_HPP

#include <optional>
#include <string>
#include <vector>

#include "AST/ASTContext.hpp"
#include "Basic/Diagnostic.hpp"
#include "Basic/SourceLocation.hpp"
#include "Basic/SourceManager.hpp"
#include "Decl/ModuleDecl.hpp"
#include "GIL/GILPrinter.hpp"
#include "Module.hpp"
#include "Parser/Parser.hpp"
#include "Scanner.hpp"
#include "Sema/ImportManager.hpp"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Allocator.h>
#include <llvm/Support/raw_ostream.h>

#include <memory>

namespace glu::driver {

enum Stage {
    PrintTokens,
    PrintASTGen,
    PrintConstraints,
    PrintAST,
    PrintInterface,
    PrintGILGen,
    PrintGIL,
    PrintLLVMIR,
    EmitBitcode,
    EmitAssembly,
    EmitObject,
    Linking
};

/// @brief Main compiler driver class that orchestrates the entire compilation
/// process from command line parsing through code generation and linking.
///
/// The CompilerDriver handles:
/// - Command line argument parsing and validation
/// - Initialization of compiler components (lexer, parser, semantic analyzer,
/// etc.)
/// - Management of the compilation pipeline (lexing -> parsing -> sema ->
/// codegen -> linking)
/// - Output stream management for various compiler outputs
/// - Error handling and diagnostics reporting
class CompilerDriver {

    /// @brief Configuration struct to hold all compiler options parsed from
    /// command line
    struct CompilerConfig {
        std::string inputFile; ///< Input source file path
        std::string outputFile; ///< Output file path (empty for stdout)
        std::vector<std::string>
            importDirs; ///< Additional import search directories
        std::string targetTriple; ///< Target architecture triple
        unsigned optLevel = 0; ///< Optimization level (0-3)
        Stage stage;
    };

    // Configuration and control flow
    CompilerConfig _config; ///< Parsed command line configuration
    char const *_argv0; ///< Program name for system path generation

    // Core compiler components (initialized during compilation)
    glu::SourceManager _sourceManager; ///< Manages source files and locations
    std::optional<glu::DiagnosticManager>
        _diagManager; ///< Handles error/warning reporting
    std::optional<glu::ast::ASTContext>
        _context; ///< AST memory management and context
    std::optional<glu::sema::ImportManager>
        _importManager; ///< Handles module imports

    // Code generation components
    llvm::BumpPtrAllocator _gilArena; ///< Memory arena for GIL functions
    llvm::LLVMContext _llvmContext; ///< LLVM context for IR generation
    std::optional<llvm::Module> _llvmModule; ///< Generated LLVM IR module

    // File and I/O management
    std::optional<llvm::ErrorOr<FileID>>
        _fileID; ///< Loaded source file identifier
    std::string _objectFile; ///< Path to generated object file
    llvm::raw_ostream
        *_outputStream; ///< Current output stream (file or stdout)
    std::unique_ptr<llvm::raw_fd_ostream>
        _outputFileStream; ///< File output stream when writing to file

    // AST and intermediate representations
    ModuleDecl *_ast = nullptr; ///< Parsed and analyzed AST
    std::optional<glu::gil::Module *>
        _gilModule; ///< Generated GIL intermediate representation

public:
    /// @brief Constructs a new CompilerDriver with default settings
    CompilerDriver() : _outputStream(&llvm::outs()) { }

    /// @brief Destructor that cleans up resources
    ~CompilerDriver() = default;

    /// @brief Main entry point that runs the complete compilation process
    /// @param argc Number of command line arguments
    /// @param argv Array of command line argument strings
    /// @return Exit code (0 for success, non-zero for error)
    int run(int argc, char **argv);

private:
    /// @brief Executes the core compilation pipeline
    /// @param argv0 Program name for system path generation
    /// @return Exit code (0 for success, non-zero for error)
    int executeCompilation();
    /// @brief Parse command line arguments and populate configuration
    /// @param argc Number of command line arguments
    /// @param argv Array of command line argument strings
    /// @return True if parsing was successful, false otherwise
    bool parseCommandLine(int argc, char **argv);

    /// @brief Generate system import paths based on the executable location
    /// @param argv0 The first argument from main (executable path)
    void generateSystemImportPaths();

    /// @brief Load the source file specified in the configuration with the
    /// source manager
    /// @return True if successful, false otherwise
    bool loadSourceFile();

    /// @brief Print tokens for debugging (when --print-tokens is specified)
    void printTokens();

    /// @brief Run the parser to generate the AST
    /// @return Exit code (0 for success, non-zero for error)
    int runParser();

    /// @brief Run semantic analysis on the AST
    /// @return Exit code (0 for success, non-zero for error)
    int runSema();

    /// @brief Run GIL generation from the AST
    /// @return Exit code (0 for success, non-zero for error)
    int runGILGen();

    /// @brief Run optimization passes on the GIL module
    /// @return Exit code (0 for success, non-zero for error)
    int runOptimizer();

    /// @brief Run LLVM IR generation from the GIL module
    /// @return Exit code (0 for success, non-zero for error)
    int runIRGen();

    /// @brief Compile the generated LLVM IR to object code or assembly
    /// @return Exit code (0 for success, non-zero for error)
    int compile();

    /// @brief Initialize LLVM target infrastructure for code generation
    void initializeLLVMTargets();

    /// @brief Apply LLVM optimization passes to the module
    void applyOptimizations();

    /// @brief Generate object code or assembly from LLVM IR
    /// @param emitAssembly If true, generate assembly; if false, generate
    /// object code
    void generateCode(bool emitAssembly);

    /// @brief Call the system linker to create executable from object file
    /// @return Exit code from linker (0 for success, non-zero for error)
    int callLinker();

    /// @brief Find object files from imported modules that need to be linked
    /// @return Vector of object file paths
    std::vector<std::string> findImportedObjectFiles();

    /// @brief Get the configured output stream (file or stdout)
    /// @return Reference to the output stream
    llvm::raw_ostream &getOutputStream() { return *_outputStream; }
};

}

#endif /* GLU_COMPILER_DRIVER_HPP */
