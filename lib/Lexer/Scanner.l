
%{
#include "Scanner.hpp"

#include "llvm/ADT/StringSwitch.h"

#define TK(tt) return TokenKind::tt

#define yyterminate() { fatal_end = true; TK(eofTok); }

#define YY_USER_ACTION _bufOffset += yyleng;
#define SAVE_TOKEN_LOC _bufStartOffset = _bufOffset - yyleng
#define UNSAVE_TOKEN_LOC _bufStartOffset = -1

using glu::TokenKind;

static TokenKind identifyKeyword(char const *id);
%}

%option noyywrap
%option c++
%option yyclass="Scanner"

%x SC_COMMENT
%x SC_STRING

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

escaped_char "\\"([nrt"'\\]|u{hex_digit}{4}|U{hex_digit}{8})


%%
  int comment_nesting = 0;
  if (fatal_end) yyterminate();

"/*"           { BEGIN(SC_COMMENT); SAVE_TOKEN_LOC; }
<SC_COMMENT>{
"/*"           { ++comment_nesting; }
"*"+"/"        { if (comment_nesting) --comment_nesting;
                 else { BEGIN(INITIAL); UNSAVE_TOKEN_LOC; } }
"*"+           ;
[^/*\n]+       ;
[/]            ;
\n             ;
<<EOF>>        { fatal_end = true; TK(unterminatedBlockCommentErrorTok); }
}

{line_comment} ;
[ \t\r]+       ;
\n             ;

"\""           { BEGIN(SC_STRING); SAVE_TOKEN_LOC; }
<SC_STRING>{
"\""           { BEGIN(INITIAL); TK(stringLitTok); }
{escaped_char} ;
[^"\n]+        ;
\n             ; // allow multiline strings?
<<EOF>>        { fatal_end = true; TK(unterminatedStringLitErrorTok); }
}

{num}          TK(intLitTok);
{ticked_id}    TK(identTok);
{plain_id}     return identifyKeyword(yytext);
{float_lit}    TK(floatLitTok);

"..."          TK(rangeOpTok);
"..<"          TK(exclusiveRangeOpTok);
"::<"          TK(coloncolonLtTok);

"::"           TK(coloncolonTok);
"->"           TK(arrowTok);
"=="           TK(eqOpTok);
"!="           TK(neOpTok);
"<="           TK(leOpTok);
">="           TK(geOpTok);
"&&"           TK(andOpTok);
"||"           TK(orOpTok);
"<<"           TK(bitLShiftOpTok);
">>"           TK(bitRShiftOpTok);
".*"           TK(derefOpTok);

"+"            TK(plusOpTok);
"-"            TK(subOpTok);
"*"            TK(mulOpTok);
"/"            TK(divOpTok);
"%"            TK(modOpTok);
"<"            TK(ltOpTok);
">"            TK(gtOpTok);
"&"            TK(bitAndOpTok);
"|"            TK(bitOrOpTok);
"^"            TK(bitXorOpTok);
"!"            TK(notOpTok);
"~"            TK(complOpTok);

"("            TK(lParenTok);
")"            TK(rParenTok);
"{"            TK(lBraceTok);
"}"            TK(rBraceTok);
"["            TK(lBracketTok);
"]"            TK(rBracketTok);
"."            TK(dotTok);
","            TK(commaTok);
":"            TK(colonTok);
";"            TK(semiTok);
"="            TK(equalTok);
"\\"           TK(backslashTok);
"?"            TK(questionTok);
"@"            TK(atTok);


.              TK(unknownCharErrorTok);
%%

static TokenKind identifyKeyword(char const *id) {
    return llvm::StringSwitch<TokenKind>(id)
    #define GLU_KEYWORD(K) .Case(#K, TokenKind::K##KwTok)
    #include "Basic/TokenKind.def"
    .Default(TokenKind::identTok);
}

glu::Token glu::Scanner::nextToken()
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
    _prevToken = _currentToken;
    _currentToken = glu::Token(
        kind, llvm::StringRef(_buf->getBufferStart() + offset, len)
    );
    return _currentToken;
}
