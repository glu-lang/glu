#include "Basic/SourceLocation.hpp"
#include "Basic/SourceManager.hpp"
#include "IRGen/IRGen.hpp"
#include "Lexer/Scanner.hpp"

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
