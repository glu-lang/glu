#include "Decls.hpp"

namespace glu::ast {

ModuleDecl *ASTNode::getModule()
{
    ASTNode *node = this;
    while (node->getParent() != nullptr) {
        node = node->getParent();
    }
    return llvm::cast<ModuleDecl>(node);
}

llvm::StringRef TypeDecl::getName() const
{
    if (auto *typeAlias = llvm::dyn_cast<TypeAliasDecl>(this)) {
        return typeAlias->getName();
    } else if (auto *structDecl = llvm::dyn_cast<StructDecl>(this)) {
        return structDecl->getName();
    } else if (auto *enumDecl = llvm::dyn_cast<EnumDecl>(this)) {
        return enumDecl->getName();
    }
    llvm_unreachable("Invalid type declaration");
}

types::TypeBase *TypeDecl::getType() const
{
    if (auto *typeAlias = llvm::dyn_cast<TypeAliasDecl>(this)) {
        return typeAlias->getType();
    } else if (auto *structDecl = llvm::dyn_cast<StructDecl>(this)) {
        return structDecl->getType();
    } else if (auto *enumDecl = llvm::dyn_cast<EnumDecl>(this)) {
        return enumDecl->getType();
    }
    llvm_unreachable("Invalid type declaration");
}

} // namespace glu::ast
