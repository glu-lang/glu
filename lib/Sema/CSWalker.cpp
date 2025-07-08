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

        llvm::StringRef opName = node->getOperator()->getIdentifier();
        bool success = resolveOverloadSet(
            opName, node,
            /*isExplicit=*/false,
            [&](glu::ast::FunctionDecl *fnDecl) -> Constraint * {
                auto *fnTy = fnDecl->getType();
                return Constraint::createConversion(
                    _cs.getAllocator(), expectedFunctionTy, fnTy, node
                );
            }
        );

        if (!success) {
            _diagManager.error(
                node->getLocation(),
                llvm::Twine("use of undeclared operator '") + opName + "'"
            );
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
        auto *resultType = node->getType();

        auto constraint = Constraint::createMember(
            _cs.getAllocator(), ConstraintKind::ValueMember, baseType,
            resultType, node, node
        );
        _cs.addConstraint(constraint);
    }

    void postVisitCallExpr(glu::ast::CallExpr *node)
    {
        auto *refExpr = llvm::dyn_cast<glu::ast::RefExpr>(node->getCallee());
        if (!refExpr) {
            handlePointerCall(node);
            return;
        }

        llvm::StringRef functionName = refExpr->getIdentifier();

        llvm::SmallVector<glu::types::TypeBase *, 4> argTypes;
        for (auto *arg : node->getArgs()) {
            argTypes.push_back(arg->getType());
        }

        auto &arena = node->getModule()->getContext()->getTypesMemoryArena();
        auto *expectedFnTy
            = arena.create<glu::types::FunctionTy>(argTypes, node->getType());

        bool success = resolveOverloadSet(
            functionName, node,
            /*isExplicit=*/true,
            [&](glu::ast::FunctionDecl *fnDecl) -> Constraint * {
                return Constraint::createBindOverload(
                    _cs.getAllocator(), node->getType(), fnDecl, node
                );
            }
        );

        if (!success) {
            _diagManager.error(
                node->getLocation(),
                "No function overloads found for '" + functionName.str() + "'"
            );
        }
    }

private:
    /// @brief Resolve overload set for a given name, creating a disjunction
    /// constraint.
    /// @param name The identifier (operator or function name).
    /// @param anchor The AST node to attach the constraint and diagnostics to.
    /// @param isExplicit Whether this is an explicit call (e.g. CallExpr).
    /// @param constraintBuilder Strategy to generate constraints for a
    /// FunctionDecl.
    /// @returns True if any overload constraints were added; false if none
    /// matched.
    bool resolveOverloadSet(
        llvm::StringRef name, glu::ast::ExprBase *anchor, bool isExplicit,
        llvm::function_ref<Constraint *(glu::ast::FunctionDecl *)>
            constraintBuilder
    )
    {
        auto *item = _cs.getScopeTable()->lookupItem(name);
        llvm::ArrayRef<glu::ast::DeclBase *> decls
            = item ? item->decls : llvm::ArrayRef<glu::ast::DeclBase *>();

        llvm::SmallVector<Constraint *, 4> constraints;

        for (auto *decl : decls) {
            if (auto *fnDecl = llvm::dyn_cast<glu::ast::FunctionDecl>(decl)) {
                if (auto *constraint = constraintBuilder(fnDecl)) {
                    constraints.push_back(constraint);
                }
            }
        }

        if (constraints.empty()) {
            return false;
        }

        auto *disjunction = Constraint::createDisjunction(
            _cs.getAllocator(), constraints, anchor, isExplicit
        );
        _cs.addConstraint(disjunction);
        return true;
    }

    /// @brief Processes variable declarations that might contain function
    /// references
    void processVariableDeclarations(
        glu::ast::CallExpr *node, llvm::ArrayRef<glu::ast::DeclBase *> decls
    )
    {
        for (auto decl : decls) {
            if (auto varLetDecl = llvm::dyn_cast<glu::ast::VarLetDecl>(decl)) {
                auto declType = varLetDecl->getType();
                if (!declType)
                    continue;

                glu::types::FunctionTy *functionTy
                    = extractFunctionType(declType);

                if (functionTy) {
                    _cs.addConstraint(
                        Constraint::createEqual(
                            _cs.getAllocator(), node->getType(),
                            functionTy->getReturnType(), node
                        )
                    );
                    constrainArguments(node, functionTy);
                }
            }
        }
    }

    /// @brief Handles function calls through function pointers
    void handlePointerCall(glu::ast::CallExpr *node)
    {
        auto calleeType = node->getCallee()->getType();
        if (!calleeType)
            return;

        glu::types::FunctionTy *functionTy = extractFunctionType(calleeType);

        if (functionTy) {
            _cs.addConstraint(
                Constraint::createEqual(
                    _cs.getAllocator(), node->getType(),
                    functionTy->getReturnType(), node
                )
            );
            constrainArguments(node, functionTy);
        } else {
            // Fallback: create conversion constraint for unknown callee type
            _cs.addConstraint(
                Constraint::createConversion(
                    _cs.getAllocator(), calleeType, node->getType(), node
                )
            );
        }
    }

    /// @brief Extracts function type from pointer or direct function type
    glu::types::FunctionTy *extractFunctionType(glu::types::TypeBase *type)
    {
        if (auto funcTy = llvm::dyn_cast<glu::types::FunctionTy>(type)) {
            return funcTy;
        }

        if (auto ptrTy = llvm::dyn_cast<glu::types::PointerTy>(type)) {
            return llvm::dyn_cast<glu::types::FunctionTy>(ptrTy->getPointee());
        }

        return nullptr;
    }

    /// @brief Creates constraints for function call arguments
    void constrainArguments(
        glu::ast::CallExpr *node, glu::types::FunctionTy *functionTy
    )
    {
        auto args = node->getArgs();
        auto params = functionTy->getParameters();

        if (args.size() != params.size()) {
            _diagManager.error(
                node->getLocation(),
                "Function call has " + std::to_string(args.size())
                    + " arguments but function expects "
                    + std::to_string(params.size())
            );
            return;
        }

        for (size_t i = 0; i < args.size(); ++i) {
            auto argType = args[i]->getType();
            auto paramType = params[i];

            if (argType && paramType) {
                _cs.addConstraint(
                    Constraint::createConversion(
                        _cs.getAllocator(), argType, paramType, args[i]
                    )
                );
            }
        }
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
