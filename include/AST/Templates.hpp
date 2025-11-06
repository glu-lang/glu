#ifndef GLU_AST_TEMPLATES_HPP
#define GLU_AST_TEMPLATES_HPP

#include "ASTNode.hpp"
#include "ASTNodeMacros.hpp"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Allocator.h>
#include <llvm/Support/TrailingObjects.h>

namespace glu::ast {

class TemplateParameterDecl final : public MetadataBase {
    llvm::StringRef _name;

public:
    TemplateParameterDecl(SourceLocation location, llvm::StringRef name)
        : MetadataBase(NodeKind::TemplateParameterDeclKind, location)
        , _name(name)
    {
    }

    static TemplateParameterDecl *create(
        llvm::BumpPtrAllocator &allocator, SourceLocation location,
        llvm::StringRef name
    )
    {
        void *mem = allocator.Allocate(
            sizeof(TemplateParameterDecl), alignof(TemplateParameterDecl)
        );
        return new (mem) TemplateParameterDecl(location, name);
    }

    llvm::StringRef getName() const { return _name; }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::TemplateParameterDeclKind;
    }
};

class TemplateParameterList final
    : public MetadataBase,
      private llvm::TrailingObjects<
          TemplateParameterList, TemplateParameterDecl *> {
    GLU_AST_GEN_CHILDREN_TRAILING_OBJECTS(
        TemplateParameterList, _numTemplateParameters, TemplateParameterDecl *,
        TemplateParameters
    )

    TemplateParameterList(
        llvm::ArrayRef<TemplateParameterDecl *> parameters,
        SourceLocation location
    )
        : MetadataBase(NodeKind::TemplateParameterListKind, location)
    {
        initTemplateParameters(parameters);
    }

public:
    static TemplateParameterList *create(
        llvm::BumpPtrAllocator &allocator,
        llvm::ArrayRef<TemplateParameterDecl *> parameters,
        SourceLocation location
    )
    {
        void *mem = allocator.Allocate(
            totalSizeToAlloc<TemplateParameterDecl *>(parameters.size()),
            alignof(TemplateParameterList)
        );
        return new (mem) TemplateParameterList(parameters, location);
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::TemplateParameterListKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_TEMPLATES_HPP
