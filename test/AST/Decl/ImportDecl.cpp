#include "AST/Decls.hpp"
#include "Basic/SourceLocation.hpp"

#include <llvm/Support/Casting.h>

#include <gtest/gtest.h>

using namespace glu::ast;
using namespace glu::types;

TEST(ImportDecl, ImportDeclTest)
{
    llvm::BumpPtrAllocator allocator;
    glu::SourceLocation loc(42);

    auto decl = ImportDecl::create(
        allocator, loc, nullptr, ImportPath({ { "std", "io" }, { "println" } })
    );

    ASSERT_EQ(decl->getImportPath().components.size(), 2);
    ASSERT_EQ(decl->getImportPath().selectors.size(), 1);
    ASSERT_EQ(decl->getImportPath().components[0], "std");
    ASSERT_EQ(decl->getImportPath().selectors[0], "println");
}
