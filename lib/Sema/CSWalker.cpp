#include "Sema/CSWalker.hpp"
#include "AST/ASTWalker.hpp"
#include "AST/Exprs.hpp"
#include "AST/Types.hpp"
#include "Sema/ConstraintSystem.hpp"

#include <variant>

namespace glu::sema {

class LocalCSWalker : public glu::ast::ASTWalker<LocalCSWalker, void> {
    ConstraintSystem _cs;
    glu::DiagnosticManager &_diagManager;

public:
    LocalCSWalker(ScopeTable *scope, glu::DiagnosticManager &diagManager)
        : _cs(scope), _diagManager(diagManager)
    {
    }

    ~LocalCSWalker() { _cs.solveConstraints(); }

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
        auto *expectedReturnType
            = _cs.getScopeTable()->getFunctionDecl()->getType()->getReturnType(
            );

        if (llvm::isa<glu::types::VoidTy>(expectedReturnType)
            && node->getReturnExpr() != nullptr) {
            _diagManager.error(
                node->getLocation(),
                "Function declared as void cannot return a value"
            );
            return;
        }

        auto *returnExpr = node->getReturnExpr();
        auto *returnType = returnExpr ? returnExpr->getType()
                                      : node->getModule()
                                            ->getContext()
                                            ->getTypesMemoryArena()
                                            .create<glu::types::VoidTy>();

        _cs.addConstraint(
            Constraint::createConversion(
                _cs.getAllocator(), returnType, expectedReturnType, node
            )
        );
    }

    /// @brief Visits an if statement and constrains its condition to boolean.
    void postVisitIfStmt(glu::ast::IfStmt *node)
    {
        auto *cond = node->getCondition();
        auto &memoryArena
            = cond->getModule()->getContext()->getTypesMemoryArena();
        auto *boolType = memoryArena.create<glu::types::BoolTy>();
        auto *condType = cond->getType();
        auto constraint = Constraint::createConversion(
            _cs.getAllocator(), condType, boolType, node
        );
        _cs.addConstraint(constraint);
    }

    /// @brief Visits a while statement and constrains its condition to boolean.
    void postVisitWhileStmt(glu::ast::WhileStmt *node)
    {
        auto *cond = node->getCondition();
        auto &memoryArena
            = cond->getModule()->getContext()->getTypesMemoryArena();
        auto *boolType = memoryArena.create<glu::types::BoolTy>();
        auto *condType = cond->getType();
        auto constraint = Constraint::createConversion(
            _cs.getAllocator(), condType, boolType, node
        );
        _cs.addConstraint(constraint);
    }

    /// @brief Visits a ternary conditional expression and generates type
    /// constraints.
    void postVisitTernaryConditionalExpr(glu::ast::TernaryConditionalExpr *node)
    {
        auto *conditionType = node->getCondition()->getType();
        auto *trueType = node->getTrueExpr()->getType();
        auto *falseType = node->getFalseExpr()->getType();
        auto *ternaryType = node->getType();

        auto &memoryArena
            = node->getModule()->getContext()->getTypesMemoryArena();
        auto boolType = memoryArena.create<glu::types::BoolTy>();

        _cs.addConstraint(
            Constraint::createConversion(
                _cs.getAllocator(), conditionType, boolType, node
            )
        );
        _cs.addConstraint(
            Constraint::createEqual(
                _cs.getAllocator(), trueType, ternaryType, node
            )
        );
        _cs.addConstraint(
            Constraint::createEqual(
                _cs.getAllocator(), falseType, ternaryType, node
            )
        );
    }

    void postVisitUnaryOpExpr(glu::ast::UnaryOpExpr *node)
    {
        auto *operandTy = node->getOperand()->getType();
        auto *resultTy = node->getType();

        auto &arena = node->getModule()->getContext()->getTypesMemoryArena();

        auto *expectedFunctionTy = arena.create<glu::types::FunctionTy>(
            llvm::ArrayRef<glu::types::TypeBase *> { operandTy }, resultTy
        );

        llvm::StringRef opName = node->getOperator().getLexeme();
        auto *item = _cs.getScopeTable()->lookupItem(opName);
        llvm::ArrayRef<ast::DeclBase *> overloadDecls
            = item ? item->decls : llvm::ArrayRef<ast::DeclBase *>();

        llvm::SmallVector<Constraint *, 4> constraints;

        for (auto *decl : overloadDecls) {
            if (auto *fnDecl = llvm::dyn_cast<ast::FunctionDecl>(decl)) {

                auto *fnTy = fnDecl->getType();

                constraints.push_back(
                    Constraint::createConversion(
                        _cs.getAllocator(), expectedFunctionTy, fnTy, node
                    )
                );
            }
        }

        if (constraints.empty()) {
            llvm::Twine errorMsg = llvm::Twine("use of undeclared operator '")
                + node->getOperator().getLexeme() + llvm::Twine("'");

            _diagManager.error(node->getLocation(), std::move(errorMsg));
        } else {
            auto *disjunction = Constraint::createDisjunction(
                _cs.getAllocator(), constraints, node, false
            );
            _cs.addConstraint(disjunction);
        }
    }

    void postVisitVarLetDecl(glu::ast::VarLetDecl *varLet)
    {
        auto *varType = varLet->getType();
        auto *value = varLet->getValue();
        if (!value)
            return;
        auto *valueType = value->getType();

        if (!varType) {
            auto *typeVar
                = _cs.getAllocator().Allocate<glu::types::TypeVariableTy>();
            varLet->setType(typeVar);
            _cs.addTypeVariable(typeVar);
            varType = typeVar;
        }
        auto constraint = Constraint::createConversion(
            _cs.getAllocator(), valueType, varType, varLet
        );
        _cs.addConstraint(constraint);
    }

    void postVisitStructMemberExpr(glu::ast::StructMemberExpr *node)
    {
        auto *base = node->getStructExpr();
        auto *baseType = base->getType();
        auto memberName = node->getMemberName();

        auto *structType = llvm::dyn_cast<glu::types::StructTy>(baseType);
        if (!structType) {
            _diagManager.error(
                node->getLocation(), "Base expression is not a struct type"
            );
            return;
        }

        std::optional<size_t> idx = structType->getFieldIndex(memberName);
        if (!idx) {
            _diagManager.error(
                node->getLocation(), "No such member in struct: " + memberName
            );
            return;
        }
        auto *memberType = structType->getField(*idx).type;

        auto constraint = Constraint::createEqual(
            _cs.getAllocator(), node->getType(), memberType, node
        );
        _cs.addConstraint(constraint);
    }
};

class GlobalCSWalker : public glu::ast::ASTWalker<GlobalCSWalker, void> {
    std::vector<ScopeTable> _scopeTable;
    glu::DiagnosticManager &_diagManager;

public:
    GlobalCSWalker(glu::DiagnosticManager &diagManager)
        : _diagManager(diagManager)
    {
    }

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
        LocalCSWalker(&_scopeTable.back(), _diagManager).visit(node);
    }
};

void constrainAST(
    glu::ast::ModuleDecl *module, glu::DiagnosticManager &diagManager
)
{
    GlobalCSWalker(diagManager).visit(module);
}

}
