#ifndef GLU_TOKENS_HPP
#define GLU_TOKENS_HPP

#include "llvm/ADT/StringRef.h"
namespace glu {

enum class TokenKind {
#define TOKEN(Name) Name##Tok,
#include "TokenKind.def"
    TOKEN_NUM
};

class Token {
    /// The kind of the token
    TokenKind _kind;

    /// Whether the token is the first token on a line.
    unsigned _atStartOfLine : 1;

    /// The actual lexeme of the token
    llvm::StringRef _lexeme;

public:
    Token(TokenKind kind, llvm::StringRef lexeme, unsigned commentLength = 0)
        : _kind(kind), _atStartOfLine(false), _lexeme(lexeme)
    {
    }

    Token() : Token(TokenKind::TOKEN_NUM, {}, 0) { }

    TokenKind getKind() const { return _kind; }
    void setKind(TokenKind kind) { _kind = kind; }

    bool is(TokenKind kind) const { return _kind == kind; }
    bool isNot(TokenKind kind) const { return _kind != kind; }

    bool isAtStartOfLine() const { return _atStartOfLine; }
    void setAtStartOfLine(bool atStartOfLine)
    {
        _atStartOfLine = atStartOfLine;
    }

    /// True if the token is any keyword.
    bool isKeyword() const
    {
        switch (_kind) {
#define KEYWORD(X) glu::TokenKind::X##Kw : return true;
#include "TokenKind.def"
        default: return false;
        }
    }
};

}

#endif // GLU_TOKENS_HPP
