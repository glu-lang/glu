#include "GILGen/GILGen.hpp"
#include "Parser.hpp"
#include "Scanner.hpp"

#include <Module.hpp>
#include <gtest/gtest.h>

using namespace glu::gilgen;
using namespace glu::gil;
using namespace glu::ast;

#define PREP_PARSER(str)                                    \
    std::unique_ptr<llvm::MemoryBuffer> buf(                \
        llvm::MemoryBuffer::getMemBufferCopy(str)           \
    );                                                      \
    glu::Scanner scanner(buf.get());                        \
    glu::ast::ASTContext context;                           \
    glu::SourceManager sm;                                  \
    sm.loadBuffer(std::move(buf), "main.glu");              \
    glu::DiagnosticManager diag(sm);                        \
    glu::Parser parser(scanner, context, sm, diag /*, 1*/); \
    EXPECT_TRUE(parser.parse());                            \
    auto module = parser.getAST();

TEST(GILGenStmt, Empty)
{
    PREP_PARSER("func test() {}");

    assert(module->getDecls().size() == 1);
    auto decl = module->getDecls().front();
    auto fn = llvm::cast<FunctionDecl>(decl);
    llvm::BumpPtrAllocator arena;
    auto gilModule = gil::Module("filename");
    auto *f = GILGen().generateFunction(&gilModule, fn, arena);
    EXPECT_EQ(f->getName(), "test");
    EXPECT_EQ(f->getBasicBlockCount(), 1);
    auto bb = f->getEntryBlock();
    EXPECT_EQ(bb->getInstructionCount(), 1);
    EXPECT_EQ(
        bb->getInstructions().front().getKind(), InstKind::ReturnInstKind
    );
}
