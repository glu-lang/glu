#include "GILGen/GILGen.hpp"
#include "Parser.hpp"
#include "Scanner.hpp"
#include "Sema/Sema.hpp"

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
    glu::SourceManager sm;                                  \
    glu::ast::ASTContext context(&sm);                      \
    sm.loadBuffer(std::move(buf), "main.glu");              \
    glu::DiagnosticManager diag(sm);                        \
    glu::Parser parser(scanner, context, sm, diag /*, 1*/); \
    EXPECT_TRUE(parser.parse());                            \
    auto module = parser.getAST();                          \
    sema::constrainAST(module, diag);                       \
    EXPECT_FALSE(diag.hasErrors());                         \
    (void) module;

TEST(GILGenExpr, TernaryBasic)
{
    PREP_PARSER(
        "func test(cond: Bool, x: Int, y: Int) -> Int { return cond ? x : y; }"
    );

    ASSERT_EQ(module->getDecls().size(), 1u);
    auto decl = module->getDecls().front();
    auto *fn = llvm::cast<FunctionDecl>(decl);
    auto gilModule = std::make_unique<gil::Module>("test_module");
    GlobalContext globalCtx(gilModule.get());
    auto *f = generateFunction(gilModule.get(), fn, globalCtx);

    // Expect 5 basic blocks: entry + then + else + result + unreachable (after
    // explicit return)
    EXPECT_EQ(f->getBasicBlockCount(), 5u);

    // Collect block labels
    llvm::SmallVector<llvm::StringRef, 4> labels;
    for (auto &bb : f->getBasicBlocks()) {
        labels.push_back(bb.getLabel());
    }
    EXPECT_TRUE(llvm::is_contained(labels, "ternary.then"));
    EXPECT_TRUE(llvm::is_contained(labels, "ternary.else"));
    EXPECT_TRUE(llvm::is_contained(labels, "ternary.result"));
    // Unreachable block is auto-generated after an explicit return; label is
    // "unreachable"
    EXPECT_TRUE(llvm::is_contained(labels, "unreachable"));

    // Find result block and verify it has one argument
    gil::BasicBlock *resultBB = nullptr;
    for (auto &bb : f->getBasicBlocks()) {
        if (bb.getLabel() == "ternary.result") {
            resultBB = &bb;
            break;
        }
    }
    ASSERT_NE(resultBB, nullptr);
    EXPECT_EQ(resultBB->getArgumentCount(), 1u);

    // Check terminators kinds in the control flow blocks
    // Entry: CondBrInst, then/else: BrInst, result: ReturnInst
    // We only assert that result is Return and entry is CondBr to avoid being
    // overly brittle.
    auto &entryBB = f->getBasicBlocks().front();
    auto *entryTerm = entryBB.getTerminator();
    ASSERT_NE(entryTerm, nullptr);
    EXPECT_EQ(entryTerm->getKind(), gil::InstKind::CondBrInstKind);

    auto *resultTerm = resultBB->getTerminator();
    ASSERT_NE(resultTerm, nullptr);
    EXPECT_EQ(resultTerm->getKind(), gil::InstKind::ReturnInstKind);
}
