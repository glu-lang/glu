#include "Basic/SourceLocation.hpp"
#include "Basic/SourceManager.hpp"
#include "GIL/GILPrinter.hpp"
#include "GIL/Module.hpp"
#include "GILGen/GILGen.hpp"
#include "IRGen/IRGen.hpp"
#include "Lexer/Scanner.hpp"
#include "Parser/Parser.hpp"
#include "Sema/CSWalker.hpp"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"

using namespace llvm::cl;

static opt<bool>
    PrintAST("print-ast", desc("Print the AST after parsing"), init(false));

static opt<bool> PrintTokens(
    "print-tokens", desc("Print tokens after lexical analysis"), init(false)
);

static opt<bool>
    PrintGIL("print-gil", desc("Print GIL after generation"), init(false));

static opt<bool>
    PrintLLVMIR("print-llvm-ir", desc("Print LLVM IR after generation"), init(false));

static opt<std::string> TargetTriple(
    "target", desc("Target triple"), value_desc("triple")
);

static opt<unsigned>
    OptLevel("O", desc("Optimization level (0-3)"), init(0), value_desc("level")
);

static opt<bool>
    EmitAssembly("S", desc("Emit assembly code"), init(false));

static opt<bool>
    EmitObject("c", desc("Emit object file"), init(false));

static opt<std::string> OutputFilename(
    "o", desc("Redirect output to the specified file"), value_desc("filename")
);

static list<std::string>
    InputFilenames(Positional, OneOrMore, desc("<input glu files>"));

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
        // Use a default target triple for x86_64
        module.setTargetTriple("x86_64-pc-linux-gnu");
    }

    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(module.getTargetTriple(), error);
    if (!target) {
        llvm::errs() << "Error looking up target: " << error << "\n";
        return;
    }

    llvm::TargetOptions targetOptions;
    auto RM = llvm::Reloc::Model();
    auto targetMachine = target->createTargetMachine(
        module.getTargetTriple(), "generic", "", targetOptions, RM
    );

    module.setDataLayout(targetMachine->createDataLayout());

    // For now, skip complex optimization passes
    // TODO: Add proper optimization passes when LLVM API is stable

    // Add codegen pass - for simplicity, output to outs() for now
    llvm::legacy::PassManager codegenPM;
    llvm::CodeGenFileType fileType = emitAssembly ? 
        llvm::CodeGenFileType::AssemblyFile : 
        llvm::CodeGenFileType::ObjectFile;

    // For now, write to standard output to avoid the pwrite_stream issue
    if (targetMachine->addPassesToEmitFile(codegenPM, out, nullptr, fileType)) {
        llvm::errs() << "Error adding codegen passes\n";
        return;
    }

    codegenPM.run(module);
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
    auto llvmModule = std::make_unique<llvm::Module>("glu_module", llvmContext);

    // Create GIL module to collect functions
    glu::gil::Module gilModule("main");
    // Keep a vector of functions for manual management since they're allocated with BumpPtrAllocator
    std::vector<glu::gil::Function *> gilFunctions;

    for (auto const &inputFile : InputFilenames) {
        auto fileID = sourceManager.loadFile(inputFile.c_str());

        if (!fileID) {
            llvm::errs() << "Error loading " << inputFile << ": "
                         << fileID.getError().message() << "\n";
            continue;
        }

        glu::Scanner scanner(sourceManager.getBuffer(*fileID));

        if (PrintTokens) {
            printTokens(sourceManager, scanner, out);
            continue;
        }

        glu::Parser parser(scanner, context, sourceManager, diagManager);

        if (parser.parse()) {
            auto ast = llvm::cast<glu::ast::ModuleDecl>(parser.getAST());

            if (!ast) {
                continue;
            }

            if (PrintAST) {
                ast->debugPrint(out);
                continue;
            }

            sema::constrainAST(ast, diagManager);

            // Generate GIL for all functions
            for (auto decl : ast->getDecls()) {
                if (auto fn = llvm::dyn_cast<glu::ast::FunctionDecl>(decl)) {
                    glu::gil::Function *GILFn
                        = glu::gilgen::GILGen().generateFunction(
                            fn, GILFuncArena
                        );

                    if (PrintGIL) {
                        GILPrinter.visit(GILFn);
                    }

                    // Add function to GIL module manually via the functions list
                    // Note: We don't use push_back because of memory allocation issues
                    gilFunctions.push_back(GILFn);
                }
            }
        }
    }

    // Print diagnostics if there are any errors
    diagManager.printAll(llvm::errs());
    if (diagManager.hasErrors()) {
        return 1;
    }

    // If we're only printing intermediate representations, we're done
    if (PrintTokens || PrintAST || PrintGIL) {
        return 0;
    }

    // Generate LLVM IR from GIL functions
    if (!gilFunctions.empty()) {
        glu::irgen::IRGen irgen;
        
        // Visit each function individually to avoid Module memory management issues
        for (auto *fn : gilFunctions) {
            irgen.generateIR(*llvmModule, fn);
        }

        // Verify the generated IR
        if (llvm::verifyModule(*llvmModule, &llvm::errs())) {
            llvm::errs() << "Error: Generated LLVM IR is invalid\n";
            return 1;
        }

        if (PrintLLVMIR) {
            llvmModule->print(out, nullptr);
            return 0;
        }

        // Generate object code or assembly
        if (EmitAssembly || EmitObject || !OutputFilename.empty()) {
            if (!OutputFilename.empty()) {
                std::error_code EC;
                auto fileOut = std::make_unique<llvm::raw_fd_ostream>(
                    OutputFilename, EC, llvm::sys::fs::OF_None
                );
                if (EC) {
                    llvm::errs() << "Error opening output file " << OutputFilename 
                                 << ": " << EC.message() << "\n";
                    return 1;
                }
                generateCode(*llvmModule, TargetTriple, OptLevel, EmitAssembly, *fileOut);
            } else {
                generateCode(*llvmModule, TargetTriple, OptLevel, EmitAssembly, llvm::outs());
            }
        }
    }

    return 0;
}
