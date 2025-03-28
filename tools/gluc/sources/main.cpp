#include "Basic/SourceLocation.hpp"
#include "Basic/SourceManager.hpp"
#include "Lexer/Scanner.hpp"
#include "Parser/Parser.hpp"

#include "llvm/Support/CommandLine.h"

using namespace llvm;

static cl::opt<bool> PrintAST(
    "print-ast", cl::desc("Print the AST after parsing"), cl::init(false)
);

static cl::list<std::string> InputFilenames(
    cl::Positional, cl::OneOrMore, cl::desc("<input glu files>")
);

int main(int argc, char **argv)
{
    cl::ParseCommandLineOptions(argc, argv);

    if (InputFilenames.empty()) {
        errs() << "Usage: " << argv[0] << " <input files...>\n";
        return 1;
    }

    glu::SourceManager sourceManager;
    glu::ast::ASTContext context;

    for (auto const &inputFile : InputFilenames) {
        auto fileID = sourceManager.loadFile(inputFile.c_str());

        if (!fileID) {
            errs() << "Error loading " << inputFile << ": "
                   << fileID.getError().message() << "\n";
            continue;
        }

        glu::Scanner scanner(sourceManager.getBuffer(*fileID));
        glu::Parser parser(scanner, context, sourceManager);

        if (parser.parse()) {
            if (PrintAST) {
                parser.getAST()->debugPrint();
            }
        }
    }

    return 0;
}
