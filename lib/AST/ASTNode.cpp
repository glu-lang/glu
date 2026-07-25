#include "ASTVisitor.hpp"
#include "Decls.hpp"

#include <llvm/Support/Path.h>
#include <regex>

namespace glu::ast {

ModuleDecl *ASTNode::getModule()
{
    ASTNode *node = this;
    while (node->getParent() != nullptr) {
        node = node->getParent();
    }
    return llvm::dyn_cast<ModuleDecl>(node);
}

llvm::StringRef TypeDecl::getName() const
{
    if (auto *typeAlias = llvm::dyn_cast<TypeAliasDecl>(this)) {
        return typeAlias->getName();
    } else if (auto *structDecl = llvm::dyn_cast<StructDecl>(this)) {
        return structDecl->getName();
    } else if (auto *enumDecl = llvm::dyn_cast<EnumDecl>(this)) {
        return enumDecl->getName();
    } else if (auto *templateParam
               = llvm::dyn_cast<TemplateParameterDecl>(this)) {
        return templateParam->getName();
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
    } else if (auto *templateParam
               = llvm::dyn_cast<TemplateParameterDecl>(this)) {
        return templateParam->getType();
    }
    llvm_unreachable("Invalid type declaration");
}

struct ManglingPathVisitor
    : public ASTVisitor<
          ManglingPathVisitor, llvm::SmallVector<llvm::StringRef, 4>> {

    using ManglingPath = llvm::SmallVector<llvm::StringRef, 4>;

    ManglingPath visitASTNode([[maybe_unused]] ASTNode *node)
    {
        // Fallback for non-declaration nodes
        // Mangling path is undefined
        return {};
    }

    ManglingPath visitModuleDecl(ModuleDecl *node)
    {
        auto moduleName = node->getImportName();
        llvm::SmallVector<llvm::StringRef, 4> path;
        for (auto component : llvm::make_range(
                 llvm::sys::path::begin(moduleName),
                 llvm::sys::path::end(moduleName)
             )) {
            path.push_back(component);
        }
        return path;
    }

    template <typename NamedDecl> ManglingPath visitNamedDecl(NamedDecl *node)
    {
        auto parentPath = this->visit(node->getParent());
        parentPath.push_back(node->getName());
        return parentPath;
    }

    ManglingPath visitNamespaceDecl(NamespaceDecl *node)
    {
        return visitNamedDecl(node);
    }

    ManglingPath visitFunctionDecl(FunctionDecl *node)
    {
        return visitNamedDecl(node);
    }

    ManglingPath visitVarLetDecl(VarLetDecl *node)
    {
        return visitNamedDecl(node);
    }

    ManglingPath visitTypeDecl(TypeDecl *node) { return visitNamedDecl(node); }
};

llvm::SmallVector<llvm::StringRef, 4> DeclBase::getManglingPath() const
{
    return ManglingPathVisitor().visit(const_cast<DeclBase *>(this));
}

std::string escapeIdentifier(llvm::StringRef name)
{
    static std::regex const identifierRegex("^[a-zA-Z][a-zA-Z0-9_]*$");
    static std::regex const backtickRegex("`");

    if (!std::regex_match(name.str(), identifierRegex)) {
        return "`" + std::regex_replace(name.str(), backtickRegex, "``") + "`";
    }
    return name.str();
}

} // namespace glu::ast
