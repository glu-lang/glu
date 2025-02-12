#include "Scanner.hpp"

#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        llvm::errs() << "Usage: " << argv[0] << " <filename>" << '\n';
        return 1;
    }

    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> bufferOrErr
        = llvm::MemoryBuffer::getFile(argv[1]);

    if (!bufferOrErr) {
        llvm::errs() << "Error reading file: "
                     << bufferOrErr.getError().message() << '\n';
        return 1;
    }

    glu::Scanner scanner((*bufferOrErr).get());

    for (glu::Token token = scanner.nextToken();
         token.isNot(glu::TokenKind::eofTok); token = scanner.nextToken()) {
        llvm::outs() << token.getKind() << " " << token.getLexeme() << '\n';
    }

    return 0;
}
