#ifndef GLU_TOKENS_HPP
#define GLU_TOKENS_HPP

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

namespace glu {

enum class TokenKind {
#define TOKEN(Name) Name##Tok,
#include "TokenKind.def"
    numberOfTok
};

class Token {
    /// The kind of the token
    TokenKind _kind;

    /// The actual lexeme of the token. Must point to a location in the source
    /// buffer.
    llvm::StringRef _lexeme;

    /// The data associated with the token. This is used for string literals.
    llvm::StringRef _data;

public:
    // Bison needs a default constructor
    Token() : _kind(TokenKind::eofTok), _lexeme(""), _data("") { }
    Token(TokenKind kind, llvm::StringRef lexeme, llvm::StringRef data = "")
        : _kind(kind), _lexeme(lexeme), _data(data)
    {
    }

    TokenKind getKind() const { return _kind; }
    void setKind(TokenKind kind) { _kind = kind; }

    llvm::StringRef getLexeme() const { return _lexeme; }

    llvm::StringRef getData() const { return _data; }

    bool is(TokenKind kind) const { return _kind == kind; }
    bool isNot(TokenKind kind) const { return _kind != kind; }

    /// True if the token is any keyword.
    bool isKeyword() const
    {
        switch (_kind) {
#define KEYWORD(X)                            \
case glu::TokenKind::X##Kw##Tok: return true;
#include "TokenKind.def"
        default: return false;
        }
    }
};

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &os, TokenKind kind)
{
    switch (kind) {
#define TOKEN(Name)                            \
case TokenKind::Name##Tok: return os << #Name;
#include "TokenKind.def"
    default: return os << "unknown";
    }
}

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &os, Token const &token)
{
    os << token.getLexeme() << " (" << token.getKind() << ")";
    return os;
}

}

#endif // GLU_TOKENS_HPP
