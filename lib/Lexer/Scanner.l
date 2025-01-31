
%{
#include "Scanner.hpp"

#include "llvm/ADT/StringSwitch.h"

#define TK(tt) return TokenKind::tt

#define yyterminate() TK(eofTok);

using glu::TokenKind;

static TokenKind identifyKeyword(char const *id);
%}

%option noyywrap
%option c++
%option yyclass="Scanner"

%x SC_COMMENT

line_comment "//"[^\n]*

digit [0-9]
alpha [a-zA-Z]
hex_digit [0-9a-fA-F]

ticked_id "`"{ticked_id_char}+"`"
ticked_id_char ("``"|[^`])
plain_id {alpha}({alpha}|{digit}|_)*

decimal_lit {digit}+
octal_lit 0o[0-7]+
hex_lit 0x{hex_digit}+
bin_lit 0b[01]+

num {decimal_lit}|{octal_lit}|{hex_lit}|{bin_lit}

float_lit {decimal_lit}("."{decimal_lit})?

string_lit "\""({escaped_char}|[^"])*"\""
escaped_char "\\"([nrt"'\\]|u{hex_digit}{4}|U{hex_digit}{8})


%%
  int comment_nesting = 0;

"/*"           { BEGIN(SC_COMMENT); }
<SC_COMMENT>{
"/*"           { ++comment_nesting; }
"*"+"/"        { if (comment_nesting) --comment_nesting;
                 else BEGIN(INITIAL); }
"*"+           ;
[^/*\n]+       ;
[/]            ;
\n             ;
}

{line_comment} ;
[ \t\r]+       ;
\n             ;

{num}          TK(intLitTok);
{ticked_id}    TK(identTok);
{plain_id}     return identifyKeyword(yytext);
{float_lit}    TK(floatLitTok);
{string_lit}   TK(stringLitTok);

.              TK(unknownCharErrorTok);
%%

static TokenKind identifyKeyword(char const *id) {
    return llvm::StringSwitch<TokenKind>(id)
    #define GLU_KEYWORD(K) .Case(#K, TokenKind::K##KwTok)
    #include "Basic/TokenKind.def"
    .Default(TokenKind::identTok);
}
