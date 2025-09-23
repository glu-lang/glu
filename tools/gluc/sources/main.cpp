#include "Basic/SourceLocation.hpp"
#include "Basic/SourceManager.hpp"
#include "GIL/GILPrinter.hpp"
#include "GILGen/GILGen.hpp"
#include "IRGen/IRGen.hpp"
#include "Lexer/Scanner.hpp"
#include "Parser/Parser.hpp"
#include "Sema/ImportManager.hpp"
#include "Sema/Sema.hpp"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"

using namespace llvm::cl;

static opt<bool> PrintTokens(
    "print-tokens", desc("Print tokens after lexical analysis"), init(false)
);

static opt<bool>
    PrintAST("print-ast", desc("Print the AST after sema"), init(false));

static opt<bool> PrintASTGen(
    "print-astgen", desc("Print the AST after parsing, before sema"),
    init(false)
);

static opt<bool>
    PrintGIL("print-gil", desc("Print GIL after generation"), init(false));

static opt<bool> PrintLLVMIR(
    "print-llvm-ir", desc("Print LLVM IR after generation"), init(false)
);

static opt<std::string>
    TargetTriple("target", desc("Target triple"), value_desc("triple"));

static opt<unsigned> OptLevel(
    "O", desc("Optimization level (0-3)"), init(0), value_desc("level")
);

static opt<bool> EmitAssembly("S", desc("Emit assembly code"), init(false));

static opt<bool> EmitObject("c", desc("Emit object file"), init(false));

static opt<std::string> OutputFilename(
    "o", desc("Redirect output to the specified file"), value_desc("filename")
);

static list<std::string> ImportDirs(
    "I", desc("Add directory to import search path"), ZeroOrMore,
    value_desc("directory")
);

static opt<std::string>
    inputFilename(Positional, Required, desc("<input glu file>"));

llvm::raw_ostream &getOutputStream()
{
    if (!OutputFilename.empty()) {
        // A static variable to hold the file stream.
        static std::unique_ptr<llvm::raw_fd_ostream> fileStream;
        if (!fileStream) {
            std::error_code EC;
            fileStream = std::make_unique<llvm::raw_fd_ostream>(
                OutputFilename, EC, llvm::sys::fs::OF_Text
            );
            if (EC) {
                llvm::errs() << "Error opening file " << OutputFilename << ": "
                             << EC.message() << "\n";
                exit(1);
            }
        }
        return *fileStream;
    }
    return llvm::outs();
}

void printTokens(
    glu::SourceManager &sourceManager, glu::Scanner &scanner,
    llvm::raw_ostream &out
)
{
    for (glu::Token token = scanner.nextToken();
         token.isNot(glu::TokenKind::eofTok); token = scanner.nextToken()) {
        glu::SourceLocation loc
            = sourceManager.getSourceLocFromStringRef(token.getLexeme());

        auto spellingLine = sourceManager.getSpellingLineNumber(loc);
        auto spellingColumn = sourceManager.getSpellingColumnNumber(loc);
        auto filepath = sourceManager.getBufferName(loc);

        out << filepath << ":" << spellingLine << ":" << spellingColumn << ": <"
            << token.getKind() << ", \"" << token.getLexeme() << "\">\n";
    }
}

/// @brief Initialize LLVM target infrastructure
void initializeLLVMTargets()
{
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
}

/// @brief Generate code using LLVM backend
void generateCode(
    llvm::Module &module, llvm::StringRef targetTriple, unsigned optLevel,
    bool emitAssembly, llvm::raw_pwrite_stream &out
)
{
    // Set target triple
    if (!targetTriple.empty()) {
        module.setTargetTriple(targetTriple);
    } else {
        // Use the host target triple
        module.setTargetTriple(llvm::sys::getDefaultTargetTriple());
    }

    std::string targetError;
    auto target = llvm::TargetRegistry::lookupTarget(
        module.getTargetTriple(), targetError
    );
    if (!target) {
        llvm::errs() << "Error looking up target: " << targetError << "\n";
        return;
    }

    llvm::TargetOptions targetOptions;
    llvm::Reloc::Model RM;
    // Set PIC relocation model for Linux executables
    if (llvm::StringRef(module.getTargetTriple()).contains("linux")) {
        RM = llvm::Reloc::PIC_;
    }
    auto targetMachine = target->createTargetMachine(
        module.getTargetTriple(), "generic", "", targetOptions, RM
    );

    module.setDataLayout(targetMachine->createDataLayout());

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
    if (optLevel > 0) {
        switch (optLevel) {
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
        MPM.run(module, MAM); // Optimize
    }

    // Use legacy PassManager for codegen
    llvm::legacy::PassManager codegenPM;
    llvm::CodeGenFileType fileType = emitAssembly
        ? llvm::CodeGenFileType::AssemblyFile
        : llvm::CodeGenFileType::ObjectFile;
    if (targetMachine->addPassesToEmitFile(codegenPM, out, nullptr, fileType)) {
        llvm::errs() << "Error adding codegen passes\n";
        return;
    }
    codegenPM.run(module);
}

int main(int argc, char **argv);

void generateSystemImportPaths(char const *argv0)
{
    // Add system import path based on compiler location
    // If the driver is /usr/bin/gluc, we add /usr/lib/glu/ to the import paths
    llvm::SmallString<128> compiler(
        llvm::sys::fs::getMainExecutable(argv0, (void *) main)
    );
    llvm::sys::path::remove_filename(compiler);
    llvm::sys::path::append(compiler, "..", "lib", "glu");
    llvm::sys::path::remove_dots(compiler, true);
    ImportDirs.push_back(compiler.str().str());
}

/// @brief Find object files for imported modules using ImportManager
std::vector<std::string>
findImportedObjectFiles(glu::sema::ImportManager const &importManager)
{
    std::vector<std::string> importedFiles;

    auto const &importedFilesMap = importManager.getImportedFiles();
    auto *sourceManager = importManager.getASTContext().getSourceManager();

    for (auto const &entry : importedFilesMap) {
        glu::FileID fileID = entry.first;
        llvm::StringRef filePath = sourceManager->getBufferName(fileID);
        if (filePath.ends_with(".glu")) {
            std::string objPath = filePath.str();
            objPath.replace(objPath.length() - 4, 4, ".o");

            if (llvm::sys::fs::exists(objPath)) {
                importedFiles.push_back(objPath);
            } else {
                llvm::errs()
                    << "Warning: Object file not found for imported module: "
                    << objPath << " (from " << filePath << ")\n";
            }
        }
    }

    return importedFiles;
}

/// @brief Call the linker (clang) to create an executable
int callLinker(
    std::vector<std::string> const &objectFiles,
    glu::sema::ImportManager const *importManager = nullptr
)
{
    auto clangPath = llvm::sys::findProgramByName("clang");
    if (!clangPath) {
        llvm::errs() << "Error: Could not find clang linker: "
                     << clangPath.getError().message() << "\n";
        return 1;
    }

    std::vector<std::string> importedFiles;
    if (importManager) {
        importedFiles = findImportedObjectFiles(*importManager);
    }

    std::vector<llvm::StringRef> args;
    args.push_back("clang");

    for (auto const &objFile : objectFiles) {
        args.push_back(objFile);
    }

    for (auto const &importedFile : importedFiles) {
        args.push_back(importedFile);
    }

    if (!OutputFilename.empty()) {
        args.push_back("-o");
        args.push_back(OutputFilename.getValue());
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

int main(int argc, char **argv)
{
    ParseCommandLineOptions(argc, argv);

    // Initialize LLVM targets
    initializeLLVMTargets();

    llvm::raw_ostream &out = getOutputStream();

    glu::SourceManager sourceManager;
    glu::DiagnosticManager diagManager(sourceManager);
    glu::ast::ASTContext context = glu::ast::ASTContext(&sourceManager);
    llvm::BumpPtrAllocator GILFuncArena;
    glu::gil::GILPrinter GILPrinter(&sourceManager, out);

    // Create LLVM module for IR generation
    llvm::LLVMContext llvmContext;

    // Track object files for linking
    std::vector<std::string> objectFiles;
    bool needsLinking = !EmitObject && !EmitAssembly;

    // Create ImportManager for tracking imported modules
    generateSystemImportPaths(argv[0]);
    glu::sema::ImportManager importManager(context, diagManager, ImportDirs);

    auto fileID = sourceManager.loadFile(inputFilename.c_str());

    if (!fileID) {
        llvm::errs() << "Error loading " << inputFilename << ": "
                     << fileID.getError().message() << "\n";
        return 1;
    }

    glu::Scanner scanner(sourceManager.getBuffer(*fileID));

    if (PrintTokens) {
        printTokens(sourceManager, scanner, out);
        return 0;
    }

    glu::Parser parser(scanner, context, sourceManager, diagManager);

    if (parser.parse()) {
        auto ast = llvm::cast<glu::ast::ModuleDecl>(parser.getAST());

        if (!ast) {
            return 1;
        }

        if (PrintASTGen) {
            ast->print(out);
            return 0;
        }

        sema::constrainAST(ast, diagManager, &importManager);

        if (PrintAST) {
            ast->print(out);
            return 0;
        }

        if (diagManager.hasErrors()) {
            diagManager.printAll(llvm::errs());
            return 1;
        }

        // Generate GIL module from the AST module
        glu::gilgen::GILGen gilgen;
        glu::gil::Module *mod = gilgen.generateModule(ast, GILFuncArena);

        if (PrintGIL) {
            // Print all functions in the generated function list
            GILPrinter.visit(mod);
            return 0;
        }

        // Generate LLVM IR from GIL functions
        glu::irgen::IRGen irgen;
        llvm::Module llvmModule(
            sourceManager.getBufferName(
                sourceManager.getLocForStartOfFile(*fileID)
            ),
            llvmContext
        );
        irgen.generateIR(llvmModule, mod, &sourceManager);

        if (PrintLLVMIR) {
            llvmModule.print(out, nullptr);
            return 0;
        }

        // Verify the generated IR
        if (llvm::verifyModule(llvmModule, &llvm::errs())) {
            llvm::errs() << "Error: Generated LLVM IR is invalid\n";
            return 1;
        }

        // Generate object code or assembly
        if (EmitAssembly || EmitObject || needsLinking) {
            std::string outputPath;
            std::unique_ptr<llvm::raw_fd_ostream> fileOut;

            if (EmitAssembly || EmitObject) {
                // For -S or -c, use specified output or stdout
                if (!OutputFilename.empty()) {
                    outputPath = OutputFilename;
                    std::error_code EC;
                    fileOut = std::make_unique<llvm::raw_fd_ostream>(
                        outputPath, EC, llvm::sys::fs::OF_None
                    );
                    if (EC) {
                        llvm::errs()
                            << "Error opening output file " << outputPath
                            << ": " << EC.message() << "\n";
                        return 1;
                    }
                    generateCode(
                        llvmModule, TargetTriple, OptLevel, EmitAssembly,
                        *fileOut
                    );
                } else {
                    generateCode(
                        llvmModule, TargetTriple, OptLevel, EmitAssembly,
                        llvm::outs()
                    );
                }
            } else if (needsLinking) {
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
                fileOut = std::make_unique<llvm::raw_fd_ostream>(
                    outputPath, EC, llvm::sys::fs::OF_None
                );
                if (EC) {
                    llvm::errs() << "Error opening temporary file "
                                 << outputPath << ": " << EC.message() << "\n";
                    return 1;
                }

                generateCode(
                    llvmModule, TargetTriple, OptLevel,
                    false, // false = object file
                    *fileOut
                );
                fileOut.reset(); // Close the file
                objectFiles.push_back(outputPath);
            }
        } else {
            // If no output options are specified, print the LLVM IR
            llvm::outs(
            ) << "No output as no output file or options are specified\n";
        }
    }

    // Print diagnostics if there are any errors
    diagManager.printAll(llvm::errs());
    if (diagManager.hasErrors()) {
        // Clean up temporary object files if linking was needed
        if (needsLinking) {
            for (auto const &objFile : objectFiles) {
                llvm::sys::fs::remove(objFile);
            }
        }
        return 1;
    }

    // Call linker if needed
    if (needsLinking && !objectFiles.empty()) {
        int linkerResult = callLinker(objectFiles, &importManager);

        // Clean up temporary object files
        for (auto const &objFile : objectFiles) {
            llvm::sys::fs::remove(objFile);
        }

        if (linkerResult != 0) {
            return linkerResult;
        }
    }

    return 0;
}
