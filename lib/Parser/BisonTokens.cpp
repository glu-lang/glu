#define STRINGIFY(x) #x

// To declare a token by deducing the literal from its name
#define TOKEN(NAME) % token<glu::Token> NAME __COUNTER__ STRINGIFY(NAME)

// To declare a token by explicitly specifying the literal
#define TOKEN_STR(NAME, LITERAL) % token<glu::Token> NAME __COUNTER__ LITERAL

// For keywords
#define KEYWORD(Keyword) TOKEN_STR(Keyword##Kw, STRINGIFY(Keyword))

// For punctuations, operators, and literals
#define PUNCTUATOR(Keyword, value) TOKEN_STR(Keyword, value)
#define OPERATOR(Name, value, ...) TOKEN_STR(Name##Op, value)
#define LITERAL(Name) TOKEN_STR(Name##Lit, STRINGIFY(Name))
#define ERROR(Name) TOKEN_STR(Name##Error, STRINGIFY(Name))

#include "Basic/TokenKind.def"
