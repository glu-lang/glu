%{
#include "Parser.hpp"
#include "Basic/Tokens.hpp"
#include <iostream>
%}

// Bison options
%require "3.8"
%language "c++"
//%debug

// Namespace for the generated parser
%define api.namespace {glu}
%define api.parser.class {BisonParser}
%define api.token.constructor
%define api.value.type variant
%define parse.error verbose

// Parameters for the scanner
%parse-param { glu::Scanner& scanner }
%lex-param { glu::Scanner& scanner }

%code requires {
    #include "Basic/SourceLocation.hpp"
    namespace glu {
        class Scanner;
        class Token;
    }
}

%code {
    #include "Lexer/Scanner.hpp"
    #include "Basic/Tokens.hpp"

    static int tokenKindToInt(glu::TokenKind kind) {
        return static_cast<int>(kind);
    }


    // Redefine yylex to call our scanner and return a symbol
    #define yylex(scanner) ([&]() -> BisonParser::symbol_type { \
        glu::Token tok = scanner.nextToken(); \
        return BisonParser::symbol_type(tokenKindToInt(tok.getKind()), std::move(tok)); \
    }())
}

// --- Explicit declaration of tokens with their values ---
// The values used here are those defined in the TokenKind enum in the order of TokenKind.def
%token <glu::Token> IDENT       1
%token <glu::Token> INT_LIT     61
%token <glu::Token> FLOAT_LIT   62
%token <glu::Token> STRING_LIT  63
%token <glu::Token> STRUCT      5
%token <glu::Token> UNION       6
%token <glu::Token> ENUM        7
%token <glu::Token> FUNC        9
%token <glu::Token> LET         10
%token <glu::Token> VAR         11
%token <glu::Token> IF          13
%token <glu::Token> ELSE        14
%token <glu::Token> WHILE       15
%token <glu::Token> FOR         16
%token <glu::Token> RETURN      17
%token <glu::Token> LPAREN      24
%token <glu::Token> RPAREN      25
%token <glu::Token> LBRACE      26
%token <glu::Token> RBRACE      27
%token <glu::Token> SEMI        33
%token <glu::Token> COMMA       31
%token <glu::Token> EQUAL       35

%%

program
    : /* empty */
    | declarations
    ;

declarations
    : declaration
    | declarations declaration
    ;

declaration
    : function_decl
    | var_decl
    | struct_decl
    ;

function_decl
    : FUNC IDENT LPAREN RPAREN compound_stmt
    { std::cout << "Parsed function declaration\n"; }
    ;

var_decl
    : LET IDENT SEMI
    | VAR IDENT SEMI
    { std::cout << "Parsed variable declaration\n"; }
    ;

struct_decl
    : STRUCT IDENT LBRACE struct_members RBRACE
    { std::cout << "Parsed struct declaration\n"; }
    ;

struct_members
    : /* empty */
    | struct_member
    | struct_members struct_member
    ;

struct_member
    : var_decl
    ;

compound_stmt
    : LBRACE statements RBRACE
    ;

statements
    : /* empty */
    | statement
    | statements statement
    ;

statement
    : expr_stmt
    | compound_stmt
    | if_stmt
    | while_stmt
    | return_stmt
    ;

expr_stmt
    : expr SEMI
    ;

if_stmt
    : IF LPAREN expr RPAREN statement
    | IF LPAREN expr RPAREN statement ELSE statement
    ;

while_stmt
    : WHILE LPAREN expr RPAREN statement
    ;

return_stmt
    : RETURN SEMI
    | RETURN expr SEMI
    ;

expr
    : IDENT
    | INT_LIT
    | FLOAT_LIT
    | STRING_LIT
    | LPAREN expr RPAREN
    ;

%%

void glu::BisonParser::error(const std::string& msg) {
    std::cerr << "Error: " << msg << std::endl;
}
