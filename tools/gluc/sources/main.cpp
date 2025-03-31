#include "Basic/SourceLocation.hpp"
#include "Basic/SourceManager.hpp"
#include "GIL/GILPrinter.hpp"
#include "GILGen/GILGen.hpp"
#include "Lexer/Scanner.hpp"
#include "Parser/Parser.hpp"

#include "llvm/Support/CommandLine.h"

using namespace llvm;

static cl::opt<bool> PrintAST(
    "print-ast", cl::desc("Print the AST after parsing"), cl::init(false)
);

static cl::opt<bool> PrintTokens(
    "print-tokens", cl::desc("Print tokens after lexical analysis"),
    cl::init(false)
);

static cl::opt<bool> PrintGIL(
    "print-gil", cl::desc("Print GIL after generation"), cl::init(false)
);

static cl::list<std::string> InputFilenames(
    cl::Positional, cl::OneOrMore, cl::desc("<input glu files>")
);

void printTokens(glu::SourceManager &sourceManager, glu::Scanner &scanner)
{
    for (glu::Token token = scanner.nextToken();
         token.isNot(glu::TokenKind::eofTok); token = scanner.nextToken()) {
        glu::SourceLocation loc
            = sourceManager.getSourceLocFromStringRef(token.getLexeme());

        auto spellingLine = sourceManager.getSpellingLineNumber(loc);
        auto spellingColumn = sourceManager.getSpellingColumnNumber(loc);

        llvm::outs() << sourceManager.getBufferName(loc) << ":" << spellingLine
                     << ":" << spellingColumn << ": <" << token.getKind()
                     << ", \"" << token.getLexeme() << "\">\n";
    }
}

int main(int argc, char **argv)
{
    cl::ParseCommandLineOptions(argc, argv);

    glu::SourceManager sourceManager;
    glu::ast::ASTContext context;
    llvm::BumpPtrAllocator GILFuncArena;
    glu::gil::GILPrinter GILPrinter(sourceManager);

    for (auto const &inputFile : InputFilenames) {
        auto fileID = sourceManager.loadFile(inputFile.c_str());

        if (!fileID) {
            errs() << "Error loading " << inputFile << ": "
                   << fileID.getError().message() << "\n";
            continue;
        }

        glu::Scanner scanner(sourceManager.getBuffer(*fileID));

        if (PrintTokens) {
            printTokens(sourceManager, scanner);
            continue;
        }

        glu::Parser parser(scanner, context, sourceManager);

        if (parser.parse()) {
            auto ast = llvm::cast<glu::ast::ModuleDecl>(parser.getAST());

            if (!ast) {
                continue;
            }

            if (PrintAST) {
                ast->debugPrint();
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
