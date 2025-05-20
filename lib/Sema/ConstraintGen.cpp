#include "AST/ASTNode.hpp"
#include "AST/TypedASTWalker.hpp"
#include "Sema/Constraint.hpp"
#include "Sema/ConstraintSystem.hpp"

namespace glu::sema {

class ConstraintGen
    : public ast::TypedASTWalker<
          ConstraintGen, Constraint *, Constraint *, Constraint *> {
    ConstraintSystem *cs;

public:
    ConstraintGen(ConstraintSystem *cs) : cs(cs) { }
};

Constraint *ConstraintSystem::genConstraints(glu::ast::DeclBase *parentModule)
{
    return ConstraintGen(this).visit(parentModule);
}

}
