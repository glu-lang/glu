#ifndef GLU_GILGEN_SCOPE_HPP
#define GLU_GILGEN_SCOPE_HPP

#include "Decls.hpp"
#include "Stmts.hpp"

#include "BasicBlock.hpp"

namespace glu::gilgen {

/// Represents a scope, in the GILGen sense. A scope is a compound statement in
/// the AST. This class is used to keep track of the current scope for GIL code
/// generation.
class Scope {
    /// The block statement this scope represents.
    ast::CompoundStmt *_block;
    /// The parent scope of this scope, or nullptr if this is the function
    /// scope.
    Scope *_parent;

    /// The basic block that represents the exit point of this scope, if this is
    /// a loop.
    gil::BasicBlock *_breakDestination = nullptr;
    /// The basic block that represents the continue point of this scope, if
    /// this is a loop.
    gil::BasicBlock *_continueDestination = nullptr;

    // TODO: variables are handled here

public:
    /// @brief Creates a scope for a function.
    /// @param functionScope the AST function declaration.
    Scope(ast::FunctionDecl *functionScope)
        : _block(functionScope->getBody()), _parent(nullptr)
    {
    }

    /// Returns true if this scope represents a function.
    bool isFunctionScope() const
    {
        return llvm::isa<ast::FunctionDecl>(_block->getParent());
    }

    /// Returns true if this scope represents a loop (while or for).
    bool isLoopScope() const
    {
        return llvm::isa<ast::WhileStmt>(_block->getParent())
            || llvm::isa<ast::ForStmt>(_block->getParent());
    }

    /// Returns true if this scope represents a conditional (if/else).
    bool isIfScope() const
    {
        return llvm::isa<ast::CompoundStmt>(_block->getParent());
    }

    /// Returns true if this scope is unnamed (simple {} block).
    bool isUnnamedScope() const
    {
        return llvm::isa<ast::CompoundStmt>(_block->getParent());
    }
};

} // namespace glu::gilgen

#endif // GLU_GILGEN_SCOPE_HPP
