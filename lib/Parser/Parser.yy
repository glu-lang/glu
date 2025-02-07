%{
#include "Parser.hpp"
#include "Basic/Tokens.hpp"
#include <iostream>
%}

// Bison options
%require "3.8"
%language "c++"
%debug

// Namespace for the generated parser
%define api.namespace {glu}
%define api.parser.class {BisonParser}
%define api.token.constructor
%define api.value.type variant
%define parse.error verbose

// Parameters for the scanner
%parse-param { glu::Scanner &scanner }
%lex-param { glu::Scanner &scanner }

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

    // Redefine yylex to call our scanner and return a symbol
    static glu::BisonParser::symbol_type yylex(glu::Scanner& scanner) {
        glu::Token tok = scanner.nextToken();
        return glu::BisonParser::symbol_type(
            static_cast<int>(tok.getKind()), std::move(tok)
        );
    }
}

// --- Explicit declaration of tokens with their values ---
// The values used here are those defined in the TokenKind enum in the order of TokenKind.def
%token <glu::Token> eof 0 "EOF"
%token <glu::Token> ident 1 "identifier"

%token <glu::Token> unterminatedBlockCommentError 2 "unterminated block comment"
%token <glu::Token> unterminatedStringLitError 3 "unterminated string literal"
%token <glu::Token> unknownCharError 4 "unknown character"

%token <glu::Token> structKw 5 "struct"
%token <glu::Token> unionKw 6 "union"
%token <glu::Token> enumKw 7 "enum"
%token <glu::Token> typealiasKw 8 "typealias"
%token <glu::Token> funcKw 9 "func"
%token <glu::Token> letKw 10 "let"
%token <glu::Token> varKw 11 "var"
%token <glu::Token> importKw 12 "import"

%token <glu::Token> ifKw 13 "if"
%token <glu::Token> elseKw 14 "else"
%token <glu::Token> whileKw 15 "while"
%token <glu::Token> forKw 16 "for"
%token <glu::Token> returnKw 17 "return"
%token <glu::Token> breakKw 18 "break"
%token <glu::Token> continueKw 19 "continue"
%token <glu::Token> inKw 20 "in"

%token <glu::Token> trueKw 21 "true"
%token <glu::Token> falseKw 22 "false"
%token <glu::Token> asKw 23 "as"

%token <glu::Token> lParen 24 "("
%token <glu::Token> rParen 25 ")"
%token <glu::Token> lBrace 26 "{"
%token <glu::Token> rBrace 27 "}"
%token <glu::Token> lBracket 28 "["
%token <glu::Token> rBracket 29 "]"
%token <glu::Token> dot 30 "."
%token <glu::Token> comma 31 ","
%token <glu::Token> colon 32 ":"
%token <glu::Token> semi 33 ";"
%token <glu::Token> arrow 34 "->"
%token <glu::Token> equal 35 "="
%token <glu::Token> backslash 36 "\\"
%token <glu::Token> question 37 "?"
%token <glu::Token> at 38 "@"

%token <glu::Token> plusOp 39 "+"
%token <glu::Token> subOp 40 "-"
%token <glu::Token> mulOp 41 "*"
%token <glu::Token> divOp 42 "/"
%token <glu::Token> modOp 43 "%"
%token <glu::Token> eqOp 44 "=="
%token <glu::Token> neOp 45 "!="
%token <glu::Token> ltOp 46 "<"
%token <glu::Token> leOp 47 "<="
%token <glu::Token> gtOp 48 ">"
%token <glu::Token> geOp 49 ">="
%token <glu::Token> andOp 50 "&&"
%token <glu::Token> orOp 51 "||"
%token <glu::Token> bitAndOp 52 "&"
%token <glu::Token> bitOrOp 53 "|"
%token <glu::Token> bitXorOp 54 "^"
%token <glu::Token> bitLShiftOp 55 "<<"
%token <glu::Token> bitRShiftOp 56 ">>"
%token <glu::Token> rangeOp 57 ".."
%token <glu::Token> exclusiveRangeOp 58 "..<"
%token <glu::Token> notOp 59 "!"
%token <glu::Token> complOp 60 "~"

%token <glu::Token> intLit 61 "integer literal"
%token <glu::Token> floatLit 62 "floating-point literal"
%token <glu::Token> stringLit 63 "string literal"

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
    : funcKw ident lParen rParen compound_stmt
    { std::cout << "Parsed " << ($2.getLexeme().str()) << " function declaration\n"; }
    ;

var_decl
    : letKw ident semi
    | varKw ident semi
    { std::cout << "Parsed variable declaration\n"; }
    ;

struct_decl
    : structKw ident lBrace struct_members rBrace
    { std::cout << "Parsed struct declaration\n"; }
    ;

struct_members
    : /* empty */
    | struct_members struct_member
    ;

struct_member
    : var_decl
    ;

compound_stmt
    : lBrace statements rBrace
    ;

statements
    : /* empty */
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
    : expr semi
    ;

if_stmt
    : ifKw lParen expr rParen statement
    | ifKw lParen expr rParen statement elseKw statement
    ;

while_stmt
    : whileKw lParen expr rParen statement
    ;

return_stmt
    : returnKw semi
    | returnKw expr semi
    ;

expr
    : ident
    | intLit
    | floatLit
    | stringLit
    | lParen expr rParen
    ;

%%

void glu::BisonParser::error(const std::string& msg) {
    std::cerr << "Error: " << msg << std::endl;
}
