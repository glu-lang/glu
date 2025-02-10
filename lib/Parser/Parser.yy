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
%token <glu::Token> eof 0 "eof"
%token <glu::Token> ident 1 "ident"

%token <glu::Token> unterminatedBlockCommentError 2 "unterminatedBlockComment"
%token <glu::Token> unterminatedStringLitError 3 "unterminatedStringLit"
%token <glu::Token> unknownCharError 4 "unknownChar"

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
%token <glu::Token> coloncolon 39 "::"

%token <glu::Token> plusOp 40 "+"
%token <glu::Token> subOp 41 "-"
%token <glu::Token> mulOp 42 "*"
%token <glu::Token> divOp 43 "/"
%token <glu::Token> modOp 44 "%"
%token <glu::Token> eqOp 45 "=="
%token <glu::Token> neOp 46 "!="
%token <glu::Token> ltOp 47 "<"
%token <glu::Token> leOp 48 "<="
%token <glu::Token> gtOp 49 ">"
%token <glu::Token> geOp 50 ">="
%token <glu::Token> andOp 51 "&&"
%token <glu::Token> orOp 52 "||"
%token <glu::Token> bitAndOp 53 "&"
%token <glu::Token> bitOrOp 54 "|"
%token <glu::Token> bitXorOp 55 "^"
%token <glu::Token> bitLShiftOp 56 "<<"
%token <glu::Token> bitRShiftOp 57 ">>"
%token <glu::Token> rangeOp 58 "..."
%token <glu::Token> exclusiveRangeOp 59 "..<"
%token <glu::Token> notOp 60 "!"
%token <glu::Token> complOp 61 "~"

%token <glu::Token> intLit 62 "int"
%token <glu::Token> floatLit 63 "float"
%token <glu::Token> stringLit 64 "string"

%token <glu::Token> uniqueKw 65 "unique"
%token <glu::Token> sharedKw 66 "shared"

%%

document:
      /* empty */
    | document top_level
    ;

top_level:
      import_declaration
    | type_declaration
    /* | function_declaration */
    | statement
    | expression
    | type
    ;

attributes:
      /* empty */
    | attributes attribute
    ;

attribute:
      at ident
    ;

import_declaration:
      importKw import_path semi
    {
        std::cerr << "Parsed import declaration" << std::endl;
    }
    ;

import_path:
      ident
    | import_path coloncolon ident
    | import_path coloncolon mulOp
    | import_path coloncolon lBrace import_item_list rBrace
    ;

import_item_list:
      import_path
    | import_item_list comma import_path
    ;

type_declaration:
      struct_declaration
    | enum_declaration 
    | typealias_declaration
    ;

struct_declaration:
      attributes structKw ident template_definition_opt struct_body semi
      { 
          std::cerr << "Parsed struct declaration" << std::endl;
      }
    ;

template_definition_opt:
      /* empty */
    | template_definition
    ;

template_definition:
      ltOp template_parameter_list gtOp
    ;

template_parameter_list:
      template_parameter
    | template_parameter_list comma template_parameter
    ;

template_parameter:
      ident
    ;

struct_body:
      lBrace struct_field_list rBrace
    ;

struct_field_list:
      struct_field
    | struct_field_list comma struct_field
    ;

struct_field:
      ident colon type
    | ident colon type equal expression
    ;

enum_declaration:
      attributes enumKw ident colon type enum_body semi
    {
        std::cerr << "Parsed enum declaration" << std::endl;
    }
    ;

enum_body:
      lBrace enum_variant_list rBrace
    ;

enum_variant_list:
      enum_variant
    | enum_variant_list comma enum_variant
    ;

enum_variant:
      ident
    | ident equal expression
    ;

typealias_declaration:
      attributes typealiasKw ident template_definition_opt equal type semi
    {
        std::cerr << "Parsed typealias declaration" << std::endl;
    }
    ;

function_declaration:
      attributes funcKw ident template_definition_opt function_signature function_body
    ;

function_signature:
      lParen parameter_list_opt rParen arrow type
    | lParen parameter_list_opt rParen
    ;

parameter_list_opt:
      /* empty */
    | parameter_list
    ;

parameter_list:
      parameter
    | parameter_list comma parameter
    ;

parameter:
      ident colon type
    | ident colon type equal expression
    ;

function_body:
      block
    | semi
    ;

block:
      lBrace statement_list rBrace
    ;

statement_list:
      /* empty */
    | statement_list statement
    ;

statement:
      block
    | expression semi
    | var_stmt
    | let_stmt
    | return_stmt
    | if_stmt
    | while_stmt
    | for_stmt
    | break_stmt
    | continue_stmt
    | assignment_stmt
    | semi
    ;

var_stmt:
      varKw ident type_opt equal_opt expression_opt semi
    {
        std::cerr << "Parsed var statement" << std::endl;
    }
    ;

type_opt:
      /* empty */
    | colon type
    ;

equal_opt:
      /* empty */
    | equal
    ;

expression_opt:
      /* empty */
    | expression
    ;

let_stmt:
      letKw ident type_opt equal expression semi
    {
        std::cerr << "Parsed let statement" << std::endl;
    }
    ;

return_stmt:
      returnKw expression_opt semi
    {
        std::cerr << "Parsed return statement" << std::endl;
    }
    ;

if_stmt:
      ifKw expression block else_opt
    {
        std::cerr << "Parsed if statement" << std::endl;
    }
    ;

else_opt:
      /* empty */
    | elseKw statement
    ;

while_stmt:
      whileKw expression block
    {
        std::cerr << "Parsed while statement" << std::endl;
    }
    ;

for_stmt:
      forKw ident inKw expression block
    {
        std::cerr << "Parsed for statement" << std::endl;
    }
    ;

break_stmt:
      breakKw semi
    {
        std::cerr << "Parsed break statement" << std::endl;
    }
    ;

continue_stmt:
      continueKw semi
    {
        std::cerr << "Parsed continue statement" << std::endl;
    }
    ;

assignment_stmt:
      expression equal expression semi
    {
        std::cerr << "Parsed assignment statement" << std::endl;
    }
    ;

expression:
      literal
    | ident
    | function_call
    | binary_expression
    | unary_expression
    | paren_expression
    | initializer_list
    | ternary_expression
    | field_access
    | subscript
    | cast_expression
    ;

function_call:
      namespaced_identifier template_arguments_opt lParen rParen
    | namespaced_identifier template_arguments_opt lParen argument_list rParen
    ;

template_arguments_opt:
      /* empty */
    | template_arguments
    ;

template_arguments:
      ltOp type_list gtOp
    ;

type_list:
      type
    | type_list comma type
    ;

argument_list_opt:
      /* empty */
    | argument_list
    ;

argument_list:
      expression
    | argument_list comma expression
    ;

binary_expression:
      expression binary_operator expression
    ;

binary_operator:
      plusOp
    | subOp
    | mulOp
    | divOp
    | modOp
    | eqOp
    | neOp
    | ltOp
    | leOp
    | gtOp
    | geOp
    | andOp
    | orOp
    | bitAndOp
    | bitOrOp
    | bitXorOp
    | bitLShiftOp
    | bitRShiftOp
    | rangeOp
    | exclusiveRangeOp
    ;

unary_expression:
      unary_operator expression
    ;

unary_operator:
      plusOp
    | subOp
    | notOp
    | complOp
    | bitAndOp
    ;

paren_expression:
      lParen expression rParen
    ;

initializer_list:
      lBrace argument_list_opt rBrace
    ;

ternary_expression:
      expression question expression colon expression
    ;

field_access:
      expression dot ident
    | expression dot mulOp
    ;

subscript:
      expression lBracket expression rBracket
    ;

cast_expression:
      expression asKw type
    ;

type:
      namespaced_identifier template_arguments_opt
    | function_type
    | array_type
    | pointer_type
    ;

function_type:
      lParen rParen arrow type
    | lParen non_empty_type_list rParen arrow type
    ;

non_empty_type_list:
      type
    | non_empty_type_list comma type
    ;

array_type:
      type lBracket intLit rBracket
    ;

pointer_type:
      mulOp unique_shared_opt type
    ;

unique_shared_opt:
      /* empty */
    | uniqueKw
    | sharedKw
    ;

literal:
      boolean_literal
    | intLit
    | floatLit
    | stringLit
    ;

boolean_literal:
      trueKw
    | falseKw
    ;

namespaced_identifier:
      ident ns_id_list_opt
    ;

ns_id_list_opt:
      /* empty */
    | ns_id_list
    ;

ns_id_list:
      coloncolon ident
    | ns_id_list coloncolon ident
    ;

%%

void glu::BisonParser::error(const std::string& msg) {
    std::cerr << "Error: " << msg << std::endl;
}
