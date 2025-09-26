#include "CompilerDriver.hpp"

#include "GILGen/GILGen.hpp"
#include "IRGen/IRGen.hpp"
#include "Parser/Parser.hpp"
#include "Sema/Sema.hpp"

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/WithColor.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

#include <cassert>

using namespace llvm::cl;

int main(int argc, char **argv);

namespace glu {

/// @brief Find object files for imported modules using ImportManager
std::vector<std::string> CompilerDriver::findImportedObjectFiles()
{
    std::vector<std::string> importedFiles;

    auto const &importedFilesMap = _importManager->getImportedFiles();
    auto *sourceManager = _importManager->getASTContext().getSourceManager();

    for (auto const &entry : importedFilesMap) {
        glu::FileID fileID = entry.first;
        llvm::StringRef filePath = sourceManager->getBufferName(fileID);
        if (filePath.ends_with(".glu")) {
            std::string objPath = filePath.str();
            objPath.replace(objPath.length() - 4, 4, ".o");

            if (llvm::sys::fs::exists(objPath)) {
                importedFiles.push_back(objPath);
            } else {
                llvm::WithColor::warning(llvm::errs())
                    << "Object file not found for imported module: " << objPath
                    << " (from " << filePath << ")\n";
            }
        }
    }

    return importedFiles;
}

bool CompilerDriver::parseCommandLine(int argc, char **argv)
{
    // Create all command line options locally
    opt<bool> PrintTokens(
        "print-tokens", desc("Print tokens after lexical analysis"), init(false)
    );

    opt<bool> PrintAST(
        "print-ast", desc("Print the AST after sema"), init(false)
    );

    opt<bool> PrintASTGen(
        "print-astgen", desc("Print the AST after parsing, before sema"),
        init(false)
    );

    opt<bool> PrintGIL(
        "print-gil", desc("Print GIL after generation"), init(false)
    );

    opt<bool> PrintLLVMIR(
        "print-llvm-ir", desc("Print LLVM IR after generation"), init(false)
    );

    opt<std::string> TargetTriple(
        "target", desc("Target triple"), value_desc("triple")
    );

    opt<unsigned> OptLevel(
        "O", desc("Optimization level (0-3)"), init(0), value_desc("level")
    );

    opt<bool> EmitAssembly("S", desc("Emit assembly code"), init(false));

    opt<bool> EmitObject("c", desc("Emit object file"), init(false));

    opt<std::string> OutputFilename(
        "o", desc("Redirect output to the specified file"),
        value_desc("filename")
    );

    list<std::string> ImportDirs(
        "I", desc("Add directory to import search path"), ZeroOrMore,
        value_desc("directory")
    );

    opt<std::string> InputFilename(
        Positional, Required, desc("<input glu file>")
    );

    // Parse the command line
    ParseCommandLineOptions(argc, argv, "Glu Compiler\n");

    // Store parsed values in config_ member
    _config = { .inputFile = InputFilename,
                .outputFile = OutputFilename,
                .targetTriple = TargetTriple,
                .optLevel = OptLevel,
                .printTokens = PrintTokens,
                .printAST = PrintAST,
                .printASTGen = PrintASTGen,
                .printGIL = PrintGIL,
                .printLLVMIR = PrintLLVMIR,
                .emitAssembly = EmitAssembly,
                .emitObject = EmitObject };

    _config.importDirs.assign(ImportDirs.begin(), ImportDirs.end());

    // Validate options
    if (_config.optLevel > 3) {
        llvm::errs() << "Error: Invalid optimization level " << _config.optLevel
                     << " (must be 0-3)\n";
        return false;
    }

    if (_config.emitAssembly && _config.emitObject) {
        llvm::errs() << "Error: Cannot specify both -S and -c\n";
        return false;
    }

    // Calculate if linking is needed
    _needsLinking = !_config.emitObject && !_config.emitAssembly
        && !_config.printTokens && !_config.printASTGen && !_config.printAST
        && !_config.printGIL && !_config.printLLVMIR;

    // Set up output stream based on configuration
    if (!_config.outputFile.empty()) {
        // Open the specified output file
        std::error_code EC;
        _outputFileStream = std::make_unique<llvm::raw_fd_ostream>(
            _config.outputFile, EC, llvm::sys::fs::OF_None
        );
        if (EC) {
            llvm::errs() << "Error opening output file '" << _config.outputFile
                         << "': " << EC.message() << "\n";
            return false;
        }
        _outputStream = _outputFileStream.get();
    } else {
        // Use stdout when no output file is specified
        _outputStream = &llvm::outs();
    }

    return true;
}

void CompilerDriver::initializeLLVMTargets()
{
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
}

void CompilerDriver::generateSystemImportPaths(char const *argv0)
{
    // Add system import path based on compiler location
    // If the driver is /usr/bin/gluc, we add /usr/lib/glu/ to the
    // import paths
    llvm::SmallString<128> compiler(
        llvm::sys::fs::getMainExecutable(argv0, (void *) main)
    );
    llvm::sys::path::remove_filename(compiler);
    llvm::sys::path::append(compiler, "..", "lib", "glu");
    llvm::sys::path::remove_dots(compiler, true);
    _config.importDirs.push_back(compiler.str().str());
}

void CompilerDriver::printTokens()
{
    for (glu::Token token = _scanner->nextToken();
         token.isNot(glu::TokenKind::eofTok); token = _scanner->nextToken()) {
        glu::SourceLocation loc
            = _sourceManager.getSourceLocFromStringRef(token.getLexeme());

        auto spellingLine = _sourceManager.getSpellingLineNumber(loc);
        auto spellingColumn = _sourceManager.getSpellingColumnNumber(loc);
        auto filepath = _sourceManager.getBufferName(loc);

        *_outputStream << filepath << ":" << spellingLine << ":"
                       << spellingColumn << ": <" << token.getKind() << ", \""
                       << token.getLexeme() << "\">\n";
    }
}

bool CompilerDriver::configureParser()
{
    _fileID.emplace(_sourceManager.loadFile(_config.inputFile));

    if (!(*_fileID)) {
        llvm::errs() << "Error loading " << _config.inputFile << ": "
                     << _fileID->getError().message() << "\n";
        return false;
    }

    _scanner.emplace(_sourceManager.getBuffer(**_fileID));

    _parser.emplace(
        _scanner.value(), _context.value(), _sourceManager, _diagManager.value()
    );
    return true;
}

int CompilerDriver::processPreCompilationOptions()
{
    _ast = llvm::cast<glu::ast::ModuleDecl>(_parser->getAST());

    assert(_ast && "AST should always exist if parse is successful");

    if (_config.printASTGen) {
        _ast->print(*_outputStream);
        return 0;
    }

    sema::constrainAST(_ast, *_diagManager, &(*_importManager));

    if (_config.printAST) {
        _ast->print(*_outputStream);
        return 0;
    }

    if (_diagManager->hasErrors()) {
        return 1;
    }

    glu::gilgen::GILGen gilgen;

    _gilModule.emplace(gilgen.generateModule(_ast, _GILFuncArena));

    if (_config.printGIL) {
        // Print all functions in the generated function list
        _gilPrinter->visit(*_gilModule);
        return 0;
    }

    glu::irgen::IRGen irgen;
    _llvmModule.emplace(
        _sourceManager.getBufferName(
            _sourceManager.getLocForStartOfFile(**_fileID)
        ),
        _llvmContext
    );
    irgen.generateIR(*_llvmModule, *_gilModule, &_sourceManager);

    if (_config.printLLVMIR) {
        _llvmModule->print(*_outputStream, nullptr);
        return 0;
    }

    return 0;
}

void CompilerDriver::generateCode(bool emitAssembly)
{
    // Set target triple
    if (!_config.targetTriple.empty()) {
        _llvmModule->setTargetTriple(_config.targetTriple);
    } else {
        // Use the host target triple
        _llvmModule->setTargetTriple(llvm::sys::getDefaultTargetTriple());
    }

    std::string targetError;
    auto target = llvm::TargetRegistry::lookupTarget(
        _llvmModule->getTargetTriple(), targetError
    );
    if (!target) {
        llvm::errs() << "Error looking up target: " << targetError << "\n";
        return;
    }

    llvm::TargetOptions targetOptions;
    llvm::Reloc::Model RM;
    // Set PIC relocation model for Linux executables
    if (llvm::StringRef(_llvmModule->getTargetTriple()).contains("linux")) {
        RM = llvm::Reloc::PIC_;
    }
    auto targetMachine = target->createTargetMachine(
        _llvmModule->getTargetTriple(), "generic", "", targetOptions, RM
    );

    _llvmModule->setDataLayout(targetMachine->createDataLayout());

    // Create the analysis managers.
    // These must be declared in this order so that they are destroyed in the
    // correct order due to inter-analysis-manager references.
    llvm::LoopAnalysisManager LAM;
    llvm::FunctionAnalysisManager FAM;
    llvm::CGSCCAnalysisManager CGAM;
    llvm::ModuleAnalysisManager MAM;

    // Create the new pass manager builder.
    // Take a look at the PassBuilder constructor parameters for more
    // customization, e.g. specifying a TargetMachine or various debugging
    // options.
    llvm::PassBuilder PB;

    // Register all the basic analyses with the managers.
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    // Create the pass manager.
    // This one corresponds to a typical -O2 optimization pipeline.
    llvm::ModulePassManager MPM;

    // Optimize the IR!
    if (_config.optLevel > 0) {
        switch (_config.optLevel) {
        case 1:
            MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O1);
            break;
        case 2:
            MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O2);
            break;
        case 3:
            MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);
            break;
        default: break;
        }
        MPM.run(*_llvmModule, MAM); // Optimize
    }

    // Use legacy PassManager for codegen
    llvm::legacy::PassManager codegenPM;
    llvm::CodeGenFileType fileType = emitAssembly
        ? llvm::CodeGenFileType::AssemblyFile
        : llvm::CodeGenFileType::ObjectFile;
    if (targetMachine->addPassesToEmitFile(
            codegenPM, *_outputFileStream, nullptr, fileType
        )) {
        llvm::errs() << "Error adding codegen passes\n";
        return;
    }
    codegenPM.run(*_llvmModule);
}

int CompilerDriver::compile()
{
    // Generate object code or assembly
    if (_config.emitAssembly || _config.emitObject || _needsLinking) {
        std::string outputPath;
        std::unique_ptr<llvm::raw_fd_ostream> fileOut;

        if (_config.emitAssembly || _config.emitObject) {
            // For -S or -c, use specified output or stdout
            if (!_config.outputFile.empty()) {
                outputPath = _config.outputFile;
                std::error_code EC;
                llvm::raw_fd_ostream fileOut(
                    outputPath, EC, llvm::sys::fs::OF_None
                );

                if (EC) {
                    llvm::errs() << "Error opening output file " << outputPath
                                 << ": " << EC.message() << "\n";
                    return 1;
                }
            }
            generateCode(_config.emitAssembly);
        } else if (_needsLinking) {
            // For linking, create temporary object file
            llvm::SmallString<128> tempPath;
            std::error_code EC
                = llvm::sys::fs::createTemporaryFile("gluc", "o", tempPath);
            if (EC) {
                llvm::errs()
                    << "Error creating temporary file: " << EC.message()
                    << "\n";
                return 1;
            }

            outputPath = tempPath.str().str();
            _outputFileStream = std::make_unique<llvm::raw_fd_ostream>(
                outputPath, EC, llvm::sys::fs::OF_None
            );
            if (EC) {
                llvm::errs() << "Error opening temporary file " << outputPath
                             << ": " << EC.message() << "\n";
                return 1;
            }

            generateCode(false);
            _outputFileStream.reset(); // Close the file
            _objectFile = outputPath;
        }
    } else {
        // If no output options are specified, print the LLVM IR
        llvm::outs(
        ) << "No output as no output file or options are specified\n";
    }
    return 0;
}

int CompilerDriver::callLinker(std::string const &objectFile)
{
    auto clangPath = llvm::sys::findProgramByName("clang");
    if (!clangPath) {
        llvm::errs() << "Error: Could not find clang linker: "
                     << clangPath.getError().message() << "\n";
        return 1;
    }

    std::vector<std::string> importedFiles;
    if (_importManager) {
        importedFiles = findImportedObjectFiles();
    }

    std::vector<llvm::StringRef> args;
    args.push_back("clang");

    args.push_back(objectFile);

    for (auto const &importedFile : importedFiles) {
        args.push_back(importedFile);
    }

    if (!_config.outputFile.empty()) {
        args.push_back("-o");
        args.push_back(_config.outputFile);
    }

    std::string errorMsg;
    int result = llvm::sys::ExecuteAndWait(
        *clangPath, args, std::nullopt, {}, 0, 0, &errorMsg
    );

    if (result != 0) {
        llvm::errs() << "Linker failed with exit code " << result;
        if (!errorMsg.empty()) {
            llvm::errs() << ": " << errorMsg;
        }
        llvm::errs() << "\n";
    }

    return result;
}

int CompilerDriver::processLinking()
{
    int linkerResult = callLinker(_objectFile);

    // Clean up temporary object file
    std::error_code removeEC = llvm::sys::fs::remove(_objectFile);
    if (removeEC) {
        llvm::errs() << "Warning: Failed to remove temporary file "
                     << _objectFile << ": " << removeEC.message() << "\n";
    }

    if (linkerResult != 0) {
        return linkerResult;
    }

    return 0;
}

int CompilerDriver::executeCompilation(char const *argv0)
{
    // Initialize LLVM targets and create managers
    initializeLLVMTargets();
    _diagManager.emplace(_sourceManager);
    _context.emplace(&_sourceManager);
    _gilPrinter.emplace(&_sourceManager, *_outputStream);
    generateSystemImportPaths(argv0);
    _importManager.emplace(*_context, *_diagManager, _config.importDirs);

    // Configure parser
    if (!configureParser()) {
        return 1;
    }

    // Handle print-tokens early exit
    if (_config.printTokens) {
        printTokens();
        return 0;
    }

    // Parse the source code
    _parser->parse();

    // Process pre-compilation options
    auto compileResult = processPreCompilationOptions();

    // Handle early exit cases for print options
    if (_config.printAST || _config.printASTGen || _config.printGIL
        || _config.printLLVMIR) {
        return compileResult;
    }

    if (compileResult != 0) {
        return compileResult;
    }

    // Verify generated IR
    if (llvm::verifyModule(*_llvmModule, &llvm::errs())) {
        llvm::errs() << "Error: Generated LLVM IR is invalid\n";
        return 1;
    }

    // Compile to object code
    compile();

    // Check for errors before proceeding to linking
    if (_diagManager->hasErrors()) {
        return 1;
    }

    // Call linker if needed
    if (_needsLinking && !_objectFile.empty()) {
        int linkResult = processLinking();
        if (linkResult != 0) {
            return linkResult;
        }
    }

    return 0;
}

int CompilerDriver::run(int argc, char **argv)
{
    // Parse command line arguments
    if (!parseCommandLine(argc, argv)) {
        return 1;
    }

    // Perform the main compilation
    int result = executeCompilation(argv[0]);

    // Always print diagnostics at the end
    if (_diagManager) {
        _diagManager->printAll(llvm::errs());
    }

    // Clean up temporary object file if there was an error and linking was
    // needed
    if (result != 0 && _needsLinking && !_objectFile.empty()) {
        std::error_code removeEC = llvm::sys::fs::remove(_objectFile);
        if (removeEC) {
            llvm::errs() << "Warning: Failed to remove temporary file "
                         << _objectFile << ": " << removeEC.message() << "\n";
        }
    }

    return result;
}
}
