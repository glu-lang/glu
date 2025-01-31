#ifndef GLU_LEXER_SCANNER_HPP
#define GLU_LEXER_SCANNER_HPP

#if !defined(yyFlexLexerOnce)
    #include <FlexLexer.h>
#endif

// Change return type and name of the yylex function
#undef YY_DECL
#define YY_DECL glu::TokenKind glu::Scanner::getNextToken()

#include "Basic/Tokens.hpp"

#include <llvm/Support/MemoryBuffer.h>
#include <sstream>

namespace glu {

class Scanner : public yyFlexLexer {
private:
    /// @brief The buffer we are reading from
    llvm::MemoryBuffer *_buf = nullptr;
    /// @brief The underlying string stream
    std::stringstream _ss;
    /// @brief If we have encountered a fatal error, force the scanner to return
    /// eofTok
    bool fatal_end = false;
    /// @brief The current offset in the buffer
    size_t _bufOffset = 0;
    /// @brief The offset of the start of the token, or -1 to infer from yyleng
    size_t _bufStartOffset = -1;

public:
    Scanner(llvm::MemoryBuffer *buf)
        : yyFlexLexer(nullptr, nullptr), _buf(buf), _ss(buf->getBuffer().str())
    {
        switch_streams(&_ss, nullptr);
    }
    virtual ~Scanner() = default;

    glu::Token nextToken()
    {
        TokenKind kind = getNextToken();
        auto offset = _bufStartOffset;
        if (offset == (size_t) -1) {
            if (fatal_end) {
                offset = _bufOffset;
            } else {
                offset = _bufOffset - yyleng;
            }
        }
        auto len = _bufOffset - offset;
        _bufStartOffset = -1; // Reset for next token
        return glu::Token(
            kind, llvm::StringRef(_buf->getBufferStart() + offset, len)
        );
    }

private:
    // yylex implementation
    glu::TokenKind getNextToken();
};

}
#endif // GLU_LEXER_SCANNER_HPP
