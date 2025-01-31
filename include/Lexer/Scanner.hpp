#ifndef GLU_LEXER_SCANNER_HPP
#define GLU_LEXER_SCANNER_HPP

#if !defined(yyFlexLexerOnce)
    #include <FlexLexer.h>
#endif

// Change return type and name of the yylex function
#undef YY_DECL
#define YY_DECL glu::TokenKind glu::Scanner::getNextToken()

#include "Basic/Tokens.hpp"

namespace glu {

class Scanner : public yyFlexLexer {
public:
    Scanner(std::istream *in = nullptr, std::ostream *out = nullptr)
        : yyFlexLexer(in, out)
    {
    }
    virtual ~Scanner() = default;

    glu::TokenKind getNextToken();
    std::string getTokenText() const { return yytext; }

private:
    bool fatal_end = false;
};

}
#endif // GLU_LEXER_SCANNER_HPP
