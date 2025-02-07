// A macro to convert a symbol to a string
#define STRINGIFY(x) #x

// Redefine TOKEN to accept either a single argument (and deduce the literal via
// tokenization) or two arguments (the second being the desired literal).
#define _TOKEN_IMPL(NAME, STR) %token <glu::Token> NAME __COUNTER__ STR
#define _GET_TOKEN_MACRO(_1, _2, NAME, ...) NAME
#define TOKEN1(NAME) _TOKEN_IMPL(NAME, STRINGIFY(NAME))
#define TOKEN2(NAME, STR) _TOKEN_IMPL(NAME, STR)
#define TOKEN(...) _GET_TOKEN_MACRO(__VA_ARGS__, TOKEN2, TOKEN1)(__VA_ARGS__)

// For keywords: for DECL_KEYWORD, STMT_KEYWORD, EXPR_KEYWORD, etc. we want to
// get the version with the "Kw" suffix and the corresponding literal (e.g.,
// "struct" for DECL_KEYWORD(struct))
#define KEYWORD(Keyword) TOKEN(Keyword##Kw, STRINGIFY(Keyword))
#define GLU_KEYWORD(Keyword) KEYWORD(Keyword)
#define DECL_KEYWORD(Keyword) KEYWORD(Keyword)
#define STMT_KEYWORD(Keyword) KEYWORD(Keyword)
#define EXPR_KEYWORD(Keyword) KEYWORD(Keyword)
#define GIL_KEYWORD(Keyword) KEYWORD(Keyword)

// For punctuators, operators, and literals, we retrieve the literal from the
// second argument
#define PUNCTUATOR(Keyword, value) TOKEN(Keyword, value)
#define OPERATOR(Name, value) TOKEN(Name##Op, value)
#define LITERAL(Name) TOKEN(Name##Lit, STRINGIFY(Name))
#define ERROR(Name) TOKEN(Name##Error, STRINGIFY(Name))

#include "Basic/TokenKind.def"
