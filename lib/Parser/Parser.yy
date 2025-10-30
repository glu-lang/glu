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
        std::vector<glu::ast::ImportSelector> selectors;
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
%type <std::vector<llvm::StringRef>> identifier_sequence
%type <std::vector<glu::ast::ImportSelector>> import_item_list_opt import_item_list
%type <llvm::StringRef> single_import_item
%type <glu::ast::ImportSelector> import_item

%type <ExprBase *> expression expression_opt initializer_opt
%type <ExprBase *> boolean_literal cast_expression conditional_expression logical_or_expression logical_and_expression equality_expression relational_expression additive_expression multiplicative_expression shift_expression unary_expression postfix_expression primary_expression literal
%type <ExprBase *> postfix_expr_stmt primary_expr_stmt

%type <ExprBase *> namespaced_identifier
%type <llvm::SmallVector<ExprBase *>> argument_list argument_list_opt
%type <std::vector<llvm::StringRef>> identifier_list

%type <TypeBase *> type type_opt array_type primary_type pointer_type function_return_type
%type <std::vector<TypeBase *>> function_type_param_types

%type <DeclBase *> type_declaration struct_declaration enum_declaration typealias_declaration function_declaration varlet_decl var_decl let_decl global_varlet_decl

%type <StmtBase *> statement expression_stmt assignment_or_call_stmt varlet_stmt return_stmt if_stmt while_stmt for_stmt break_stmt continue_stmt

%type <StmtBase *> else_opt
%type <CompoundStmt *> block function_body
%type <llvm::SmallVector<StmtBase *>> statement_list
%type <glu::Token> equality_operator relational_operator additive_operator multiplicative_operator shift_operator unary_operator overloadables

%type <llvm::SmallVector<FieldDecl*>> struct_body struct_field_list_opt struct_field_list
%type <FieldDecl*> struct_field

%type <std::vector<FieldDecl*>> enum_body enum_variant_list_opt enum_variant_list
%type <FieldDecl*> enum_variant

%type <ParamDecl*> parameter
%type <std::vector<ParamDecl*>> parameter_list parameter_list_opt function_params

%type <llvm::SmallVector<Attribute *>> attributes attribute

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
%token <glu::Token> nullKw 28 "null"

%token <glu::Token> lParen 29 "("
%token <glu::Token> rParen 30 ")"
%token <glu::Token> lBrace 31 "{"
%token <glu::Token> rBrace 32 "}"
%token <glu::Token> lBracket 33 "["
%token <glu::Token> rBracket 34 "]"
%token <glu::Token> dot 35 "."
%token <glu::Token> comma 36 ","
%token <glu::Token> colon 37 ":"
%token <glu::Token> semi 38 ";"
%token <glu::Token> arrow 39 "->"
%token <glu::Token> equal 40 "="
%token <glu::Token> backslash 41 "\\"
%token <glu::Token> question 42 "?"
%token <glu::Token> at 43 "@"
%token <glu::Token> coloncolon 44 "::"
%token <glu::Token> coloncolonLt 45 "::<"

%token <glu::Token> plusOp 46 "+"
%token <glu::Token> subOp 47 "-"
%token <glu::Token> mulOp 48 "*"
%token <glu::Token> divOp 49 "/"
%token <glu::Token> modOp 50 "%"
%token <glu::Token> eqOp 51 "=="
%token <glu::Token> neOp 52 "!="
%token <glu::Token> ltOp 53 "<"
%token <glu::Token> leOp 54 "<="
%token <glu::Token> gtOp 55 ">"
%token <glu::Token> geOp 56 ">="
%token <glu::Token> andOp 57 "&&"
%token <glu::Token> orOp 58 "||"
%token <glu::Token> bitAndOp 59 "&"
%token <glu::Token> bitOrOp 60 "|"
%token <glu::Token> bitXorOp 61 "^"
%token <glu::Token> bitLShiftOp 62 "<<"
%token <glu::Token> bitRShiftOp 63 ">>"
%token <glu::Token> rangeOp 64 "..."
%token <glu::Token> exclusiveRangeOp 65 "..<"
%token <glu::Token> notOp 66 "!"
%token <glu::Token> complOp 67 "~"
%token <glu::Token> derefOp 68 ".*"

%token <glu::Token> intLit 69 "int"
%token <glu::Token> floatLit 70 "float"
%token <glu::Token> stringLit 71 "string"

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
        *module = CREATE_NODE<ModuleDecl>(fileLoc, $1, &ctx);
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
    | type_declaration
    | function_declaration
    | global_varlet_decl
    ;

attributes:
      %empty
      {
        $$ = llvm::SmallVector<Attribute *>();
      }
    | attributes attribute
      {
        $$ = $1;
        $$.insert($$.end(), $2.begin(), $2.end());
      }
    ;

attribute:
      at ident
      {
        ast::AttributeKind kind =
            ast::Attribute::getAttributeKindFromLexeme($2.getLexeme());
        if (kind == ast::AttributeKind::InvalidKind) {
            diagnostics.warning(
                LOC(scanner.getPrevToken()),
                "Ignoring unknown attribute: @" + $2.getLexeme().str()
            );
        } else {
            $$.push_back(CREATE_NODE<Attribute>(kind, LOC($1), nullptr));
        }
      }
    | at ident lParen expression rParen
      {
        ast::AttributeKind kind =
            ast::Attribute::getAttributeKindFromLexeme($2.getLexeme());
        if (kind == ast::AttributeKind::InvalidKind) {
            diagnostics.warning(
                LOC(scanner.getPrevToken()),
                "Ignoring unknown attribute: @" + $2.getLexeme().str()
            );
        } else {
            $$.push_back(CREATE_NODE<Attribute>(kind, LOC($1), $4));
        }
      }
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
        std::vector<glu::ast::ImportSelector> sels;

        for (auto &s : $4.components)
            comps.push_back(llvm::StringRef(s));

        for (auto &s : $4.selectors)
            sels.push_back(s);
        ip.components = llvm::ArrayRef<llvm::StringRef>(comps);
        ip.selectors  = llvm::ArrayRef<glu::ast::ImportSelector>(sels);
        auto attrList = CREATE_NODE<AttributeList>($1, LOC($3));

        $$ = CREATE_NODE<ImportDecl>(LOC($3), nullptr, ip, $2, attrList);
      }
    ;

import_path:
      single_import_item
      {
        $$ = NamespaceSemantic { {}, {glu::ast::ImportSelector($1)} };
      }
    | single_import_item asKw ident
      {
        $$ = NamespaceSemantic { {}, {glu::ast::ImportSelector($1, $3.getLexeme())} };
      }
    | identifier_sequence coloncolon single_import_item
      {
        $$ = NamespaceSemantic { $1, {glu::ast::ImportSelector($3)} };
      }
    | identifier_sequence coloncolon single_import_item asKw ident
      {
        $$ = NamespaceSemantic { $1, {glu::ast::ImportSelector($3, $5.getLexeme())} };
      }
    | identifier_sequence coloncolon lBrace import_item_list_opt rBrace
      {
        $$ = NamespaceSemantic { $1, $4 };
      }
    ;

single_import_item:
      ident
      {
        $$ = $1.getLexeme();
      }
    | mulOp
      {
        $$ = "@all";
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

import_item:
      ident
      {
        $$ = glu::ast::ImportSelector($1.getLexeme());
      }
    | ident asKw ident
      {
        $$ = glu::ast::ImportSelector($1.getLexeme(), $3.getLexeme());
      }
    | overloadables
      {
        $$ = glu::ast::ImportSelector($1.getLexeme());
      }
    | overloadables asKw ident
      {
        $$ = glu::ast::ImportSelector($1.getLexeme(), $3.getLexeme());
      }
    ;

import_item_list_opt:
      %empty
      {
        $$ = std::vector<glu::ast::ImportSelector>();
      }
    | import_item_list
    | import_item_list comma
      { $$ = $1; }
    ;

import_item_list:
      import_item
      {
        $$ = std::vector<glu::ast::ImportSelector>{$1};
      }
    | import_item_list comma import_item
      {
        $$ = $1;
        $$.push_back($3);
      }
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
        auto attrList = CREATE_NODE<AttributeList>($1, LOC($3));
        $$ = CREATE_NODE<StructDecl>(ctx, LOC($3), nullptr, $4.getLexeme(), $6, $2, attrList);
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
      attributes visibility_opt ident colon type initializer_opt
      {
        auto attrList = CREATE_NODE<AttributeList>($1, LOC($3));
        $$ = CREATE_NODE<FieldDecl>(LOC($3), $3.getLexeme(), $5, $6, attrList, $2);
      }
    ;

enum_declaration:
      attributes visibility_opt enumKw ident type_opt enum_body
      {
        auto attrList = CREATE_NODE<AttributeList>($1, LOC($3));
        $$ = CREATE_NODE<EnumDecl>(ctx, LOC($3), nullptr, $4.getLexeme(), $6, $2, attrList);
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
      attributes visibility_opt ident initializer_opt
      {
        auto attrList = CREATE_NODE<AttributeList>($1, LOC($3));
        $$ = CREATE_NODE<FieldDecl>(LOC($3), $3.getLexeme(), nullptr, $4, attrList, $2);
      }
    ;

typealias_declaration:
      attributes visibility_opt typealiasKw ident template_definition_opt equal type semi
      {
        auto attrList = CREATE_NODE<AttributeList>($1, LOC($3));
        $$ = CREATE_NODE<TypeAliasDecl>(ctx, LOC($4), nullptr, $4.getLexeme(), $7, $2, attrList);
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
    | lBracket rBracket
    { $$ = $1; }
    | ident
    ;

function_declaration:
      attributes visibility_opt funcKw overloadables template_definition_opt function_params function_return_type function_body
      {
        std::vector<TypeBase *> paramsTy;

        std::transform($6.begin(), $6.end(), std::back_inserter(paramsTy), [](const ParamDecl* p) { return p->getType(); });

        if ($4.getLexeme() == "main") {
          $1.push_back(CREATE_NODE<Attribute>(ast::AttributeKind::NoManglingKind, LOC($3)));
        }

        auto attList = CREATE_NODE<AttributeList>($1, LOC($3));
        auto funcTy = CREATE_TYPE<FunctionTy>(
          paramsTy,
          $7,
          attList->hasAttribute(ast::AttributeKind::CVariadicKind)
        );
        $$ = CREATE_NODE<FunctionDecl>(LOC($3), nullptr, $4.getLexeme(), funcTy,
          $6, $8, $2, attList);
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
      attributes ident colon type initializer_opt
      {
        auto attrList = CREATE_NODE<AttributeList>($1, LOC($2));
        $$ = CREATE_NODE<ParamDecl>(LOC($2), $2.getLexeme(), $4, $5, attrList);
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
    | varlet_stmt
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

var_decl:
      varKw ident type_opt initializer_opt semi
      {
        $$ = CREATE_NODE<VarDecl>(LOC($2), $2.getLexeme(), $3, $4);
      }
    ;

let_decl:
      letKw ident type_opt equal expression semi
      {
        $$ = CREATE_NODE<LetDecl>(LOC($2), $2.getLexeme(), $3, $5);
      }
    ;

varlet_decl:
      var_decl
    | let_decl
    ;

varlet_stmt:
      attributes varlet_decl
      {
        $2->setAttributes(CREATE_NODE<AttributeList>($1, $2->getLocation()));
        $$ = CREATE_NODE<DeclStmt>($2->getLocation(), $2);
      }
    ;

global_varlet_decl:
      attributes visibility_opt varlet_decl
      {
        $3->setAttributes(CREATE_NODE<AttributeList>($1, $3->getLocation()));
        $3->setVisibility($2);
        $$ = $3;
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
    | elseKw if_stmt
      {
        // Else-if chain
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

/* Level 1: expression (top level) */
expression:
      conditional_expression
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
    | bitOrOp
    | bitXorOp
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
    | bitAndOp
    ;

/* Level 8: multiplication/division/modulo */
multiplicative_expression:
      multiplicative_expression multiplicative_operator shift_expression
      {
        auto *ref = CREATE_NODE<RefExpr>(LOC($2), NamespaceIdentifier::fromOp($2));
        $$ = CREATE_NODE<BinaryOpExpr>(LOC($2), $1, ref, $3);
      }
    | shift_expression
    ;

shift_operator:
      bitLShiftOp
    | bitRShiftOp
    ;

/* Level 9: bitwise shifts */
shift_expression:
      shift_expression shift_operator cast_expression
      {
        auto *ref = CREATE_NODE<RefExpr>(LOC($2), NamespaceIdentifier::fromOp($2));
        $$ = CREATE_NODE<BinaryOpExpr>(LOC($2), $1, ref, $3);
      }
    | cast_expression
    ;

/* Level 9.5: cast expression */
cast_expression:
      unary_expression
    | cast_expression asKw type %prec asKw
      {
        $$ = CREATE_NODE<CastExpr>(LOC($2), $1, $3);
      }
    ;

unary_operator:
      plusOp
    | subOp
    | notOp
    | complOp
    | bitAndOp
    ;

/* Level 10: unary expressions */
unary_expression:
      unary_operator unary_expression %prec PREFIX_UNARY
      {
        auto *ref = CREATE_NODE<RefExpr>(LOC($1), NamespaceIdentifier::fromOp($1));
        $$ = CREATE_NODE<UnaryOpExpr>(LOC($1), $2, ref);
      }
    | postfix_expression
    ;

/* Level 11: postfix (function call, subscript, field access) */
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

/* Level 12: primary expressions */
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
    | lParen rParen arrow primary_type { $$ = CREATE_TYPE<FunctionTy>(std::vector<TypeBase *>(), $4, false); }
    | lParen type rParen arrow primary_type { $$ = CREATE_TYPE<FunctionTy>(std::vector<TypeBase *>{$2}, $5, false); }
    | lParen function_type_param_types rParen arrow primary_type { $$ = CREATE_TYPE<FunctionTy>($2, $5, false); }
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
      type comma type
      {
        $$ = std::vector<TypeBase *>();
        $$.push_back($1);
        $$.push_back($3);
      }
    | function_type_param_types comma type
      {
        $1.push_back($3);
        $$ = $1;
      }
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
          $1.getData(),
          CREATE_TYPE<TypeVariableTy>(),
          LOC($1)
        );
      }
    | nullKw
      {
        $$ = CREATE_NODE<LiteralExpr>(nullptr, CREATE_TYPE<NullTy>(), LOC($1));
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
