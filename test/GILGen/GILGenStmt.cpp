#include "GILGen/GILGen.hpp"
#include "Instructions.hpp"
#include "Parser.hpp"
#include "Scanner.hpp"
#include "Sema/Sema.hpp"

#include <Module.hpp>
#include <gtest/gtest.h>

using namespace glu::gilgen;
using namespace glu::gil;
using namespace glu::ast;

#define PREP_PARSER(str)                                            \
    std::unique_ptr<llvm::MemoryBuffer> buf(                        \
        llvm::MemoryBuffer::getMemBufferCopy(str)                   \
    );                                                              \
    glu::SourceManager sm;                                          \
    glu::ast::ASTContext context(&sm);                              \
    glu::Scanner scanner(buf.get(), context.getScannerAllocator()); \
    sm.loadBuffer(std::move(buf), "main.glu");                      \
    glu::DiagnosticManager diag(sm);                                \
    glu::Parser parser(scanner, context, sm, diag /*, 1*/);         \
    EXPECT_TRUE(parser.parse());                                    \
    auto module = parser.getAST();                                  \
    sema::constrainAST(module, diag);                               \
    EXPECT_FALSE(diag.hasErrors());

TEST(GILGenStmt, Empty)
{
    PREP_PARSER("func test() {}");

    assert(module->getDecls().size() == 1);
    auto decl = module->getDecls().front();
    auto fn = llvm::cast<FunctionDecl>(decl);
    auto gilModule = std::make_unique<gil::Module>("test_module");
    GlobalContext globalCtx(gilModule.get());
    auto *f = generateFunction(gilModule.get(), fn, globalCtx);
    EXPECT_EQ(f->getName(), "test");
    EXPECT_EQ(f->getBasicBlockCount(), 1);
    auto bb = f->getEntryBlock();
    EXPECT_EQ(bb->getInstructionCount(), 1);
    EXPECT_EQ(
        bb->getInstructions().front().getKind(), InstKind::ReturnInstKind
    );
}

TEST(GILGenStmt, StructFieldPtrResultTypePointsToFieldType)
{
    PREP_PARSER(R"(
        struct Inner {
            value: Int
        }

        func copy(i: *Inner) -> Inner {
            var result: Inner;
            result.value = i.*.value;
            return result;
        }
    )");

    ASSERT_EQ(module->getDecls().size(), 2u);
    auto *fn = llvm::cast<FunctionDecl>(module->getDecls()[1]);
    auto gilModule = std::make_unique<gil::Module>("test_module");
    GlobalContext globalCtx(gilModule.get());
    auto *f = generateFunction(gilModule.get(), fn, globalCtx);

    auto *structFieldPtrInst = [&]() -> StructFieldPtrInst * {
        for (auto &bb : f->getBasicBlocks()) {
            for (auto &inst : bb.getInstructions()) {
                if (auto *fieldPtr = llvm::dyn_cast<StructFieldPtrInst>(&inst))
                    return fieldPtr;
            }
        }
        return nullptr;
    }();

    ASSERT_NE(structFieldPtrInst, nullptr);
    auto *resultType
        = llvm::dyn_cast<types::PointerTy>(structFieldPtrInst->getResultType());
    ASSERT_NE(resultType, nullptr);
    EXPECT_EQ(
        resultType->getPointee(), structFieldPtrInst->getMember().getType()
    );
}
