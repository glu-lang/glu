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

%token <glu::Token> uniqueKw 5 "unique"
%token <glu::Token> sharedKw 6 "shared"

%token <glu::Token> structKw 7 "struct"
%token <glu::Token> unionKw 8 "union"
%token <glu::Token> enumKw 9 "enum"
%token <glu::Token> typealiasKw 10 "typealias"
%token <glu::Token> funcKw 11 "func"
%token <glu::Token> letKw 12 "let"
%token <glu::Token> varKw 13 "var"
%token <glu::Token> importKw 14 "import"

%token <glu::Token> ifKw 15 "if"
%token <glu::Token> elseKw 16 "else"
%token <glu::Token> whileKw 17 "while"
%token <glu::Token> forKw 18 "for"
%token <glu::Token> returnKw 19 "return"
%token <glu::Token> breakKw 20 "break"
%token <glu::Token> continueKw 21 "continue"
%token <glu::Token> inKw 22 "in"

%token <glu::Token> trueKw 23 "true"
%token <glu::Token> falseKw 24 "false"
%token <glu::Token> asKw 25 "as"

%token <glu::Token> lParen 26 "("
%token <glu::Token> rParen 27 ")"
%token <glu::Token> lBrace 28 "{"
%token <glu::Token> rBrace 29 "}"
%token <glu::Token> lBracket 30 "["
%token <glu::Token> rBracket 31 "]"
%token <glu::Token> dot 32 "."
%token <glu::Token> comma 33 ","
%token <glu::Token> colon 34 ":"
%token <glu::Token> semi 35 ";"
%token <glu::Token> arrow 36 "->"
%token <glu::Token> equal 37 "="
%token <glu::Token> backslash 38 "\\"
%token <glu::Token> question 39 "?"
%token <glu::Token> at 40 "@"
%token <glu::Token> coloncolon 41 "::"
%token <glu::Token> coloncolonLt 42 "::<"

%token <glu::Token> plusOp 43 "+"
%token <glu::Token> subOp 44 "-"
%token <glu::Token> mulOp 45 "*"
%token <glu::Token> divOp 46 "/"
%token <glu::Token> modOp 47 "%"
%token <glu::Token> eqOp 48 "=="
%token <glu::Token> neOp 49 "!="
%token <glu::Token> ltOp 50 "<"
%token <glu::Token> leOp 51 "<="
%token <glu::Token> gtOp 52 ">"
%token <glu::Token> geOp 53 ">="
%token <glu::Token> andOp 54 "&&"
%token <glu::Token> orOp 55 "||"
%token <glu::Token> bitAndOp 56 "&"
%token <glu::Token> bitOrOp 57 "|"
%token <glu::Token> bitXorOp 58 "^"
%token <glu::Token> bitLShiftOp 59 "<<"
%token <glu::Token> bitRShiftOp 60 ">>"
%token <glu::Token> rangeOp 61 "..."
%token <glu::Token> exclusiveRangeOp 62 "..<"
%token <glu::Token> notOp 63 "!"
%token <glu::Token> complOp 64 "~"

%token <glu::Token> intLit 65 "int"
%token <glu::Token> floatLit 66 "float"
%token <glu::Token> stringLit 67 "string"

// --- Precedence and associativity declarations ---
%nonassoc TERNARY
%nonassoc asKw
%left orOp
%left andOp
%nonassoc eqOp neOp
%nonassoc ltOp leOp gtOp geOp
%left plusOp subOp
%left mulOp divOp modOp
%right notOp complOp
%right PREFIX_UNARY
%nonassoc POSTFIX

%start document

%%

/*--------------------------------*/
/* High-level rules               */
/*--------------------------------*/

document:
      top_level_list
    ;

top_level_list:
      /* empty */
    | top_level top_level_list
    ;

top_level:
      import_declaration
          { std::cerr << "Parsed top level import declaration" << std::endl; }
    | type_declaration
          { std::cerr << "Parsed top level type declaration" << std::endl; }
    | function_declaration
          { std::cerr << "Parsed top level function declaration" << std::endl; }
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
    { std::cerr << "Parsed import declaration" << std::endl; }
    ;

import_path:
      import_item
    | identifier_sequence coloncolon import_item
    ;

identifier_sequence:
      ident
    | identifier_sequence coloncolon ident
    ;

import_item:
      ident
    | mulOp
    | lBrace import_item_list_opt rBrace
    ;

import_item_list_opt:
      /* empty */
    | import_item_list
    ;

import_item_list:
      import_path
    | import_item_list comma import_path
    | import_item_list comma
    ;

/*--------------------------------*/
/* Type declarations              */
/*--------------------------------*/

type_declaration:
      struct_declaration
    | enum_declaration 
    | typealias_declaration
    ;

struct_declaration:
      attributes structKw ident template_definition_opt struct_body
    { std::cerr << "Parsed struct declaration" << std::endl; }
    ;

template_definition_opt:
      /* empty */
    | template_definition
    ;

template_definition:
      ltOp template_parameter_list_opt gtOp
    ;

template_parameter_list_opt:
      /* empty */
    | template_parameter_list
    ;

template_parameter_list:
      template_parameter
    | template_parameter_list comma template_parameter
    | template_parameter_list comma
    ;

template_parameter:
      ident
    ;

struct_body:
      lBrace struct_field_list_opt rBrace
    ;

struct_field_list_opt:
      /* empty */
    | struct_field_list
    ;

struct_field_list:
      struct_field
    | struct_field_list comma struct_field
    | struct_field_list comma
    ;

struct_field:
      ident colon type
    | ident colon type equal expression
    ;

enum_declaration:
      attributes enumKw ident colon type enum_body
    { std::cerr << "Parsed enum declaration" << std::endl; }
    ;

enum_body:
      lBrace enum_variant_list_opt rBrace
    ;

enum_variant_list_opt:
      /* empty */
    | enum_variant_list
    ;

enum_variant_list:
      enum_variant
    | enum_variant_list comma enum_variant
    | enum_variant_list comma
    ;

enum_variant:
      ident
    | ident equal expression
    ;

typealias_declaration:
      attributes typealiasKw ident template_definition_opt equal type semi
    { std::cerr << "Parsed typealias declaration" << std::endl; }
    ;

/*--------------------------------*/
/* Function declarations          */
/*--------------------------------*/

function_declaration:
      attributes funcKw ident template_definition_opt function_signature function_body
    { std::cerr << "Parsed function declaration" << std::endl; }
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
    | parameter_list comma
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
    | expression_stmt semi
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

expression_stmt:
      conditional_expression_stmt
    ;

conditional_expression_stmt:
  logical_or_expression_stmt
    | logical_or_expression_stmt question expression colon conditional_expression_stmt %prec TERNARY
      { std::cerr << "Parsed ternary expression" << std::endl; }
    ;

logical_or_expression_stmt:
  logical_or_expression_stmt orOp logical_and_expression_stmt
      { std::cerr << "Parsed logical or" << std::endl; }
    | logical_and_expression_stmt

logical_and_expression_stmt:
  logical_and_expression_stmt andOp conditional_expression_stmt
      { std::cerr << "Parsed logical and" << std::endl; }
    | function_call_stmt
    ;

function_call_stmt:
  namespaced_identifier
    | lParen expression_stmt rParen
    | function_call_stmt function_call
    ;

function_call:
  function_template_arguments lParen argument_list_opt rParen %prec POSTFIX
    ; 

function_template_arguments:
  /* empty */
      { std::cerr << "Parsed empty function template arguments" << std::endl; }
    | coloncolonLt type_list gtOp
      { std::cerr << "Parsed function template arguments" << std::endl; }
    ;

var_stmt:
      varKw ident type_opt initializer_opt semi
    { std::cerr << "Parsed var statement" << std::endl; }
    ;

type_opt:
      /* empty */
    | colon type
    ;

initializer_opt:
      /* empty */
    | equal expression
    ;

let_stmt:
      letKw ident type_opt equal expression semi
    { std::cerr << "Parsed let statement" << std::endl; }
    ;

return_stmt:
      returnKw expression_opt semi
    { std::cerr << "Parsed return statement" << std::endl; }
    ;

expression_opt:
      /* empty */
    | expression
    ;

if_stmt:
      ifKw expression block else_opt
    { std::cerr << "Parsed if statement" << std::endl; }
    ;

else_opt:
      /* empty */
    | elseKw statement
    ;

while_stmt:
      whileKw expression block
    { std::cerr << "Parsed while statement" << std::endl; }
    ;

for_stmt:
      forKw ident inKw expression block
    { std::cerr << "Parsed for statement" << std::endl; }
    ;

break_stmt:
      breakKw semi
    { std::cerr << "Parsed break statement" << std::endl; }
    ;

continue_stmt:
      continueKw semi
    { std::cerr << "Parsed continue statement" << std::endl; }
    ;

assignment_stmt:
      expression equal expression semi
    { std::cerr << "Parsed assignment statement" << std::endl; }
    ;

/*--------------------------------*/
/* Grammar of expressions         */
/* separated into multiple levels */
/*--------------------------------*/

/* Level 1: expression (cast, etc.) */
expression:
  cast_expression
    ;

cast_expression:
  conditional_expression
    | conditional_expression asKw type %prec asKw
      { std::cerr << "Parsed cast expression" << std::endl; }
    ;

/* Level 2: conditional expression (ternary) */
conditional_expression:
  logical_or_expression
    | logical_or_expression question expression colon conditional_expression %prec TERNARY
      { std::cerr << "Parsed ternary expression" << std::endl; }
    ;

/* Level 3: logical OR */
logical_or_expression:
  logical_or_expression orOp logical_and_expression
      { std::cerr << "Parsed logical or" << std::endl; }
    | logical_and_expression
    ;

/* Level 4: logical AND */
logical_and_expression:
  logical_and_expression andOp equality_expression
      { std::cerr << "Parsed logical and" << std::endl; }
    | equality_expression
    ;

/* Level 5: equality operators (nonassociative) */
equality_expression:
  relational_expression eqOp relational_expression
      { std::cerr << "Parsed equality" << std::endl; }
    | relational_expression neOp relational_expression
      { std::cerr << "Parsed inequality" << std::endl; }
    | relational_expression
    ;

/* Level 6: relational operators (nonassociative) */
relational_expression:
  additive_expression ltOp additive_expression
      { std::cerr << "Parsed less than" << std::endl; }
    | additive_expression leOp additive_expression
      { std::cerr << "Parsed less or equal" << std::endl; }
    | additive_expression gtOp additive_expression
      { std::cerr << "Parsed greater than" << std::endl; }
    | additive_expression geOp additive_expression
      { std::cerr << "Parsed greater or equal" << std::endl; }
    | additive_expression
    ;

/* Level 7: addition/subtraction */
additive_expression:
  additive_expression plusOp multiplicative_expression
      { std::cerr << "Parsed addition" << std::endl; }
    | additive_expression subOp multiplicative_expression
      { std::cerr << "Parsed subtraction" << std::endl; }
    | multiplicative_expression
    ;

/* Level 8: multiplication/division/modulo */
multiplicative_expression:
  multiplicative_expression mulOp unary_expression
      { std::cerr << "Parsed multiplication" << std::endl; }
    | multiplicative_expression divOp unary_expression
      { std::cerr << "Parsed division" << std::endl; }
    | multiplicative_expression modOp unary_expression
      { std::cerr << "Parsed modulo" << std::endl; }
    | unary_expression
    ;

/* Level 9: unary expressions */
unary_expression:
  plusOp unary_expression %prec PREFIX_UNARY
      { std::cerr << "Parsed unary plus" << std::endl; }
    | subOp unary_expression %prec PREFIX_UNARY
      { std::cerr << "Parsed unary minus" << std::endl; }
    | notOp unary_expression
      { std::cerr << "Parsed unary not" << std::endl; }
    | complOp unary_expression
      { std::cerr << "Parsed unary complement" << std::endl; }
    | bitAndOp unary_expression
      { std::cerr << "Parsed unary bitand" << std::endl; }
    | postfix_expression
    ;

/* Level 10: postfix (function call, subscript, field access) */
postfix_expression:
  primary_expression
      // log in function_call
    | postfix_expression function_call
    | postfix_expression lBracket expression rBracket %prec POSTFIX
      { std::cerr << "Parsed subscript expression" << std::endl; }
    | postfix_expression dot ident %prec POSTFIX
      { std::cerr << "Parsed field access" << std::endl; }
    ;

/* Level 11: primary expressions */
primary_expression:
  literal
    | namespaced_identifier
    | lParen expression rParen
    | lBrace argument_list_opt rBrace
      { std::cerr << "Parsed initializer list" << std::endl; }
    ;

/*--------------------------------*/
/* Argument list                  */
/*--------------------------------*/

argument_list_opt:
    /* empty */
    | argument_list
    ;

argument_list:
  expression
    | argument_list comma expression
    ;

/*--------------------------------*/
/* Type grammar                   */
/*--------------------------------*/

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
    | type_list comma
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
    type non_empty_type_list_tail
    ;

non_empty_type_list_tail:
      /* empty */
    | comma type non_empty_type_list_tail
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
      ident ns_id_list_tail
    ;

ns_id_list_tail:
      /* empty */
    | coloncolon ident ns_id_list_tail
    ;

%%

void glu::BisonParser::error(const std::string& msg) {
    std::cerr << "Error: " << msg << std::endl;
}
