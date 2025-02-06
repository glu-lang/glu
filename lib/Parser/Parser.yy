%{
#include "Parser.hpp"
#include "Basic/Tokens.hpp"
#include <iostream>
%}

// Bison options
%require "3.8"
%language "c++"

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

#define yylex(scanner) ([&]() -> BisonParser::symbol_type { \
    glu::Token tok = scanner.nextToken(); \
    return BisonParser::symbol_type(tokenKindToInt(tok.getKind()), std::move(tok)); \
}())

}

// Tokens definition
%token <glu::Token> IDENT "identifier"
%token <glu::Token> INT_LIT "integer literal"
%token <glu::Token> FLOAT_LIT "float literal"
%token <glu::Token> STRING_LIT "string literal"
%token <glu::Token> STRUCT "struct"
%token <glu::Token> UNION "union"
%token <glu::Token> ENUM "enum"
%token <glu::Token> FUNC "func"
%token <glu::Token> LET "let"
%token <glu::Token> VAR "var"
%token <glu::Token> IF "if"
%token <glu::Token> ELSE "else"
%token <glu::Token> WHILE "while"
%token <glu::Token> FOR "for"
%token <glu::Token> RETURN "return"
%token <glu::Token> LPAREN "("
%token <glu::Token> RPAREN ")"
%token <glu::Token> LBRACE "{"
%token <glu::Token> RBRACE "}"
%token <glu::Token> SEMI ";"
%token <glu::Token> COMMA ","
%token <glu::Token> EQUAL "="

%%

program
    : declarations
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
