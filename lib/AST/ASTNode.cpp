#include "Decls.hpp"

namespace glu::ast {

ModuleDecl *ASTNode::getModule()
{
    ASTNode *node = this;
    while (node->getParent() != nullptr) {
        node = node->getParent();
    }
    return llvm::cast<ModuleDecl>(node);
}
} // namespace glu::ast
