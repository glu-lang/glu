#ifndef GLU_IR_DEC_HPP
#define GLU_IR_DEC_HPP

#include "AST/ASTContext.hpp"
#include "AST/Types.hpp"
#include <llvm/IR/Type.h>

namespace glu::irdec {

class TypeLifter {

    glu::ast::ASTContext &_context;

public:
    TypeLifter(glu::ast::ASTContext &context) : _context(context) { }
    ~TypeLifter() = default;

    /// @brief Lift an LLVM type to a GLU type
    /// @param type The LLVM type to lift
    /// @return The lifted GLU type, or nullptr if the type could not be lifted
    glu::types::TypeBase *lift(llvm::Type *type) const;

private:
    llvm::StringRef
    copyString(llvm::StringRef str, llvm::BumpPtrAllocator &alloc) const;
};

} // namespace glu::irdec

#endif // GLU_IR_DEC_HPP
