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

    glu::types::TypeBase *lift(llvm::Type *type) const;
};

} // namespace glu::irdec

#endif // GLU_IR_DEC_HPP
