#ifndef GLU_AST_STMT_FORSTMT_HPP
#define GLU_AST_STMT_FORSTMT_HPP

#include "ASTNode.hpp"

#include "ASTNode.hpp"
#include "Decl/ForBindingDecl.hpp"
#include "Stmt/CompoundStmt.hpp"

namespace glu::ast {

/// @class ForStmt
/// @brief Represents a for statement in the AST.
///
/// This class inherits from StmtBase and encapsulates the details of a for
/// statement.
class ForStmt : public StmtBase {
    /// @brief The binding of the for statement.
    ForBindingDecl *_binding;
    /// @brief The range of the for statement.
    ExprBase *_range;
    /// @brief The body of the for statement.
    CompoundStmt *_body;

public:
    /// @brief Constructor for the ForStmt class.
    /// @param location The source location of the compound statement.
    /// @param parent The parent AST node.
    /// @param binding The binding of the for statement.
    /// @param body The body of the for statement.
    /// @param range The range of the for statement.
    ForStmt(
        SourceLocation location, ForBindingDecl *binding, ExprBase *range,
        CompoundStmt *body
    )
        : StmtBase(NodeKind::ForStmtKind, location)
        , _binding(binding)
        , _range(range)
        , _body(body)
    {
        assert(binding && "Binding cannot be null.");
        assert(range && "Range cannot be null.");
        assert(body && "Body cannot be null.");
        binding->setParent(this);
        range->setParent(this);
        body->setParent(this);
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::ForStmtKind;
    }

    /// @brief Get the binding of the for statement.
    /// @return The binding of the for statement.
    ForBindingDecl *getBinding() { return _binding; }

    /// @brief Get the range of the for statement.
    /// @return The range of the for statement.
    ExprBase *getRange() { return _range; }

    /// @brief Get the body of the for statement.
    /// @return The body of the for statement.
    CompoundStmt *getBody() { return _body; }

    /// @brief Set the binding of the for statement.
    /// @param binding The new binding for the for statement.
    void setBinding(ForBindingDecl *binding)
    {
        _binding = binding;
        if (_binding)
            _binding->setParent(this);
    }

    /// @brief Set the range of the for statement.
    /// @param range The new range for the for statement.
    void setRange(ExprBase *range)
    {
        _range = range;
        if (_range)
            _range->setParent(this);
    }

    /// @brief Set the body of the for statement.
    /// @param body The new body for the for statement.
    void setBody(CompoundStmt *body)
    {
        _body = body;
        if (_body)
            _body->setParent(this);
    }
};
}

#endif // GLU_AST_STMT_FORSTMT_HPP
