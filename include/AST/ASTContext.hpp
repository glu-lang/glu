#ifndef GLU_AST_ASTCONTEXT_HPP
#define GLU_AST_ASTCONTEXT_HPP

#include "ASTNode.hpp"
#include "Basic/InternedMemoryArena.hpp"
#include "Basic/SourceManager.hpp"
#include "Types.hpp"

namespace glu::ast {

class ASTContext {
    TypedMemoryArena<ASTNode> _astMemoryArena;
    InternedMemoryArena<types::TypeBase> _typesMemoryArena;
    SourceManager *_sm;

public:
    explicit ASTContext(SourceManager *sm = nullptr) : _sm(sm) { }

    /// @brief Get the memory arena used by the AST context.
    /// @return The memory arena used by the AST context.
    TypedMemoryArena<ASTNode> &getASTMemoryArena() { return _astMemoryArena; }

    /// @brief Get the memory arena used by the types in the AST context.
    /// @return The memory arena used by the types in the AST context.
    InternedMemoryArena<types::TypeBase> &getTypesMemoryArena()
    {
        return _typesMemoryArena;
    }

    /// @brief Get the source manager used by the AST context.
    /// @return The source manager used by the AST context.
    SourceManager *getSourceManager() const { return _sm; }

    llvm::BumpPtrAllocator &getScannerAllocator()
    {
        return _astMemoryArena.getAllocator();
    }
};

}; // namespace glu::ast

#endif /* GLU_AST_ASTCONTEXT_HPP */
