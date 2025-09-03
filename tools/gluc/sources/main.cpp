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
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"

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

            if (PrintASTGen) {
                ast->debugPrint(out);
                continue;
            }

            sema::constrainAST(ast, diagManager);

            if (PrintAST) {
                ast->debugPrint(out);
                continue;
            }

            if (diagManager.hasErrors()) {
                continue;
            }

            // Generate GIL module from the AST module
            glu::gilgen::GILGen gilgen;
            glu::gil::Module *mod = gilgen.generateModule(ast, GILFuncArena);

            if (PrintGIL) {
                // Print all functions in the generated function list
                GILPrinter.visit(mod);
            }

            // If we're only printing intermediate representations, we're done
            if (PrintTokens || PrintAST || PrintGIL) {
                continue;
            }

            // Generate LLVM IR from GIL functions
            glu::irgen::IRGen irgen;
            llvm::Module llvmModule(
                sourceManager.getBufferName(
                    sourceManager.getLocForStartOfFile(*fileID)
                ),
                llvmContext
            );
            irgen.generateIR(llvmModule, mod);

            if (PrintLLVMIR) {
                llvmModule.print(out, nullptr);
                continue;
            }

            // Verify the generated IR
            if (llvm::verifyModule(llvmModule, &llvm::errs())) {
                llvm::errs() << "Error: Generated LLVM IR is invalid\n";
                return 1;
            }

            // Generate object code or assembly
            if (EmitAssembly || EmitObject || !OutputFilename.empty()) {
                if (!OutputFilename.empty()) {
                    std::error_code EC;
                    llvm::raw_fd_ostream fileOut(
                        OutputFilename, EC, llvm::sys::fs::OF_None
                    );
                    if (EC) {
                        llvm::errs()
                            << "Error opening output file " << OutputFilename
                            << ": " << EC.message() << "\n";
                        return 1;
                    }
                    generateCode(
                        llvmModule, TargetTriple, OptLevel, EmitAssembly,
                        fileOut
                    );
                } else {
                    generateCode(
                        llvmModule, TargetTriple, OptLevel, EmitAssembly,
                        llvm::outs()
                    );
                }
            } else {
                // If no output options are specified, print the LLVM IR
                llvm::outs()
                    << "No output as no output file or options are specified\n";
            }
        }
    }

    // Print diagnostics if there are any errors
    diagManager.printAll(llvm::errs());
    if (diagManager.hasErrors()) {
        return 1;
    }

    return 0;
}
