#include "AST/ASTNode.hpp"
#include "AST/ASTWalker.hpp"
#include "AST/Exprs.hpp"
#include "AST/Types.hpp"
#include "Sema/ConstraintSystem.hpp"

#include <variant>

namespace glu::sema {

class LocalCSWalker : public glu::ast::ASTWalker<LocalCSWalker, void> {
    ConstraintSystem _cs;

public:
    LocalCSWalker(ScopeTable *scope) : _cs(scope) { }
    ~LocalCSWalker() { _cs.resolveConstraints(); }

    /// @brief preVisit method for all expressions to ensure they have a type
    /// before visiting them.
    void preVisitExprBase(glu::ast::ExprBase *node)
    {
        // Ensure the expression has a type before visiting it
        if (!node->getType()) {
            // If the type is not set, we can create a type variable for it
            auto typeVar
                = _cs.getAllocator().Allocate<glu::types::TypeVariableTy>();
            node->setType(typeVar);
            _cs.addTypeVariable(typeVar);
        }
    }

    /// @brief Visits a cast expression and emit an equality or defaultable
    /// constraint
    void postVisitCastExpr(glu::ast::CastExpr *node)
    {
        glu::ast::ExprBase *inner = node->getCastedExpr();

        auto *fromType = inner->getType();
        auto *toType = node->getDestType();

        auto constraint = Constraint::createCheckedCast(
            _cs.getAllocator(), fromType, toType, node
        );

        _cs.addConstraint(constraint);
    }

    /// @brief Visits a variable declaration and generates type constraints.
    void postVisitAssignStmt(glu::ast::AssignStmt *node)
    {
        glu::ast::ExprBase *lhs = node->getExprLeft();
        glu::ast::ExprBase *rhs = node->getExprRight();

        glu::types::TypeBase *leftType = lhs->getType();
        glu::types::TypeBase *rightType = rhs->getType();

        auto constraint = Constraint::createConversion(
            _cs.getAllocator(), rightType, leftType, node
        );
        _cs.addConstraint(constraint);
    }

    /// @brief Visits a literal expression and generates type constraints.
    void postVisitLiteralExpr(glu::ast::LiteralExpr *node)
    {
        auto &memoryArena
            = node->getModule()->getContext()->getTypesMemoryArena();
        auto value = node->getValue();

        // Get the current type of the literal expression
        glu::types::TypeBase *nodeType = node->getType();

        // Create appropriate default type based on literal type
        glu::types::TypeBase *defaultType = nullptr;

        std::visit(
            [&](auto &&arg) {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, llvm::APInt>) {
                    // Integer literal - default to signed 32-bit integer
                    defaultType = memoryArena.create<glu::types::IntTy>(
                        glu::types::IntTy(glu::types::IntTy::Signed, 32)
                    );
                } else if constexpr (std::is_same_v<T, llvm::APFloat>) {
                    // Float literal - default to 64-bit double
                    defaultType = memoryArena.create<glu::types::FloatTy>(
                        glu::types::FloatTy(glu::types::FloatTy::DOUBLE)
                    );
                } else if constexpr (std::is_same_v<T, bool>) {
                    // Boolean literal
                    defaultType = memoryArena.create<glu::types::BoolTy>();
                } else if constexpr (std::is_same_v<T, llvm::StringRef>) {
                    // String literal - create pointer to char type
                    auto charType = memoryArena.create<glu::types::CharTy>();
                    defaultType
                        = memoryArena.create<glu::types::PointerTy>(charType);
                }
            },
            value
        );

        // If we created a default type, create a defaultable constraint
        if (defaultType) {
            // Create a defaultable constraint between the node's type and the
            // default type
            auto constraint = glu::sema::Constraint::createDefaultable(
                _cs.getAllocator(), nodeType, defaultType, node
            );

            // Add the constraint to the constraint system
            _cs.addConstraint(constraint);
        }
    }

    /// @brief Visits a return statement and generates type constraints.
    void postVisitReturnStmt(glu::ast::ReturnStmt *node)
    {
        auto *returnType = node->getReturnExpr()->getType();
        auto *expectedReturnType
            = _cs.getScopeTable()->getFunctionDecl()->getType()->getReturnType(
            );

        // If the function is void, we don't need to constrain the return
        // type
        if (expectedReturnType == nullptr
            || llvm::isa<glu::types::VoidTy>(expectedReturnType)) {
            return;
        }

        _cs.addConstraint(Constraint::createConversion(
            _cs.getAllocator(), returnType, expectedReturnType, node
        ));
    }
};

class GlobalCSWalker : public glu::ast::ASTWalker<GlobalCSWalker, void> {
    std::vector<ScopeTable> _scopeTable;

public:
    GlobalCSWalker() { }

    void preVisitModuleDecl(glu::ast::ModuleDecl *node)
    {
        _scopeTable.push_back(ScopeTable(node));
    }

    void postVisitModuleDecl([[maybe_unused]] glu::ast::ModuleDecl *node)
    {
        _scopeTable.pop_back();
    }

    void preVisitFunctionDecl(glu::ast::FunctionDecl *node)
    {
        _scopeTable.push_back(ScopeTable(&_scopeTable.back(), node));
    }

    void postVisitFunctionDecl([[maybe_unused]] glu::ast::FunctionDecl *node)
    {
        _scopeTable.pop_back();
    }

    void preVisitCompoundStmt(glu::ast::CompoundStmt *node)
    {
        _scopeTable.push_back(ScopeTable(&_scopeTable.back(), node));
    }

    void postVisitCompoundStmt([[maybe_unused]] glu::ast::CompoundStmt *node)
    {
        assert(_scopeTable.back().getParent() && "Cannot pop global scope");
        _scopeTable.pop_back();
    }

    void preVisitForStmt(glu::ast::ForStmt *node)
    {
        _scopeTable.push_back(ScopeTable(&_scopeTable.back(), node));
    }

    void postVisitForStmt([[maybe_unused]] glu::ast::ForStmt *node)
    {
        _scopeTable.pop_back();
    }

    void postVisitVarLetDecl(glu::ast::VarLetDecl *node)
    {
        _scopeTable.back().insertItem(node->getName(), node);
    }

    void preVisitStmt(glu::ast::StmtBase *node)
    {
        LocalCSWalker(&_scopeTable.back()).visit(node);
    }
};

void constrainAST(glu::ast::ModuleDecl *module)
{
    GlobalCSWalker().visit(module);
}

}
