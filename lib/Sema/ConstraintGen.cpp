#include "AST/ASTNode.hpp"
#include "AST/TypedASTWalker.hpp"
#include "Sema/Constraints.hpp"

namespace glu::sema {

class ConstraintGen
    : public ast::TypedASTWalker<
          ConstraintGen, Constraint *, Constraint *, Constraint *> { };

Constraint *genConstrains(ASTNode *parentModule)
{
    return ConstraintGen().visit(parentModule);
}

}
