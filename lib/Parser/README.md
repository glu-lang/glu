# Parser Documentation

## Token Generation for Bison Parser

When adding new tokens to the Glu language, you need to regenerate the token list for the parser. This process helps ensure all tokens are properly defined in the Bison grammar file.

### Generating Token List

To generate the token list after adding a new token, run the following command from the `lib/Parser` directory:

```bash
gcc -E BisonTokens.cpp -I../../include
```

This command will output a formatted list of tokens suitable for use in the Bison parser file (`Parser.yy`). The output format looks like this:

```bison
% token<glu::Token> eof 0 "eof"
% token<glu::Token> ident 1 "ident"

% token<glu::Token> unterminatedBlockCommentError 2 "unterminatedBlockComment"
% token<glu::Token> unterminatedStringLitError 3 "unterminatedStringLit"
% token<glu::Token> unknownCharError 4 "unknownChar"

% token<glu::Token> uniqueKw 5 "unique"
% token<glu::Token> sharedKw 6 "shared"

% token<glu::Token> structKw 7 "struct"
% token<glu::Token> unionKw 8 "union"
% token<glu::Token> enumKw 9 "enum"
% token<glu::Token> typealiasKw 10 "typealias"
% token<glu::Token> funcKw 11 "func"
% token<glu::Token> letKw 12 "let"
% token<glu::Token> varKw 13 "var"
% token<glu::Token> importKw 14 "import"
% token<glu::Token> privateKw 15 "private"
% token<glu::Token> publicKw 16 "public"

% token<glu::Token> ifKw 17 "if"
% token<glu::Token> elseKw 18 "else"
% token<glu::Token> whileKw 19 "while"
% token<glu::Token> forKw 20 "for"
% token<glu::Token> returnKw 21 "return"
% token<glu::Token> breakKw 22 "break"
% token<glu::Token> continueKw 23 "continue"
% token<glu::Token> inKw 24 "in"

% token<glu::Token> trueKw 25 "true"
% token<glu::Token> falseKw 26 "false"
% token<glu::Token> asKw 27 "as"

% token<glu::Token> lParen 28 "("
% token<glu::Token> rParen 29 ")"
% token<glu::Token> lBrace 30 "{"
% token<glu::Token> rBrace 31 "}"
% token<glu::Token> lBracket 32 "["
% token<glu::Token> rBracket 33 "]"
% token<glu::Token> dot 34 "."
% token<glu::Token> comma 35 ","
% token<glu::Token> colon 36 ":"
% token<glu::Token> semi 37 ";"
% token<glu::Token> arrow 38 "->"
% token<glu::Token> equal 39 "="
% token<glu::Token> backslash 40 "\\"
% token<glu::Token> question 41 "?"
% token<glu::Token> at 42 "@"
% token<glu::Token> coloncolon 43 "::"
% token<glu::Token> coloncolonLt 44 "::<"

% token<glu::Token> plusOp 45 "+"
% token<glu::Token> subOp 46 "-"
% token<glu::Token> mulOp 47 "*"
% token<glu::Token> divOp 48 "/"
% token<glu::Token> modOp 49 "%"
% token<glu::Token> eqOp 50 "=="
% token<glu::Token> neOp 51 "!="
% token<glu::Token> ltOp 52 "<"
% token<glu::Token> leOp 53 "<="
% token<glu::Token> gtOp 54 ">"
% token<glu::Token> geOp 55 ">="
% token<glu::Token> andOp 56 "&&"
% token<glu::Token> orOp 57 "||"
% token<glu::Token> bitAndOp 58 "&"
% token<glu::Token> bitOrOp 59 "|"
% token<glu::Token> bitXorOp 60 "^"
% token<glu::Token> bitLShiftOp 61 "<<"
% token<glu::Token> bitRShiftOp 62 ">>"
% token<glu::Token> rangeOp 63 "..."
% token<glu::Token> exclusiveRangeOp 64 "..<"
% token<glu::Token> notOp 65 "!"
% token<glu::Token> complOp 66 "~"
% token<glu::Token> derefOp 67 ".*"

% token<glu::Token> intLit 68 "int"
% token<glu::Token> floatLit 69 "float"
% token<glu::Token> stringLit 70 "string"
```

### Usage

1. After adding a new token to the token definitions, run the generation command
2. Copy the relevant output and format it appropriately for your `Parser.yy` file
3. This saves time compared to manually copying all token definitions

This automated approach ensures consistency and reduces the chance of errors when updating the parser with new tokens.
