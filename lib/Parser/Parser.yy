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
%parse-param { glu::ast::ASTContext &ctx }
%parse-param { glu::SourceManager &sm }
%lex-param { glu::Scanner &scanner }

%code requires {
    #include "AST/ASTContext.hpp"
    #include "AST/Decls.hpp"
    #include "AST/Types.hpp"
    #include "AST/Exprs.hpp"
    #include "AST/Stmts.hpp"
    #include "Basic/SourceLocation.hpp"

    namespace glu {
        class Scanner;
        class SourceManager;
        class Token;
    }

    using namespace glu;
    using namespace glu::ast;
    using namespace glu::types;

    #define LOC(tok) (sm.getSourceLocFromToken(tok))
    #define CREATE_NODE ctx.getASTMemoryArena().create
    #define CREATE_TYPE ctx.getTypesMemoryArena().create
}

%code {
    // Redefine yylex to call our scanner and return a symbol
    static glu::BisonParser::symbol_type yylex(glu::Scanner& scanner) {
        glu::Token tok = scanner.nextToken();
        return glu::BisonParser::symbol_type(
            static_cast<int>(tok.getKind()), std::move(tok)
        );
    }
}

%type <std::string> ns_id_list_tail

%type <ExprBase *> expression expression_opt initializer_opt function_call
%type <ExprBase *> boolean_literal cast_expression conditional_expression logical_or_expression logical_and_expression equality_expression relational_expression additive_expression multiplicative_expression unary_expression postfix_expression primary_expression literal
%type <ExprBase *> namespaced_identifier
%type <ExprBase *> postfix_expr_stmt primary_expr_stmt

%type <TypeBase *> type type_opt array_type primary_type pointer_type function_type_param_types function_type_param_types_tail

%type <DeclBase *> var_stmt let_stmt type_declaration struct_declaration enum_declaration typealias_declaration function_declaration

%type <StmtBase *> statement expression_stmt assignment_or_call_stmt return_stmt if_stmt while_stmt for_stmt break_stmt continue_stmt block statement_list
%type <glu::Token> equality_operator relational_operator additive_operator multiplicative_operator unary_operator

// --- Explicit declaration of tokens with their values ---
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
%token <glu::Token> derefOp 65 ".*"

%token <glu::Token> intLit 66 "int"
%token <glu::Token> floatLit 67 "float"
%token <glu::Token> stringLit 68 "string"

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
      {
        $$ = nullptr;
        std::cerr << "Parsed struct declaration" << std::endl;
      }
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
      ident colon type initializer_opt
    ;

enum_declaration:
      attributes enumKw ident colon type enum_body
      {
        $$ = nullptr;
        std::cerr << "Parsed enum declaration" << std::endl;
      }
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
      ident initializer_opt
    ;

typealias_declaration:
      attributes typealiasKw ident template_definition_opt equal type semi
      {
        $$ = CREATE_NODE<TypeAliasDecl>(ctx, LOC($3), nullptr, $3.getLexeme().str(), $6);
        std::cerr << "Parsed typealias declaration" << std::endl;
      }
    ;

/*--------------------------------*/
/* Function declarations          */
/*--------------------------------*/

function_declaration:
      attributes funcKw ident template_definition_opt function_signature function_body
      {
        $$ = nullptr;
        std::cerr << "Parsed function declaration" << std::endl;
      }
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
    ident colon type initializer_opt
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
    | semi
    ;

expression_stmt:
      assignment_or_call_stmt
    ;

assignment_or_call_stmt:
      postfix_expr_stmt equal expression
      {
        $$ = CREATE_NODE<AssignStmt>(LOC($2), $1, $2, $3);
        std::cerr << "Parsed assignment statement" << std::endl;
      }
    | postfix_expr_stmt function_call
      {
        $$ = nullptr;
        std::cerr << "Parsed function call statement" << std::endl;
      }
    | postfix_expr_stmt
      {
        // we create a dummy expression statement because all the ast is not implemented
        // and maybe $1 return null
        // TODO: function_call and initializer list
        auto loc = $1 ? $1->getLocation() : SourceLocation(0);
        $$ = CREATE_NODE<ExpressionStmt>(loc, $1);
        std::cerr << "Parsed expression statement" << std::endl;
      }
    ;

postfix_expr_stmt:
      primary_expr_stmt
    | postfix_expr_stmt lBracket expression rBracket %prec POSTFIX
      {
        $$ = CREATE_NODE<BinaryOpExpr>(LOC($2), $1, $2, $3);
        std::cerr << "Parsed subscript expression" << std::endl;
      }
    | postfix_expr_stmt dot ident %prec POSTFIX
      {
        $$ = CREATE_NODE<StructMemberExpr>(LOC($2), $1, $3.getLexeme());
        std::cerr << "Parsed field access" << std::endl;
      }
    | postfix_expr_stmt derefOp %prec POSTFIX
      {
        $$ = CREATE_NODE<UnaryOpExpr>(LOC($2), $1, $2);
        std::cerr << "Parsed dereference" << std::endl;
      }
    ;

primary_expr_stmt:
  namespaced_identifier
    | lParen expression rParen { $$ = $2; }
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
      {
        $$ = CREATE_NODE<VarDecl>(LOC($2), $2.getLexeme().str(), $3, $4);
        std::cerr << "Parsed var declaration: " << $2.getLexeme().str() << std::endl; 
      }
    ;

type_opt:
      /* empty */
        { $$ = nullptr; }
    | colon type
        { $$ = $2; }
    ;

initializer_opt:
      /* empty */
        { $$ = nullptr; }
    | equal expression
      {
         if ($2 == nullptr) {
             error("Missing expression after '='");
             YYERROR;
         }
         $$ = $2; 
      }
    ;

let_stmt:
      letKw ident type_opt equal expression semi
      { 
        $$ = CREATE_NODE<LetDecl>(LOC($2), $2.getLexeme().str(), $3, $5);
        std::cerr << "Parsed let declaration: " << $2.getLexeme().str() << std::endl;
      }
    ;

return_stmt:
      returnKw expression_opt semi
      {
        $$ = CREATE_NODE<ReturnStmt>(LOC($1), $2);
        std::cerr << "Parsed return statement" << std::endl;
      }
    ;

expression_opt:
      /* empty */
        { $$ = nullptr; }
    | expression
    ;

if_stmt:
      ifKw expression block else_opt
      {
        $$ = nullptr;
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
        $$ = nullptr;
        std::cerr << "Parsed while statement" << std::endl;
      }
    ;

for_stmt:
      forKw ident inKw expression block
      {
        $$ = nullptr;
        std::cerr << "Parsed for statement" << std::endl;
      }
    ;

break_stmt:
      breakKw semi
      {
        $$ = CREATE_NODE<BreakStmt>(LOC($1));
        std::cerr << "Parsed break statement" << std::endl;
      }
    ;

continue_stmt:
      continueKw semi
      {
        $$ = CREATE_NODE<ContinueStmt>(LOC($1));
        std::cerr << "Parsed continue statement" << std::endl;
      }
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
      {
        $$ = CREATE_NODE<CastExpr>(LOC($2), $1, $3);
        std::cerr << "Parsed cast expression" << std::endl;
      }
    ;

/* Level 2: conditional expression (ternary) */
conditional_expression:
      logical_or_expression
    | logical_or_expression question expression colon conditional_expression %prec TERNARY
      {
        $$ = nullptr;
        std::cerr << "Parsed ternary expression" << std::endl;
      }
    ;

/* Level 3: logical OR */
logical_or_expression:
      logical_or_expression orOp logical_and_expression
      {
        $$ = CREATE_NODE<BinaryOpExpr>(LOC($2), $1, $2, $3);
        std::cerr << "Parsed logical or" << std::endl;
      }
    | logical_and_expression
    ;

/* Level 4: logical AND */
logical_and_expression:
      logical_and_expression andOp equality_expression
      {
        $$ = CREATE_NODE<BinaryOpExpr>(LOC($2), $1, $2, $3);
        std::cerr << "Parsed logical and" << std::endl;
      }
    | equality_expression
    ;

equality_operator:
      eqOp
    | neOp
    ;

/* Level 5: equality operators (nonassociative) */
equality_expression:
      relational_expression equality_operator relational_expression
      {
        $$ = CREATE_NODE<BinaryOpExpr>(LOC($2), $1, $2, $3);
        std::cerr << "Parsed equality expression: " << $2.getLexeme().str() << std::endl;
      }
    | relational_expression
    ;

relational_operator:
      ltOp
    | leOp
    | gtOp
    | geOp
    ;

/* Level 6: relational operators (nonassociative) */
relational_expression:
      additive_expression ltOp additive_expression
      {
        $$ = CREATE_NODE<BinaryOpExpr>(LOC($2), $1, $2, $3);
        std::cerr << "Parsed relational expression: " << $2.getLexeme().str() << std::endl;
      }
    | additive_expression
    
  ;

additive_operator:
      plusOp
    | subOp
    ;

/* Level 7: addition/subtraction */
additive_expression:
      additive_expression additive_operator multiplicative_expression
      {
        $$ = CREATE_NODE<BinaryOpExpr>(LOC($2), $1, $2, $3);
        std::cerr << "Parsed additive expression: " << $2.getLexeme().str() << std::endl;
      }
    | multiplicative_expression
    ;

multiplicative_operator:
      mulOp
    | divOp
    | modOp
    ;

/* Level 8: multiplication/division/modulo */
multiplicative_expression:
      multiplicative_expression multiplicative_operator unary_expression
      {
        $$ = CREATE_NODE<BinaryOpExpr>(LOC($2), $1, $2, $3);
        std::cerr << "Parsed multiplicative expression: " << $2.getLexeme().str() << std::endl;
      }
    | unary_expression
    ;

unary_operator:
      plusOp
    | subOp
    | notOp
    | complOp
    | bitAndOp
    ;

/* Level 9: unary expressions */
unary_expression:
      unary_operator unary_expression %prec PREFIX_UNARY
      {
        $$ = CREATE_NODE<UnaryOpExpr>(LOC($1), $2, $1);
        std::cerr << "Parsed unary expression: " << $1.getLexeme().str() << std::endl;
      }
    | postfix_expression
    ;

/* Level 10: postfix (function call, subscript, field access) */
postfix_expression:
      primary_expression
    | postfix_expression function_call
        { $$ = nullptr; }
    | postfix_expression lBracket expression rBracket %prec POSTFIX
      {
        $$ = CREATE_NODE<BinaryOpExpr>(LOC($2), $1, $2, $3);
        std::cerr << "Parsed subscript expression" << std::endl;
      }
    | postfix_expression dot ident %prec POSTFIX
      {
        $$ = CREATE_NODE<StructMemberExpr>(LOC($2), $1, $3.getLexeme());
        std::cerr << "Parsed field access" << std::endl; }
    | postfix_expression derefOp %prec POSTFIX
      {
        $$ = CREATE_NODE<UnaryOpExpr>(LOC($2), $1, $2);
        std::cerr << "Parsed dereference" << std::endl;
      }
    ;

/* Level 11: primary expressions */
primary_expression:
      literal
    | namespaced_identifier
        { $$ = $1;}
    | lParen expression rParen
        { $$ = $2; }
    | lBrace argument_list_opt rBrace
      {
        $$ = nullptr;
        std::cerr << "Parsed initializer list" << std::endl;
      }
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
      array_type
    ;

array_type:
      primary_type
    | array_type lBracket intLit rBracket
    ;

primary_type:
      lParen type rParen { $$ = $2; }
    | lParen rParen arrow primary_type { $$ = $4; }
    | lParen type rParen arrow primary_type { $$ = $5; }
    | lParen function_type_param_types rParen arrow primary_type { $$ = $5; }
    | namespaced_identifier template_arguments_opt
    | pointer_type
    ;

pointer_type:
      mulOp unique_shared_opt primary_type { $$ = $3; }
    ;

function_type_param_types:
      type comma function_type_param_types_tail
    ;

function_type_param_types_tail:
      /* empty */ { $$ = nullptr; }
    | type
    | type comma function_type_param_types_tail
    ;

unique_shared_opt:
      /* empty */
    | uniqueKw
    | sharedKw
    ;

literal:
      boolean_literal
    | intLit
      {
        llvm::APInt intVal;
        if ($1.getLexeme().getAsInteger(0, intVal)) {
          error("Invalid integer literal");
          YYERROR;
        }
        $$ = CREATE_NODE<LiteralExpr>(intVal,
          CREATE_TYPE<IntTy>(IntTy::Signed, intVal.getBitWidth()), LOC($1));
        std::cerr << "Parsed int literal: " << $1.getLexeme().str() << std::endl;
      }
    | floatLit
      { 
        double doubleVal = std::stod($1.getLexeme().str());
        $$ = CREATE_NODE<LiteralExpr>(llvm::APFloat(doubleVal),
          CREATE_TYPE<FloatTy>(FloatTy::DOUBLE), LOC($1));
        std::cerr << "Parsed double literal: " << doubleVal << std::endl;
      }
    | stringLit
      {
        $$ = CREATE_NODE<LiteralExpr>(
          $1.getLexeme(),
          CREATE_TYPE<StaticArrayTy>(CREATE_TYPE<CharTy>(), $1.getLexeme().size()),
          LOC($1)
        );
        std::cerr << "Parsed string literal: " << $1.getLexeme().str() << std::endl;
      }
    ;

boolean_literal:
    | trueKw
      {
        $$ = CREATE_NODE<LiteralExpr>(true, CREATE_TYPE<BoolTy>(), LOC($1));
      }
    | falseKw
      {
        $$ = CREATE_NODE<LiteralExpr>(false, CREATE_TYPE<BoolTy>(), LOC($1));
      }
    ;

namespaced_identifier:
      ident ns_id_list_tail
      {
        std::string fullName = $1.getLexeme().str() + $2;
        $$ = CREATE_NODE<RefExpr>(LOC($1), fullName);
        std::cerr << "Parsed identifier reference: " << fullName << std::endl;
      }
    ;

ns_id_list_tail:
      /* empty */
        { $$ = ""; }
    | coloncolon ident ns_id_list_tail
      {
        $$ = "::" + $2.getLexeme().str() + $3;
      }
    ;

%%

void glu::BisonParser::error(const std::string& msg) {
    std::cerr << "Error: " << msg << std::endl;
}
