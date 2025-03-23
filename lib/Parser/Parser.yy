%{
#include "Parser.hpp"
#include "Basic/Tokens.hpp"
#include <iostream>
#include <algorithm>
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
%parse-param { glu::ast::ModuleDecl **module }
%lex-param { glu::Scanner &scanner }

%code requires {
    #include "AST/ASTContext.hpp"
    #include "AST/Decls.hpp"
    #include "AST/Types.hpp"
    #include "AST/Exprs.hpp"
    #include "AST/Stmts.hpp"
    #include "Basic/SourceLocation.hpp"
    #include <vector>
    #include <string>

    struct NamespaceSemantic {
        std::vector<std::string> components;
        std::vector<std::string> selectors;
    };

    namespace glu {
        class Scanner;
        class SourceManager;
        class Token;
    }

    using namespace glu;
    using namespace glu::ast;
    using namespace glu::types;

    #define LOC(tok) (sm.getSourceLocFromToken(tok))
    #define LOC_NAME(name) (sm.getSourceLocFromStringRef(name))
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

%type <DeclBase *> top_level
%type <llvm::SmallVector<DeclBase *>> top_level_list
%type <DeclBase *> import_declaration

%type <NamespaceSemantic> import_path
%type <std::vector<std::string>> identifier_sequence import_item_list_opt import_item_list
%type <std::string> single_import_item

%type <ExprBase *> expression expression_opt initializer_opt function_call
%type <ExprBase *> boolean_literal cast_expression conditional_expression logical_or_expression logical_and_expression equality_expression relational_expression additive_expression multiplicative_expression unary_expression postfix_expression primary_expression literal
%type <ExprBase *> postfix_expr_stmt primary_expr_stmt

%type <ExprBase *> namespaced_identifier
%type <std::vector<llvm::StringRef>> identifier_list

%type <TypeBase *> type type_opt array_type primary_type pointer_type function_type_param_types function_type_param_types_tail function_return_type

%type <DeclBase *> var_stmt let_stmt type_declaration struct_declaration enum_declaration typealias_declaration function_declaration

%type <StmtBase *> statement expression_stmt assignment_or_call_stmt return_stmt if_stmt while_stmt for_stmt break_stmt continue_stmt

%type <CompoundStmt *> else_opt
%type <CompoundStmt *> block function_body
%type <llvm::SmallVector<StmtBase *>> statement_list
%type <glu::Token> equality_operator relational_operator additive_operator multiplicative_operator unary_operator variable_literal single_import_item_token

%type <llvm::SmallVector<Field>> struct_body struct_field_list_opt struct_field_list
%type <Field> struct_field

%type <std::vector<Case>> enum_body enum_variant_list_opt enum_variant_list
%type <Case> enum_variant

%type <ParamDecl*> parameter
%type <std::vector<ParamDecl*>> parameter_list parameter_list_opt function_params

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
      {
        *module = CREATE_NODE<ModuleDecl>(SourceLocation(1),
          sm.getBufferName(SourceLocation(1)), $1);
      }
  ;

top_level_list:
      %empty
        { $$ = llvm::SmallVector<DeclBase*>(); }
    | top_level
      { 
        llvm::SmallVector<DeclBase*> vec;
        vec.push_back($1);
        $$ = vec;
      }
    | top_level_list top_level
      {
        $$ = $1;
        $$.push_back($2);
      }
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
      %empty
    | attributes attribute
    ;

attribute:
      at ident
    ;

import_declaration:
      importKw import_path semi
      {
        ImportPath ip;
        std::vector<llvm::StringRef> comps;
        std::vector<llvm::StringRef> sels;

        for (auto &s : $2.components)
            comps.push_back(llvm::StringRef(s));
        for (auto &s : $2.selectors)
            sels.push_back(llvm::StringRef(s));
        ip.components = llvm::ArrayRef<llvm::StringRef>(comps);
        ip.selectors  = llvm::ArrayRef<llvm::StringRef>(sels);

        $$ = CREATE_NODE<ImportDecl>(LOC($1), nullptr, ip);
        std::cerr << "Parsed import declaration : " << ip.toString() << std::endl;
      }
    ;

import_path:
      single_import_item
      {
        $$ = NamespaceSemantic { {}, {$1} };
      }
    | identifier_sequence coloncolon single_import_item
      {
        $$ = NamespaceSemantic { $1, {$3} }; 
      }
    | identifier_sequence coloncolon lBrace import_item_list_opt rBrace
      {
        $$ = NamespaceSemantic { $1, $4 };
      }
    ;

single_import_item_token:
      ident
    | mulOp
    ;

single_import_item:
      single_import_item_token
      {
        $$ = $1.getLexeme().str();
      }
    ;


identifier_sequence:
      ident
      {
        $$ = std::vector<std::string>{ $1.getLexeme().str() };
      }
    | identifier_sequence coloncolon ident
      {
        $$ = $1;
        $$.push_back($3.getLexeme().str());
      }
    ;

import_item_list_opt:
      %empty
        { $$ = std::vector<std::string>{}; }
    | import_item_list
    ;

import_item_list:
      single_import_item
      {
        $$ = std::vector<std::string>{$1};
      }
    | import_item_list comma single_import_item
      {
        $$ = $1;
        $$.push_back($3);
      }
    | import_item_list comma
        { $$ = $1; }
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
        $$ = CREATE_NODE<StructDecl>(ctx, LOC($2), nullptr, $3.getLexeme(), $5);
        std::cerr << "Parsed struct declaration" << std::endl;
      }
    ;

template_definition_opt:
      %empty
    | template_definition
    ;

template_definition:
      ltOp template_parameter_list_opt gtOp
    ;

template_parameter_list_opt:
      %empty
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
      lBrace struct_field_list_opt rBrace { $$ = std::move($2); }
    ;

struct_field_list_opt:
      %empty
      {
        $$ = llvm::SmallVector<Field>();
      }
    | struct_field_list
      {
        $$ = std::move($1);
      }
    ;

struct_field_list:
      struct_field
      {
        llvm::SmallVector<Field> vec;
        vec.push_back($1);
        $$ = std::move(vec);
      }
    | struct_field_list comma struct_field
      {
        $$ = std::move($1);
        $$.push_back($3);
      }
    | struct_field_list comma
      {
        $$ = std::move($1);
      }
    ;

struct_field:
      ident colon type initializer_opt
      {
        // TODO: implement initializer option
        $$ = Field { $1.getLexeme(), $3 };
      }
    ;

enum_declaration:
      attributes enumKw ident colon type enum_body
      {
        $$ = CREATE_NODE<EnumDecl>(ctx, LOC($2), nullptr, $3.getLexeme(), $6);
        std::cerr << "Parsed enum declaration" << std::endl;
      }
    ;

enum_body:
      lBrace enum_variant_list_opt rBrace
      {
        std::vector<glu::types::Case> cases = std::move($2);
        for (unsigned i = 0; i < cases.size(); ++i)
          cases[i].value = llvm::APInt(32, i);
        $$ = std::move(cases);
      }
    ;

enum_variant_list_opt:
      %empty { $$ = std::vector<glu::types::Case>(); }
    | enum_variant_list { $$ = $1; }
    ;

enum_variant_list:
      enum_variant
      { 
        $$ = std::vector<glu::types::Case>(); 
        $$.push_back($1); 
      }
    | enum_variant_list comma enum_variant
      { 
        $$ = $1; 
        $$.push_back($3); 
      }
    | enum_variant_list comma
      {
        $$ = $1;
      }
    ;

enum_variant:
      ident initializer_opt
      {
        // TODO: handle initializer
        llvm::APInt caseValue(32, 0); // default value
        $$ = glu::types::Case { $1.getLexeme(), caseValue };
      }
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
      attributes funcKw ident template_definition_opt function_params function_return_type function_body
      {
        std::vector<TypeBase *> paramsTy;

        std::transform($5.begin(), $5.end(), std::back_inserter(paramsTy), [](const ParamDecl* p) { return p->getType(); });

        auto funcTy = CREATE_TYPE<FunctionTy>(
          paramsTy,
          $6
        );

        $$ = CREATE_NODE<FunctionDecl>(LOC($2), nullptr, $3.getLexeme(), funcTy, $5, $7);
        std::cerr << "Parsed function declaration" << std::endl;
      }
    ;

function_return_type:
     %empty
     {
       $$ = CREATE_TYPE<VoidTy>();
     }
    | arrow type
    {
      $$ = $2;
    }
    ;

function_params:
     lParen parameter_list_opt rParen
     {
       $$ = std::move($2);
     }
    ;

parameter_list_opt:
      %empty
      {
        $$ = std::vector<ParamDecl*>();
      }
    | parameter_list
    ;

parameter_list:
      parameter
      {
        $$ = std::vector<ParamDecl*>();
        $$.push_back($1);
      }
    | parameter_list comma parameter
      {
        $1.push_back($3);
        $$ = $1;
      }
    | parameter_list comma
      {
        $$ = $1;
      }
    ;

parameter:
      ident colon type initializer_opt
      {
        $$ = CREATE_NODE<ParamDecl>(LOC($1), $1.getLexeme(), $3, $4);
        std::cerr << "Parsed function declaration" << std::endl;
      }
    ;

function_body:
      block
    | semi
      {
        $$ = nullptr;
      }
    ;

block:
      lBrace statement_list rBrace
      {
        $$ = CREATE_NODE<CompoundStmt>(LOC($1), $2);
      }
  ;

statement_list:
      %empty
      {
        $$ = llvm::SmallVector<StmtBase*>();
      }
    | statement_list statement
      {
        $$ = $1;
        if ($2)
          $$.push_back($2);
      }
  ;

statement:
      block { $$ = $1; }
    | expression_stmt semi
    | var_stmt
    | let_stmt
    | return_stmt
    | if_stmt
    | while_stmt
    | for_stmt
    | break_stmt
    | continue_stmt
    | semi { $$ = nullptr; }
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
        { $$ = nullptr; }
    ;

function_template_arguments:
      %empty
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
      %empty
        { $$ = nullptr; }
    | colon type
        { $$ = $2; }
    ;

initializer_opt:
      %empty
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
      %empty
        { $$ = nullptr; }
    | expression
    ;

if_stmt:
      ifKw expression block else_opt
      {
        $$ = CREATE_NODE<IfStmt>(LOC($1), $2, $3, $4);
        std::cerr << "Parsed if statement" << std::endl;
      }
    ;

else_opt:
      %empty { $$ = nullptr; }
    | elseKw block
      {
        $$ = $2;
        std::cerr << "Parsed else statement" << std::endl;
      }
    ;

while_stmt:
      whileKw expression block
      {
        $$ = CREATE_NODE<WhileStmt>(LOC($1), $2, $3);
        std::cerr << "Parsed while statement" << std::endl;
      }
    ;

for_stmt:
      forKw ident inKw expression block
      {
        auto binding = CREATE_NODE<ForBindingDecl>(
          LOC($2),
          $2.getLexeme().str(),
          CREATE_TYPE<TypeVariableTy>());

        $$ = CREATE_NODE<ForStmt>(LOC($1), binding, $4, $5);
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
        $$ = CREATE_NODE<TernaryConditionalExpr>(LOC($2), $1, $3, $5);
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
      additive_expression relational_operator additive_expression
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
      %empty
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
      %empty
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
      {
        std::string name = static_cast<RefExpr *>($1)->getIdentifier().str();
        $$ = CREATE_TYPE<UnresolvedNameTy>(name);
        std::cerr << "Parsed type: " << name << std::endl;
      }
    | pointer_type
    ;

pointer_type:
      mulOp unique_shared_opt primary_type
      {
        // TODO: implement unique/shared pointer types
        $$ = CREATE_TYPE<PointerTy>($3);
      }
    ;

function_type_param_types:
      type comma function_type_param_types_tail
    ;

function_type_param_types_tail:
      %empty { $$ = nullptr; }
    | type
    | type comma function_type_param_types_tail
    ;

unique_shared_opt:
      %empty
    | uniqueKw
    | sharedKw
    ;

variable_literal:
      intLit
    | floatLit
    | stringLit

literal:
      boolean_literal
    | variable_literal
      {
        $$ = CREATE_NODE<LiteralExpr>(
          $1.getLexeme(),
          CREATE_TYPE<TypeVariableTy>(),
          LOC($1)
        );
        std::cerr << "Parsed type variable literal: " << $1.getLexeme().str() << std::endl;
      }
    ;

boolean_literal:
      trueKw
      {
        $$ = CREATE_NODE<LiteralExpr>(true, CREATE_TYPE<BoolTy>(), LOC($1));
      }
    | falseKw
      {
        $$ = CREATE_NODE<LiteralExpr>(false, CREATE_TYPE<BoolTy>(), LOC($1));
      }
    ;

identifier_list:
      ident
      {
        $$ = std::vector<llvm::StringRef>{ $1.getLexeme() };
      }
    | identifier_list coloncolon ident
      {
        $$ = $1;
        $$.push_back($3.getLexeme());
      }
    ;

namespaced_identifier:
      identifier_list
      {
        NamespaceIdentifier ni;
        std::vector<llvm::StringRef> comps;

        // Don't push the last one, it's the identifier
        for (size_t i = 0; i < $1.size() - 1; ++i) {
          comps.push_back($1[i]);
        }

        ni.identifier = llvm::StringRef($1.back()); // last one is the identifier
        ni.components = llvm::ArrayRef<llvm::StringRef>(comps);

        $$ = CREATE_NODE<RefExpr>(LOC_NAME($1[0]), ni);
        std::cerr << "Parsed namespaced identifier: " << ni.toString() << std::endl;
      }
    ;

%%

void glu::BisonParser::error(const std::string& msg) {
    std::cerr << "Error: " << msg << std::endl;
}
