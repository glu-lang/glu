#define TOKEN(NAME) %token <glu::Token> NAME __COUNTER__
#include "Basic/TokenKind.def"
