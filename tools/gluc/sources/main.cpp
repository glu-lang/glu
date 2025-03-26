#include "Basic/SourceLocation.hpp"
#include "Basic/SourceManager.hpp"
#include "IRGen/IRGen.hpp"
#include "Lexer/Scanner.hpp"
#include "Parser/Parser.hpp"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        llvm::errs() << "Usage: " << argv[0] << " <input file>\n";
        return 1;
    }

    glu::SourceManager sourceManager;

    auto fileID = sourceManager.loadFile(argv[1]);

    if (!fileID) {
        llvm::errs() << fileID.getError().message() << "\n";
        return 1;
    }

    glu::Scanner scanner(sourceManager.getBuffer(*fileID));
    glu::ast::ASTContext context;
    glu::Parser parser(scanner, context, sourceManager);

    if (parser.parse()) {
        parser.getAST()->debugPrint();
    }
}
