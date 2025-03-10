#ifndef TEST_ASTPRINTER_ASTPRINTER_HPP
#define TEST_ASTPRINTER_ASTPRINTER_HPP

#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "AST/Exprs.hpp"
#include "Basic/SourceManager.hpp"
#include "Basic/Tokens.hpp"
#include "Parser.hpp"
#include "Scanner.hpp"

#include <gtest/gtest.h>
#include <memory>

class ASTPrinterTest : public ::testing::Test {
protected:
    std::string out;
    glu::SourceManager sm;
    glu::ast::ASTContext ctx;
    llvm::raw_string_ostream os;
    glu::TypedMemoryArena<glu::ast::ASTNode> &ast;

    ASTPrinterTest() : os(out), ast(ctx.getASTMemoryArena()) { }
};

#define PREP_ASTPRINTER(str, file)                \
    sm.reset();                                   \
    std::unique_ptr<llvm::MemoryBuffer> buf(      \
        llvm::MemoryBuffer::getMemBufferCopy(str) \
    );                                            \
    sm.loadBuffer(std::move(buf), file);

#define PREP_MAIN_ASTPRINTER(str) PREP_ASTPRINTER("func main() {" str "}")

#endif // TEST_ASTPRINTER_ASTPRINTER_HPP
