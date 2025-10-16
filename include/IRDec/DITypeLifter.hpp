#ifndef IR_DEC_DI_TYPE_LIFTER_HPP
#define IR_DEC_DI_TYPE_LIFTER_HPP

#include "AST/ASTContext.hpp"
#include "AST/Types.hpp"
#include <llvm/IR/DebugInfoMetadata.h>

namespace glu::irdec {

class DITypeLifter {

    glu::ast::ASTContext &_context;

public:
    DITypeLifter(glu::ast::ASTContext &context) : _context(context) { }
    ~DITypeLifter() = default;

    /// @brief Lift a DIType to a GLU type
    /// @param diType The DIType to lift
    /// @return The lifted GLU type, or nullptr if the type could not be lifted
    glu::types::TypeBase *lift(llvm::DIType const *diType) const;

    /// @brief Handle a DIBasicType and lift it to a GLU type
    /// @param arena The memory arena to use for allocation
    /// @param diBasicType The DIBasicType to handle
    /// @return The lifted GLU type, or nullptr if the type could not be lifted
    glu::types::TypeBase *handleBasicType(
        glu::InternedMemoryArena<glu::types::TypeBase> &arena,
        llvm::DIBasicType const *diBasicType
    ) const;

    /// @brief Handle a DICompositeType and lift it to a GLU type
    /// @param typesArena The memory arena to use for type allocation
    /// @param astArena The memory arena to use for AST allocation
    /// @param diCompositeType The DICompositeType to handle
    /// @return The lifted GLU type, or nullptr if the type could not be lifted
    glu::types::TypeBase *handleComposedTypes(
        glu::InternedMemoryArena<glu::types::TypeBase> &typesArena,
        glu::TypedMemoryArena<glu::ast::ASTNode> &astArena,
        llvm::DICompositeType const *diCompositeType
    ) const;

    /// @brief Handle a DIDerivedType and lift it to a GLU type
    /// @param typesArena The memory arena to use for type allocation
    /// @param astArena The memory arena to use for AST allocation
    /// @param diDerivedType The DIDerivedType to handle
    /// @return The lifted GLU type, or nullptr if the type could not be lifted
    glu::types::TypeBase *handleDerivedType(
        glu::InternedMemoryArena<glu::types::TypeBase> &typesArena,
        glu::TypedMemoryArena<glu::ast::ASTNode> &astArena,
        llvm::DIDerivedType const *diDerivedType
    ) const;
};

} // namespace glu::irdec

#endif // IR_DEC_DI_TYPE_LIFTER_HPP
