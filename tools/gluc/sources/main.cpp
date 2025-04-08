#include "Basic/SourceLocation.hpp"
#include "Basic/SourceManager.hpp"
#include "GIL/GILPrinter.hpp"
#include "GILGen/GILGen.hpp"
#include "Lexer/Scanner.hpp"
#include "Parser/Parser.hpp"

#include "llvm/Support/CommandLine.h"

using namespace llvm::cl;

static opt<bool>
    PrintAST("print-ast", desc("Print the AST after parsing"), init(false));

static opt<bool> PrintTokens(
    "print-tokens", desc("Print tokens after lexical analysis"), init(false)
);

static opt<bool>
    PrintGIL("print-gil", desc("Print GIL after generation"), init(false));

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

int main(int argc, char **argv)
{
    ParseCommandLineOptions(argc, argv);

    llvm::raw_ostream &out = getOutputStream();

    glu::SourceManager sourceManager;
    glu::ast::ASTContext context;
    llvm::BumpPtrAllocator GILFuncArena;
    glu::gil::GILPrinter GILPrinter(sourceManager, out);

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

        glu::Parser parser(scanner, context, sourceManager);

        if (parser.parse()) {
            auto ast = llvm::cast<glu::ast::ModuleDecl>(parser.getAST());

            if (!ast) {
                continue;
            }

            if (PrintAST) {
                ast->debugPrint(out);
                continue;
            }

            for (auto decl : ast->getDecls()) {
                if (auto fn = llvm::dyn_cast<glu::ast::FunctionDecl>(decl)) {
                    glu::gil::Function *GILFn
                        = glu::gilgen::GILGen().generateFunction(
                            fn, GILFuncArena
                        );

                    if (PrintGIL) {
                        GILPrinter.visit(GILFn);
                    }
                }
            }
        } else {
            std::cerr << "Error parsing" << std::endl;
        }
    }

    return 0;
}
