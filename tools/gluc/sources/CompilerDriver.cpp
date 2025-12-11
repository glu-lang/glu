#include "CompilerDriver.hpp"

#include "GILGen/GILGen.hpp"
#include "IRDec/ModuleLifter.hpp"
#include "IRGen/IRGen.hpp"
#include "Optimizer/PassManager.hpp"
#include "Parser/Parser.hpp"
#include "Sema/Sema.hpp"

#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/WithColor.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/Transforms/Instrumentation/AddressSanitizer.h>

#include <cassert>
#include <memory>

using namespace llvm::cl;

int main(int argc, char **argv);

namespace glu::driver {

/// @brief Find object files for imported modules using ImportManager
std::vector<std::string> CompilerDriver::findImportedObjectFiles()
{
    std::vector<std::string> importedFiles;

    // First, process any skipped private imports to ensure they are handled
    // before we look for their object files.
    _importManager->processSkippedImports();

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
        } else {
            // Direct LLVM IR or bitcode import: use the same file path
            importedFiles.push_back(filePath.str());
        }
    }

    return importedFiles;
}

bool CompilerDriver::parseCommandLine(int argc, char **argv)
{
    _argv0 = argv[0];
    // Create all command line options locally

    opt<Stage> CompilerStage(
        desc("Compiler stage (to stop before linking)"), init(Linking),
        values(
            clEnumValN(
                PrintTokens, "print-tokens",
                "Print tokens after lexical analysis"
            ),
            clEnumValN(
                PrintASTGen, "print-astgen", "Print the AST after parsing"
            ),
            clEnumValN(
                PrintConstraints, "print-constraints",
                "Print constraint system during semantic analysis"
            ),
            clEnumValN(
                PrintAST, "print-ast", "Print the AST after semantic analysis"
            ),
            clEnumValN(
                PrintInterface, "print-interface",
                "Print the interface declarations from the AST"
            ),
            clEnumValN(PrintGILGen, "print-gilgen", "Print GIL before passes"),
            clEnumValN(PrintGIL, "print-gil", "Print GIL after passes"),
            clEnumValN(PrintLLVMIR, "print-llvm-ir", "Print resulting LLVM IR"),
            clEnumValN(EmitBitcode, "emit-llvm-bc", "Emit LLVM bitcode"),
            clEnumValN(EmitAssembly, "S", "Emit assembly code"),
            clEnumValN(EmitObject, "c", "Emit object file")
        )
    );

    opt<std::string> TargetTriple(
        "target", desc("Target triple"), value_desc("triple")
    );

    // Optimization level options
    enum OptLevel { O0, O1, O2, O3 };
    opt<OptLevel> OptimizationLevel(
        desc("Optimization level"), init(O0),
        values(
            clEnumValN(O2, "O", "Enable default optimization (-O2)"),
            clEnumVal(O0, "No optimization"),
            clEnumVal(O1, "Enable basic optimizations"),
            clEnumVal(O2, "Enable default optimizations"),
            clEnumVal(O3, "Enable aggressive optimizations")
        )
    );

    opt<std::string> OutputFilename(
        "o", desc("Redirect output to the specified file"),
        value_desc("filename")
    );

    list<std::string> ImportDirs(
        "I", desc("Add directory to import search path"), ZeroOrMore,
        value_desc("directory")
    );

    opt<std::string> LinkerOption(
        "linker", desc("Linker command to use (default: clang)"),
        value_desc("linker")
    );

    list<std::string> LinkerArgs(
        "Wl", desc("Pass comma-separated arguments to the linker"),
        value_desc("arg"), CommaSeparated, Prefix
    );

    opt<bool> AddressSanitizer(
        "sanitize-address", desc("Enable AddressSanitizer"), init(false)
    );

    opt<std::string> InputFilename(
        Positional, Required, desc("<input glu file>")
    );

    // Parse the command line
    ParseCommandLineOptions(argc, argv, "Glu Compiler\n");

    // Store parsed values in config_ member
    _config = { .inputFile = InputFilename,
                .outputFile = OutputFilename,
                .importDirs = {},
                .targetTriple = TargetTriple,
                .linker = LinkerOption,
                .linkerArgs = {},
                .optLevel = OptimizationLevel,
                .asan = AddressSanitizer,
                .stage = CompilerStage };

    _config.importDirs.assign(ImportDirs.begin(), ImportDirs.end());
    _config.linkerArgs.assign(LinkerArgs.begin(), LinkerArgs.end());

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

void CompilerDriver::applyOptimizations()
{
    // Create the analysis managers.
    // These must be declared in this order so that they are destroyed in the
    // correct order due to inter-analysis-manager references.
    llvm::LoopAnalysisManager LAM;
    llvm::FunctionAnalysisManager FAM;
    llvm::CGSCCAnalysisManager CGAM;
    llvm::ModuleAnalysisManager MAM;

    // Create the new pass manager builder.
    llvm::PassBuilder PB;

    // Register all the basic analyses with the managers.
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    // Create the pass manager and apply optimizations
    llvm::ModulePassManager MPM;
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
    default:
        MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O0);
        break;
    }
    if (_config.asan) {
        MPM.addPass(llvm::AddressSanitizerPass({}));
    }
    MPM.run(*_llvmModule, MAM);
}

void CompilerDriver::generateSystemImportPaths()
{
    // Add system import path based on compiler location
    // If the driver is /usr/bin/gluc, we add /usr/lib/glu/ to the
    // import paths
    llvm::SmallString<128> compiler(
        llvm::sys::fs::getMainExecutable(_argv0, (void *) main)
    );
    llvm::sys::path::remove_filename(compiler);
    llvm::sys::path::append(compiler, "..", "lib", "glu");
    llvm::sys::path::remove_dots(compiler, true);
    _config.importDirs.push_back(compiler.str().str());
}

void CompilerDriver::printTokens()
{
    glu::Scanner scanner(
        _sourceManager.getBuffer(_fileID), _context.getScannerAllocator()
    );
    for (glu::Token token = scanner.nextToken();
         token.isNot(glu::TokenKind::eofTok); token = scanner.nextToken()) {
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

bool CompilerDriver::loadSourceFile()
{
    auto fileID = _sourceManager.loadFile(_config.inputFile);

    if (!fileID) {
        llvm::errs() << "Error loading " << _config.inputFile << ": "
                     << fileID.getError().message() << "\n";
        return false;
    }
    _fileID = *fileID;
    return true;
}

int CompilerDriver::runParser()
{
    glu::Scanner scanner(
        _sourceManager.getBuffer(_fileID), _context.getScannerAllocator()
    );
    glu::Parser parser(scanner, _context, _sourceManager, _diagManager);

    if (!parser.parse() || _diagManager.hasErrors()) {
        return 1;
    }

    _ast = llvm::cast<glu::ast::ModuleDecl>(parser.getAST());

    if (_config.stage == PrintASTGen) {
        _ast->print(*_outputStream);
        return 0;
    }

    return 0;
}

int CompilerDriver::runSema()
{
    sema::constrainAST(
        _ast, _diagManager, &(*_importManager),
        _config.stage == PrintConstraints
    );

    if (_config.stage == PrintConstraints) {
        // Constraints are printed by the constrainAST function itself
        return 0;
    }

    if (_config.stage == PrintAST) {
        _ast->print(*_outputStream);
        return 0;
    }

    if (_config.stage == PrintInterface) {
        _ast->printInterface(*_outputStream);
        return 0;
    }

    if (_diagManager.hasErrors()) {
        return 1;
    }

    return 0;
}

int CompilerDriver::runGILGen()
{
    _gilModule = glu::gilgen::generateModule(_ast);

    if (_config.stage == PrintGILGen) {
        // Print all functions in the generated function list
        glu::gil::printModule(
            _gilModule.get(), *_outputStream, &_sourceManager
        );
    }

    return 0;
}

int CompilerDriver::runOptimizer()
{
    glu::optimizer::PassManager passManager(
        _diagManager, _sourceManager, *_outputStream, _gilModule.get()
    );
    passManager.runPasses();

    if (_config.stage == PrintGIL) {
        // Print all functions in the generated function list
        glu::gil::printModule(
            _gilModule.get(), *_outputStream, &_sourceManager
        );
        return _diagManager.hasErrors();
    }

    if (_diagManager.hasErrors()) {
        return 1;
    }
    return 0;
}

int CompilerDriver::runIRGen()
{
    glu::irgen::IRGen irgen;
    _llvmModule = std::make_unique<llvm::Module>(
        _sourceManager.getBufferName(
            _sourceManager.getLocForStartOfFile(_fileID)
        ),
        _llvmContext
    );
    _llvmModule->setTargetTriple(llvm::sys::getDefaultTargetTriple());
    irgen.generateIR(*_llvmModule, _gilModule.get(), &_sourceManager);

    // Apply optimizations if requested
    applyOptimizations();

    if (_config.stage == PrintLLVMIR) {
        _llvmModule->print(*_outputStream, nullptr);
        return 0;
    }

    if (_config.stage == EmitBitcode) {
        llvm::WriteBitcodeToFile(*_llvmModule, *_outputStream);
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
    std::unique_ptr<llvm::TargetMachine> targetMachine(
        target->createTargetMachine(
            _llvmModule->getTargetTriple(), "generic", "", targetOptions, RM
        )
    );
    if (!targetMachine) {
        llvm::errs() << "Failed to create target machine\n";
        return;
    }

    _llvmModule->setDataLayout(targetMachine->createDataLayout());

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

static std::string getFileExtensionForStage(Stage stage)
{
    switch (stage) {
    case EmitAssembly: return ".s";
    case EmitObject: return ".o";
    default: return "";
    }
}

static std::string getOutputFilePath(
    std::string const &inputFile, std::string const &outputFile, Stage stage
)
{
    if (!outputFile.empty()) {
        return outputFile;
    }

    llvm::SmallString<256> outputPath(inputFile);

    llvm::sys::path::replace_extension(
        outputPath, getFileExtensionForStage(stage)
    );

    return outputPath.str().str();
}

int CompilerDriver::compile()
{
    assert(
        _config.stage >= EmitAssembly
        && "compile() should only be called if code generation is needed"
    );
    std::string outputPath;

    if (_config.stage == EmitAssembly || _config.stage == EmitObject) {
        outputPath = getOutputFilePath(
            _config.inputFile, _config.outputFile, _config.stage
        );

        std::error_code EC;
        _outputFileStream = std::make_unique<llvm::raw_fd_ostream>(
            outputPath, EC, llvm::sys::fs::OF_None
        );

        if (EC) {
            llvm::errs() << "Error opening output file " << outputPath << ": "
                         << EC.message() << "\n";
            _outputFileStream.reset();
            return 1;
        }

        _outputStream = _outputFileStream.get();

        generateCode(_config.stage == EmitAssembly);
    } else if (_config.stage == Linking) {
        // For linking, create temporary object file
        llvm::SmallString<128> tempPath;
        std::error_code EC
            = llvm::sys::fs::createTemporaryFile("gluc", "o", tempPath);
        if (EC) {
            llvm::errs() << "Error creating temporary file: " << EC.message()
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
    return 0;
}

int CompilerDriver::callLinker()
{
    auto linkerName = "clang";
    // CLI flag, then environment variable, otherwise clang
    if (!_config.linker.empty()) {
        linkerName = _config.linker.c_str();
    } else if (char const *envLinker = std::getenv("GLU_LINKER")) {
        linkerName = envLinker;
    }
    auto clangPath = llvm::sys::findProgramByName(linkerName);
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
    args.push_back(linkerName);

    args.push_back(_objectFile);

    for (auto const &importedFile : importedFiles) {
        args.push_back(importedFile);
    }

    // Pass linker arguments from -Wl,<arg>
    for (auto const &linkerArg : _config.linkerArgs) {
        args.push_back(linkerArg);
    }

    if (_config.asan) {
        args.push_back("-fsanitize=address");
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

int CompilerDriver::performCompilation()
{
    // Initialize LLVM targets and create managers
    initializeLLVMTargets();
    generateSystemImportPaths();
    _importManager.emplace(_context, _diagManager, _config.importDirs);

    // Configure parser
    if (!loadSourceFile()) {
        return 1;
    }

    // Handle print-tokens early exit
    if (_config.stage == PrintTokens) {
        printTokens();
        return 0;
    }

    // Parse the source code
    if (runParser()) {
        return 1;
    }

    if (_config.stage <= PrintASTGen) {
        return 0;
    }

    // Process pre-compilation options
    if (runSema()) {
        return 1;
    }

    if (_config.stage <= PrintAST) {
        return 0;
    }

    if (runGILGen()) {
        return 1;
    }

    if (_config.stage <= PrintGILGen) {
        return 0;
    }

    if (runOptimizer()) {
        return 1;
    }

    if (_config.stage <= PrintGIL) {
        return 0;
    }

    if (runIRGen()) {
        return 1;
    }

    if (_config.stage <= EmitBitcode) {
        return 0;
    }

    // Verify generated IR
    if (llvm::verifyModule(*_llvmModule, &llvm::errs())) {
        llvm::errs() << "Error: Generated LLVM IR is invalid\n";
        return 1;
    }

    // Compile to object code
    compile();

    // Check for errors before proceeding to linking
    if (_diagManager.hasErrors()) {
        return 1;
    }

    // Call linker if needed
    if (_config.stage == Linking && !_objectFile.empty()) {
        return callLinker();
    }

    return 0;
}

int CompilerDriver::runIRParser()
{
    // Initialize LLVM targets
    initializeLLVMTargets();

    // Parse the LLVM module from file (handles both .ll and .bc)
    llvm::SMDiagnostic err;
    _llvmModule = llvm::parseIRFile(_config.inputFile, err, _llvmContext);

    if (!_llvmModule) {
        llvm::errs() << "Error parsing LLVM module from '" << _config.inputFile
                     << "':\n";
        err.print(_argv0, llvm::errs());
        return 1;
    }
    return 0;
}

void CompilerDriver::runLifter()
{
    _ast = glu::irdec::liftModule(_context, _llvmModule.get());

    // Print the lifted AST
    if (_config.stage == PrintAST) {
        _ast->print(*_outputStream);
        return;
    }

    if (_config.stage == PrintInterface) {
        _ast->printInterface(*_outputStream);
        return;
    }
}

int CompilerDriver::performDecompilation()
{
    // Run the IR parser
    if (runIRParser()) {
        return 1;
    }

    runLifter();

    if (_config.stage == PrintAST || _config.stage == PrintInterface) {
        return 0;
    }

    llvm::errs() << "Error: Invalid action specified for decompilation: "
                    "expected -print-ast or -print-interface\n";
    return 1;
}

int CompilerDriver::run(int argc, char **argv)
{
    // Parse command line arguments
    if (!parseCommandLine(argc, argv)) {
        return 1;
    }

    int result;

    // Detect the input file type based on extension
    // Note, we could have a -x flag later to override this
    if (_config.inputFile.ends_with(".glu")) {
        // For glu files, run the compilation pipeline
        result = performCompilation();
    } else if (_config.inputFile.ends_with(".ll")
               || _config.inputFile.ends_with(".bc")) {
        // For LLVM IR (.ll) or bitcode (.bc) files, run decompilation
        result = performDecompilation();
    } else {
        // Unsupported input file type
        llvm::errs() << "Error: Unsupported input file type: "
                     << _config.inputFile << "\n";
        return 1;
    }

    // Always print diagnostics at the end
    _diagManager.printAll(llvm::errs());

    // Clean up temporary object file if there was an error and linking was
    // needed
    if (result != 0 && _config.stage == Linking && !_objectFile.empty()) {
        std::error_code removeEC = llvm::sys::fs::remove(_objectFile);
        if (removeEC) {
            llvm::errs() << "Warning: Failed to remove temporary file "
                         << _objectFile << ": " << removeEC.message() << "\n";
        }
    }

    return result;
}
}
