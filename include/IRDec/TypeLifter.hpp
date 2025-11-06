#ifndef GLU_IRDEC_TYPELIFTER_HPP
#define GLU_IRDEC_TYPELIFTER_HPP

#include "AST/ASTContext.hpp"
#include "AST/Decls.hpp"
#include "AST/Types.hpp"
#include <llvm/ADT/DenseMap.h>
#include <llvm/IR/Type.h>

namespace glu::irdec {

class TypeLifter {

    glu::ast::ASTContext &_context;
    llvm::DenseMap<llvm::Type const *, glu::ast::DeclBase *> _declBindings;

public:
    TypeLifter(glu::ast::ASTContext &context) : _context(context) { }
    ~TypeLifter() = default;

    /// @brief Lift an LLVM type to a GLU type
    /// @param type The LLVM type to lift
    /// @return The lifted GLU type, or nullptr if the type could not be lifted
    glu::types::TypeBase *lift(llvm::Type *type);

    /// @brief Get the declaration bindings map
    /// @return The declaration bindings map
    llvm::DenseMap<llvm::Type const *, glu::ast::DeclBase *> &
    getDeclBindings()
    {
        return _declBindings;
    }
};

} // namespace glu::irdec

#endif // GLU_IRDEC_TYPELIFTER_HPP
