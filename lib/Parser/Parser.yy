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
%parse-param { glu::DiagnosticManager &diagnostics }
%parse-param { glu::ast::ModuleDecl **module }
%lex-param { glu::Scanner &scanner }

%code requires {
    #include "AST/ASTContext.hpp"
    #include "AST/Decls.hpp"
    #include "AST/Types.hpp"
    #include "AST/Exprs.hpp"
    #include "AST/Stmts.hpp"
    #include "AST/Visibility.hpp"
    #include "Basic/SourceLocation.hpp"
    #include <vector>
    #include <string>

    struct NamespaceSemantic {
        std::vector<llvm::StringRef> components;
        std::vector<llvm::StringRef> selectors;
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

    inline unsigned getRadixFromLexeme(const llvm::StringRef &lexeme) {
        if (lexeme.starts_with("0x") || lexeme.starts_with("0X")) return 16;
        if (lexeme.starts_with("0b") || lexeme.starts_with("0B")) return 2;
        if (lexeme.starts_with("0o") || lexeme.starts_with("0O")) return 8;
        return 10;
    }
    inline llvm::StringRef stripRadixPrefix(const llvm::StringRef &lexeme, unsigned radix) {
        if (radix == 16) return lexeme.drop_front(2);
        if (radix == 2) return lexeme.drop_front(2);
        if (radix == 8) return lexeme.drop_front(2);
        return lexeme;
    }
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
%type <Visibility> visibility_opt

%type <NamespaceSemantic> import_path
%type <std::vector<llvm::StringRef>> identifier_sequence import_item_list_opt import_item_list
%type <llvm::StringRef> single_import_item

%type <ExprBase *> expression expression_opt initializer_opt
%type <ExprBase *> boolean_literal cast_expression conditional_expression logical_or_expression logical_and_expression equality_expression relational_expression additive_expression multiplicative_expression unary_expression postfix_expression primary_expression literal
%type <ExprBase *> postfix_expr_stmt primary_expr_stmt

%type <ExprBase *> namespaced_identifier
%type <llvm::SmallVector<ExprBase *>> argument_list argument_list_opt
%type <std::vector<llvm::StringRef>> identifier_list

%type <TypeBase *> type type_opt array_type primary_type pointer_type function_type_param_types function_type_param_types_tail function_return_type

%type <DeclBase *> type_declaration struct_declaration enum_declaration typealias_declaration function_declaration

%type <StmtBase *> statement expression_stmt assignment_or_call_stmt var_stmt let_stmt return_stmt if_stmt while_stmt for_stmt break_stmt continue_stmt

%type <CompoundStmt *> else_opt
%type <CompoundStmt *> block function_body
%type <llvm::SmallVector<StmtBase *>> statement_list
%type <glu::Token> equality_operator relational_operator additive_operator multiplicative_operator unary_operator single_import_item_token overloadables

%type <llvm::SmallVector<FieldDecl*>> struct_body struct_field_list_opt struct_field_list
%type <FieldDecl*> struct_field

%type <std::vector<FieldDecl*>> enum_body enum_variant_list_opt enum_variant_list
%type <FieldDecl*> enum_variant

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
%token <glu::Token> privateKw 15 "private"
%token <glu::Token> publicKw 16 "public"

%token <glu::Token> ifKw 17 "if"
%token <glu::Token> elseKw 18 "else"
%token <glu::Token> whileKw 19 "while"
%token <glu::Token> forKw 20 "for"
%token <glu::Token> returnKw 21 "return"
%token <glu::Token> breakKw 22 "break"
%token <glu::Token> continueKw 23 "continue"
%token <glu::Token> inKw 24 "in"

%token <glu::Token> trueKw 25 "true"
%token <glu::Token> falseKw 26 "false"
%token <glu::Token> asKw 27 "as"

%token <glu::Token> lParen 28 "("
%token <glu::Token> rParen 29 ")"
%token <glu::Token> lBrace 30 "{"
%token <glu::Token> rBrace 31 "}"
%token <glu::Token> lBracket 32 "["
%token <glu::Token> rBracket 33 "]"
%token <glu::Token> dot 34 "."
%token <glu::Token> comma 35 ","
%token <glu::Token> colon 36 ":"
%token <glu::Token> semi 37 ";"
%token <glu::Token> arrow 38 "->"
%token <glu::Token> equal 39 "="
%token <glu::Token> backslash 40 "\\"
%token <glu::Token> question 41 "?"
%token <glu::Token> at 42 "@"
%token <glu::Token> coloncolon 43 "::"
%token <glu::Token> coloncolonLt 44 "::<"

%token <glu::Token> plusOp 45 "+"
%token <glu::Token> subOp 46 "-"
%token <glu::Token> mulOp 47 "*"
%token <glu::Token> divOp 48 "/"
%token <glu::Token> modOp 49 "%"
%token <glu::Token> eqOp 50 "=="
%token <glu::Token> neOp 51 "!="
%token <glu::Token> ltOp 52 "<"
%token <glu::Token> leOp 53 "<="
%token <glu::Token> gtOp 54 ">"
%token <glu::Token> geOp 55 ">="
%token <glu::Token> andOp 56 "&&"
%token <glu::Token> orOp 57 "||"
%token <glu::Token> bitAndOp 58 "&"
%token <glu::Token> bitOrOp 59 "|"
%token <glu::Token> bitXorOp 60 "^"
%token <glu::Token> bitLShiftOp 61 "<<"
%token <glu::Token> bitRShiftOp 62 ">>"
%token <glu::Token> rangeOp 63 "..."
%token <glu::Token> exclusiveRangeOp 64 "..<"
%token <glu::Token> notOp 65 "!"
%token <glu::Token> complOp 66 "~"
%token <glu::Token> derefOp 67 ".*"


%token <glu::Token> intLit 68 "int"
%token <glu::Token> floatLit 69 "float"
%token <glu::Token> stringLit 70 "string"

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
        SourceLocation fileLoc = scanner.getFileStartLoc(sm);
        *module = CREATE_NODE<ModuleDecl>(fileLoc,
          sm.getBufferName(fileLoc), $1, &ctx);
      }
  ;

top_level_list:
      top_level
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
      {
        $$ = $1;
      }
    | type_declaration
      {
        $$ = $1;
      }
    | function_declaration
      {
        $$ = $1;
      }
    ;

attributes:
      %empty
    | attributes attribute
    ;

attribute:
      at ident
    ;

visibility_opt:
      %empty
      {
        $$ = Visibility::Private;
      }
    | privateKw
      {
        $$ = Visibility::Private;
      }
    | publicKw
      {
        $$ = Visibility::Public;
      }
    ;

import_declaration:
      attributes visibility_opt importKw import_path semi
      {
        ImportPath ip;
        std::vector<llvm::StringRef> comps;
        std::vector<llvm::StringRef> sels;

        for (auto &s : $4.components)
            comps.push_back(llvm::StringRef(s));
        for (auto &s : $4.selectors)
            sels.push_back(llvm::StringRef(s));
        ip.components = llvm::ArrayRef<llvm::StringRef>(comps);
        ip.selectors  = llvm::ArrayRef<llvm::StringRef>(sels);

        $$ = CREATE_NODE<ImportDecl>(LOC($3), nullptr, ip, $2);
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
    | overloadables
    ;

single_import_item:
      single_import_item_token
      {
        $$ = $1.getLexeme();
      }
    ;


identifier_sequence:
      ident
      {
        $$ = std::vector<llvm::StringRef>{ $1.getLexeme() };
      }
    | identifier_sequence coloncolon ident
      {
        $$ = $1;
        $$.push_back($3.getLexeme());
      }
    ;

import_item_list_opt:
      %empty { }
    | import_item_list
    ;

import_item_list:
      single_import_item
      {
        $$ = std::vector<llvm::StringRef>{$1};
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
      attributes visibility_opt structKw ident template_definition_opt struct_body
      {
        $$ = CREATE_NODE<StructDecl>(ctx, LOC($3), nullptr, $4.getLexeme(), $6, $2);
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
      %empty { }
    | struct_field_list
      {
        $$ = std::move($1);
      }
    ;

struct_field_list:
      struct_field
      {
        llvm::SmallVector<FieldDecl*> vec;
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
      visibility_opt ident colon type initializer_opt
      {
        $$ = CREATE_NODE<FieldDecl>(LOC($2), $2.getLexeme(), $4, $5, $1);
      }
    ;

enum_declaration:
      attributes visibility_opt enumKw ident colon type enum_body
      {
        $$ = CREATE_NODE<EnumDecl>(ctx, LOC($3), nullptr, $4.getLexeme(), $7, $2);
      }
    ;

enum_body:
      lBrace enum_variant_list_opt rBrace
      {
        $$ = std::move($2);
      }
    ;

enum_variant_list_opt:
      %empty { }
    | enum_variant_list { $$ = $1; }
    ;

enum_variant_list:
      enum_variant
      {
        $$ = std::vector<glu::ast::FieldDecl*>();
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
      visibility_opt ident initializer_opt
      {
        $$ = CREATE_NODE<FieldDecl>(LOC($2), $2.getLexeme(), nullptr, $3, $1);
      }
    ;

typealias_declaration:
      attributes visibility_opt typealiasKw ident template_definition_opt equal type semi
      {
        $$ = CREATE_NODE<TypeAliasDecl>(ctx, LOC($4), nullptr, $4.getLexeme(), $7, $2);
      }
    ;

/*--------------------------------*/
/* Function declarations          */
/*--------------------------------*/

overloadables:
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
    | bitAndOp
    | bitOrOp
    | bitXorOp
    | bitLShiftOp
    | bitRShiftOp
    | rangeOp
    | exclusiveRangeOp
    | notOp
    | complOp
    | derefOp
    | ident

function_declaration:
      attributes visibility_opt funcKw overloadables template_definition_opt function_params function_return_type function_body
      {
        std::vector<TypeBase *> paramsTy;

        std::transform($6.begin(), $6.end(), std::back_inserter(paramsTy), [](const ParamDecl* p) { return p->getType(); });

        auto funcTy = CREATE_TYPE<FunctionTy>(
          paramsTy,
          $7
        );

        $$ = CREATE_NODE<FunctionDecl>(LOC($3), nullptr, $4.getLexeme(), funcTy, $6, $8, $2);
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
      %empty { }
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
      %empty { }
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
      }
    | postfix_expr_stmt function_template_arguments lParen argument_list_opt rParen %prec POSTFIX
      {
        auto c = CREATE_NODE<CallExpr>(LOC($3), $1, $4);

        $$ = CREATE_NODE<ExpressionStmt>(c->getLocation(), c);
      }
    | postfix_expr_stmt
      {
        $$ = CREATE_NODE<ExpressionStmt>($1->getLocation(), $1);
      }
    ;

postfix_expr_stmt:
      primary_expr_stmt
    | postfix_expr_stmt lBracket expression rBracket %prec POSTFIX
      {
        auto *ref = CREATE_NODE<RefExpr>(LOC($2), NamespaceIdentifier::fromOp($2));
        $$ = CREATE_NODE<BinaryOpExpr>(LOC($2), $1, ref, $3);
      }
    | postfix_expr_stmt dot ident %prec POSTFIX
      {
        $$ = CREATE_NODE<StructMemberExpr>(LOC($2), $1, $3.getLexeme());
      }
    | postfix_expr_stmt derefOp %prec POSTFIX
      {
        auto *ref = CREATE_NODE<RefExpr>(LOC($2), NamespaceIdentifier::fromOp($2));
        $$ = CREATE_NODE<UnaryOpExpr>(LOC($2), $1, ref);
      }
    ;

primary_expr_stmt:
      namespaced_identifier
        | lParen expression rParen { $$ = $2; }
    ;

function_template_arguments:
      %empty
    | coloncolonLt type_list gtOp
    ;

var_stmt:
      varKw ident type_opt initializer_opt semi
      {
        auto varDecl = CREATE_NODE<VarDecl>(LOC($2), $2.getLexeme(), $3, $4);
        $$ = CREATE_NODE<DeclStmt>(LOC($2), varDecl);
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
        auto letDecl = CREATE_NODE<LetDecl>(LOC($2), $2.getLexeme(), $3, $5);
        $$ = CREATE_NODE<DeclStmt>(LOC($2), letDecl);
      }
    ;

return_stmt:
      returnKw expression_opt semi
      {
        $$ = CREATE_NODE<ReturnStmt>(LOC($1), $2);
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
      }
    ;

else_opt:
      %empty { $$ = nullptr; }
    | elseKw block
      {
        $$ = $2;
      }
    ;

while_stmt:
      whileKw expression block
      {
        $$ = CREATE_NODE<WhileStmt>(LOC($1), $2, $3);
      }
    ;

for_stmt:
      forKw ident inKw expression block
      {
        auto binding = CREATE_NODE<ForBindingDecl>(
          LOC($2),
          $2.getLexeme(),
          CREATE_TYPE<TypeVariableTy>());

        $$ = CREATE_NODE<ForStmt>(LOC($1), binding, $4, $5);
      }
    ;

break_stmt:
      breakKw semi
      {
        $$ = CREATE_NODE<BreakStmt>(LOC($1));
      }
    ;

continue_stmt:
      continueKw semi
      {
        $$ = CREATE_NODE<ContinueStmt>(LOC($1));
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
      }
    ;

/* Level 2: conditional expression (ternary) */
conditional_expression:
      logical_or_expression
    | logical_or_expression question expression colon conditional_expression %prec TERNARY
      {
        $$ = CREATE_NODE<TernaryConditionalExpr>(LOC($2), $1, $3, $5);
      }
    ;

/* Level 3: logical OR */
logical_or_expression:
      logical_or_expression orOp logical_and_expression
      {
        auto *ref = CREATE_NODE<RefExpr>(LOC($2), NamespaceIdentifier::fromOp($2));
        $$ = CREATE_NODE<BinaryOpExpr>(LOC($2), $1, ref, $3);
      }
    | logical_and_expression
    ;

/* Level 4: logical AND */
logical_and_expression:
      logical_and_expression andOp equality_expression
      {
        auto *ref = CREATE_NODE<RefExpr>(LOC($2), NamespaceIdentifier::fromOp($2));
        $$ = CREATE_NODE<BinaryOpExpr>(LOC($2), $1, ref, $3);
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
        auto *ref = CREATE_NODE<RefExpr>(LOC($2), NamespaceIdentifier::fromOp($2));
        $$ = CREATE_NODE<BinaryOpExpr>(LOC($2), $1, ref, $3);
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
        auto *ref = CREATE_NODE<RefExpr>(LOC($2), NamespaceIdentifier::fromOp($2));
        $$ = CREATE_NODE<BinaryOpExpr>(LOC($2), $1, ref, $3);
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
        auto *ref = CREATE_NODE<RefExpr>(LOC($2), NamespaceIdentifier::fromOp($2));
        $$ = CREATE_NODE<BinaryOpExpr>(LOC($2), $1, ref, $3);
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
        auto *ref = CREATE_NODE<RefExpr>(LOC($2), NamespaceIdentifier::fromOp($2));
        $$ = CREATE_NODE<BinaryOpExpr>(LOC($2), $1, ref, $3);
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
        auto *ref = CREATE_NODE<RefExpr>(LOC($1), NamespaceIdentifier::fromOp($1));
        $$ = CREATE_NODE<UnaryOpExpr>(LOC($1), $2, ref);
      }
    | postfix_expression
    ;

/* Level 10: postfix (function call, subscript, field access) */
postfix_expression:
      primary_expression
    | postfix_expression function_template_arguments lParen argument_list_opt rParen %prec POSTFIX
      {
        // TODO: implement function template arguments
        $$ = CREATE_NODE<CallExpr>(LOC($3), $1, $4);
      }
    | postfix_expression lBracket expression rBracket %prec POSTFIX
      {
        auto *ref = CREATE_NODE<RefExpr>(LOC($2), NamespaceIdentifier::fromOp($2));
        $$ = CREATE_NODE<BinaryOpExpr>(LOC($2), $1, ref, $3);
      }
    | postfix_expression dot ident %prec POSTFIX
      {
        $$ = CREATE_NODE<StructMemberExpr>(LOC($2), $1, $3.getLexeme());
      }
    | postfix_expression derefOp %prec POSTFIX
      {
        auto *ref = CREATE_NODE<RefExpr>(LOC($2), NamespaceIdentifier::fromOp($2));
        $$ = CREATE_NODE<UnaryOpExpr>(LOC($2), $1, ref);
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
          $$ = CREATE_NODE<StructInitializerExpr>(LOC($1), $2);
      }
    ;

/*--------------------------------*/
/* Argument list                  */
/*--------------------------------*/

argument_list_opt:
      %empty { }
    | argument_list
    ;

argument_list:
      expression
      {
        $$.push_back($1);
      }
    | argument_list comma expression
      {
        $$ = std::move($1);
        $$.push_back($3);
      }
    | argument_list comma
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
        $$ = CREATE_TYPE<UnresolvedNameTy>(static_cast<RefExpr *>($1)->getIdentifiers(), static_cast<RefExpr *>($1)->getLocation());
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

literal:
      boolean_literal
    | intLit
      {
        unsigned radix = getRadixFromLexeme($1.getLexeme());
        llvm::StringRef value = stripRadixPrefix($1.getLexeme(), radix);
        $$ = CREATE_NODE<LiteralExpr>(
          llvm::APInt(
            llvm::APInt::getSufficientBitsNeeded(value, radix),
            value,
            radix
          ),
          CREATE_TYPE<TypeVariableTy>(),
          LOC($1)
        );
      }
    | floatLit
      {
        $$ = CREATE_NODE<LiteralExpr>(
          llvm::APFloat(std::stod($1.getLexeme().str())),
          CREATE_TYPE<TypeVariableTy>(),
          LOC($1)
        );
      }
    | stringLit
      {
        $$ = CREATE_NODE<LiteralExpr>(
          llvm::StringRef($1.getLexeme()).drop_front(1).drop_back(1), // Remove quotes
          CREATE_TYPE<TypeVariableTy>(),
          LOC($1)
        );
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
      }
    ;

%%

void glu::BisonParser::error(const std::string& msg) {
    diagnostics.error(LOC(scanner.getPrevToken()), msg);
}
