#ifndef GLU_AST_DECL_TEMPLATEPARAMETERDECL_HPP
#define GLU_AST_DECL_TEMPLATEPARAMETERDECL_HPP

#include "ASTContext.hpp"
#include "ASTNodeMacros.hpp"
#include "Decl/TypeDecl.hpp"
#include "Types/TemplateParamTy.hpp"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Allocator.h>
#include <llvm/Support/TrailingObjects.h>

namespace glu::ast {

class TemplateParameterDecl final : public TypeDecl {
    llvm::StringRef _name;
    glu::types::TemplateParamTy *_type;

public:
    TemplateParameterDecl(
        ASTContext &context, SourceLocation location, llvm::StringRef name
    )
        : TypeDecl(
              NodeKind::TemplateParameterDeclKind, location, nullptr,
              Visibility::Private, nullptr
          )
        , _name(name)
        , _type(context.getTypesMemoryArena()
                    .create<glu::types::TemplateParamTy>(this))
    {
    }

    llvm::StringRef getName() const { return _name; }
    glu::types::TemplateParamTy *getType() const { return _type; }

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

#endif // GLU_AST_DECL_TEMPLATEPARAMETERDECL_HPP
