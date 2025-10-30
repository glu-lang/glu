#include "AST/ASTWalker.hpp"
#include "Sema.hpp"

#include "ConstraintSystem.hpp"

namespace glu::sema {

/// @brief Walks the AST to generate and solve constraints for expressions
/// within a statement.
class LocalCSWalker : public glu::ast::ASTWalker<LocalCSWalker, void> {
    ConstraintSystem _cs;
    glu::DiagnosticManager &_diagManager;
    glu::ast::ASTContext *_astContext;

    llvm::raw_ostream *_dumpConstraints
        = nullptr; ///< Whether to dump constraints

public:
    LocalCSWalker(
        ScopeTable *scope, glu::DiagnosticManager &diagManager,
        glu::ast::ASTContext *context,
        llvm::raw_ostream *dumpConstraints = nullptr
    )
        : _cs(scope, diagManager, context)
        , _diagManager(diagManager)
        , _astContext(context)
        , _dumpConstraints(dumpConstraints)
    {
    }

    ~LocalCSWalker()
    {
        if (_dumpConstraints) {
            printConstraints(_cs, *_dumpConstraints);
        }

        _cs.solveConstraints();
    }

    /// @brief preVisit method for all expressions to ensure they have a type
    /// before visiting them.
    void preVisitExprBase(glu::ast::ExprBase *node)
    {
        // Ensure the expression has a type before visiting it
        if (!node->getType()) {
            // If the type is not set, we can create a type variable for it
            auto typeVar = _astContext->getTypesMemoryArena()
                               .create<glu::types::TypeVariableTy>();
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
        _cs.addConstraint(
            Constraint::createDefaultable(
                _cs.getAllocator(), fromType, toType, node
            )
        );
        _cs.addConstraint(
            Constraint::createBind(
                _cs.getAllocator(), toType, node->getType(), node
            )
        );
    }

    /// @brief Visits a variable declaration and generates type constraints.
    void postVisitAssignStmt(glu::ast::AssignStmt *node)
    {
        glu::ast::ExprBase *lhs = node->getExprLeft();
        glu::ast::ExprBase *rhs = node->getExprRight();

        glu::types::TypeBase *leftType = lhs->getType();

        auto constraint
            = Constraint::createConversion(_cs.getAllocator(), rhs, leftType);
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

        ConstraintKind kind = ConstraintKind::ExpressibleByIntLiteral;

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
                    kind = ConstraintKind::ExpressibleByFloatLiteral;
                } else if constexpr (std::is_same_v<T, bool>) {
                    // Boolean literal
                    defaultType = memoryArena.create<glu::types::BoolTy>();
                    kind = ConstraintKind::ExpressibleByBoolLiteral;
                } else if constexpr (std::is_same_v<T, llvm::StringRef>) {
                    // String literal - create pointer to char type
                    defaultType = _cs.getScopeTable()->lookupType("String");
                    kind = ConstraintKind::ExpressibleByStringLiteral;
                } else if constexpr (std::is_same_v<T, std::nullptr_t>) {
                    defaultType = memoryArena.create<glu::types::NullTy>();
                    _cs.addConstraint(
                        Constraint::createBind(
                            _cs.getAllocator(), nodeType, defaultType, node
                        )
                    );
                    kind = ConstraintKind::NumberOfConstraints;
                }
            },
            value
        );

        if (kind == ConstraintKind::NumberOfConstraints)
            return;

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
        auto constraint = glu::sema::Constraint::createExpressibleByLiteral(
            _cs.getAllocator(), nodeType, node, kind
        );
        _cs.addConstraint(constraint);
    }

    /// @brief Visits a return statement and generates type constraints.
    void postVisitReturnStmt(glu::ast::ReturnStmt *node)
    {
        auto *expectedReturnType = _cs.getScopeTable()
                                       ->getFunctionDecl()
                                       ->getType()
                                       ->getReturnType();

        if (llvm::isa<glu::types::VoidTy>(expectedReturnType)
            && node->getReturnExpr() != nullptr) {
            _diagManager.error(
                node->getLocation(),
                "Function declared as void cannot return a value"
            );
            return;
        }

        auto *returnExpr = node->getReturnExpr();
        if (returnExpr) {
            _cs.addConstraint(
                Constraint::createConversion(
                    _cs.getAllocator(), returnExpr, expectedReturnType
                )
            );
        } else {
            auto *returnType = node->getModule()
                                   ->getContext()
                                   ->getTypesMemoryArena()
                                   .create<glu::types::VoidTy>();

            _cs.addConstraint(
                Constraint::createEqual(
                    _cs.getAllocator(), returnType, expectedReturnType, node
                )
            );
        }
    }

    /// @brief Visits an if statement's condition and constrains it to boolean.
    void _visitIfStmt(glu::ast::IfStmt *node)
    {
        auto *cond = node->getCondition();
        visit(cond);
        _cs.setRoot(cond);
        auto &memoryArena
            = cond->getModule()->getContext()->getTypesMemoryArena();
        auto *boolType = memoryArena.create<glu::types::BoolTy>();
        auto constraint
            = Constraint::createConversion(_cs.getAllocator(), cond, boolType);
        _cs.addConstraint(constraint);
    }

    /// @brief Visits a while statement's condition and constrains it to
    /// boolean.
    void _visitWhileStmt(glu::ast::WhileStmt *node)
    {
        auto *cond = node->getCondition();
        visit(cond);
        _cs.setRoot(cond);
        auto &memoryArena
            = cond->getModule()->getContext()->getTypesMemoryArena();
        auto *boolType = memoryArena.create<glu::types::BoolTy>();
        auto constraint
            = Constraint::createConversion(_cs.getAllocator(), cond, boolType);
        _cs.addConstraint(constraint);
    }

    void _visitForStmt([[maybe_unused]] glu::ast::ForStmt *node)
    {
        // TODO: #439 Implement for-loops
    }

    /// @brief Visits a ternary conditional expression and generates type
    /// constraints.
    void postVisitTernaryConditionalExpr(glu::ast::TernaryConditionalExpr *node)
    {
        auto *trueType = node->getTrueExpr()->getType();
        auto *falseType = node->getFalseExpr()->getType();
        auto *ternaryType = node->getType();

        auto &memoryArena
            = node->getModule()->getContext()->getTypesMemoryArena();
        auto boolType = memoryArena.create<glu::types::BoolTy>();

        _cs.addConstraint(
            Constraint::createConversion(
                _cs.getAllocator(), node->getCondition(), boolType
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

    void postVisitVarLetDecl(glu::ast::VarLetDecl *varLet)
    {
        auto *varType = varLet->getType();
        auto *value = varLet->getValue();

        if (!varType) {
            auto *typeVar = _astContext->getTypesMemoryArena()
                                .create<glu::types::TypeVariableTy>();
            varLet->setType(typeVar);
            _cs.addTypeVariable(typeVar);
            varType = typeVar;
        }

        if (value) {
            auto constraint = Constraint::createConversion(
                _cs.getAllocator(), value, varType
            );
            _cs.addConstraint(constraint);
        }
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

    void postVisitUnaryOpExpr(glu::ast::UnaryOpExpr *node)
    {
        auto *operandTy = node->getOperand()->getType();
        auto *resultTy = node->getType();

        auto &arena = node->getModule()->getContext()->getTypesMemoryArena();

        auto *expectedFnTy = arena.create<glu::types::FunctionTy>(
            llvm::ArrayRef<glu::types::TypeBase *> { operandTy }, resultTy
        );

        _cs.addConstraint(
            Constraint::createConversion(
                _cs.getAllocator(), node->getOperator(), expectedFnTy
            )
        );
    }

    void postVisitCallExpr(glu::ast::CallExpr *node)
    {
        auto *refExpr = llvm::dyn_cast<glu::ast::RefExpr>(node->getCallee());
        if (!refExpr)
            return handlePointerCall(node);

        auto *actualFnTy = this->actualFnTypeFromCallExpr(node);

        _cs.addConstraint(
            Constraint::createConversion(
                _cs.getAllocator(), node->getCallee(), actualFnTy
            )
        );
    }

    void postVisitStructInitializerExpr(glu::ast::StructInitializerExpr *node)
    {
        auto constraint = Constraint::createStructInitialiser(
            _cs.getAllocator(), node->getType(), node
        );
        _cs.addConstraint(constraint);
    }

    void postVisitBinaryOpExpr(glu::ast::BinaryOpExpr *node)
    {
        auto *typesArena
            = &node->getModule()->getContext()->getTypesMemoryArena();

        auto *lhs = node->getLeftOperand();
        auto *rhs = node->getRightOperand();
        auto *resultTy = node->getType();

        auto *concreteFnTy = typesArena->create<glu::types::FunctionTy>(
            llvm::ArrayRef<glu::types::TypeBase *> { lhs->getType(),
                                                     rhs->getType() },
            resultTy
        );

        _cs.addConstraint(
            Constraint::createConversion(
                _cs.getAllocator(), node->getOperator(), concreteFnTy
            )
        );
    }

    void postVisitRefExpr(glu::ast::RefExpr *node)
    {
        auto *item = _cs.getScopeTable()->lookupItem(node->getIdentifiers());
        auto decls = item ? item->decls : decltype(item->decls)();

        llvm::SmallVector<Constraint *, 4> constraints;

        for (auto decl : decls) {
            if (auto *fnDecl
                = llvm::dyn_cast<glu::ast::FunctionDecl>(decl.item)) {
                constraints.push_back(
                    Constraint::createBindOverload(
                        _cs.getAllocator(), node->getType(), fnDecl, node
                    )
                );
            } else if (auto *varDecl
                       = llvm::dyn_cast<glu::ast::VarLetDecl>(decl.item)) {
                constraints.push_back(
                    Constraint::createBind(
                        _cs.getAllocator(), varDecl->getType(), node->getType(),
                        node
                    )
                );
                node->setVariable(varDecl);
            }
        }

        // Special cases for operators that are overloadables but also have
        // built-in meanings
        // This is because we don't have generics for now, so we can't express
        // them as generic functions
        // Additionally, shortcircuiting operators have special evaluation rules
        // that we can't express with normal functions
        handleRefExprSpecialBuiltins(node, constraints);

        if (!constraints.empty()) {
            auto *disjunction = Constraint::createDisjunction(
                _cs.getAllocator(), constraints, node,
                /*rememberChoice=*/false
            );
            _cs.addConstraint(disjunction);
        } else {
            _diagManager.error(
                node->getLocation(),
                llvm::Twine("No overloads found for '")
                    + node->getIdentifiers().toString() + "'"
            );
        }
    }

private:
    void handleRefExprSpecialBuiltins(
        ast::RefExpr *node, llvm::SmallVector<Constraint *, 4> &constraints
    )
    {
        handleRefExprUnarySpecialBuiltins(node, constraints);
        handleRefExprBinarySpecialBuiltins(node, constraints);
    }

    void handleRefExprUnarySpecialBuiltins(
        ast::RefExpr *node, llvm::SmallVector<Constraint *, 4> &constraints
    )
    {
        auto *parent = llvm::dyn_cast<ast::UnaryOpExpr>(node->getParent());

        if (!parent || parent->getOperator() != node)
            return;

        preVisitExprBase(parent->getOperand()); // Ensure operand has a type

        Constraint *addConstraint = nullptr;

        if (node->getIdentifier() == ".*") {
            addConstraint = Constraint::createBindToPointerType(
                _cs.getAllocator(), parent->getType(),
                parent->getOperand()->getType(), node
            );
        }
        if (node->getIdentifier() == "&") {
            addConstraint = Constraint::createBindToPointerType(
                _cs.getAllocator(), parent->getOperand()->getType(),
                parent->getType(), node
            );
        }

        if (addConstraint) {
            auto &types = _astContext->getTypesMemoryArena();
            auto *fnTy = types.create<glu::types::FunctionTy>(
                llvm::ArrayRef<glu::types::TypeBase *> {
                    parent->getOperand()->getType() },
                parent->getType()
            );
            constraints.push_back(
                Constraint::createConjunction(
                    _cs.getAllocator(),
                    { addConstraint,
                      Constraint::createBind(
                          _cs.getAllocator(), node->getType(), fnTy, node
                      ) },
                    node
                )
            );
        }
    }

    void handleRefExprBinarySpecialBuiltins(
        ast::RefExpr *node, llvm::SmallVector<Constraint *, 4> &constraints
    )
    {
        auto *parent = llvm::dyn_cast<ast::BinaryOpExpr>(node->getParent());

        if (!parent || parent->getOperator() != node)
            return;

        preVisitExprBase(parent->getLeftOperand());
        preVisitExprBase(parent->getRightOperand());

        auto &types = _astContext->getTypesMemoryArena();
        types::FunctionTy *fnTy = nullptr;

        if (node->getIdentifier() == "&&" || node->getIdentifier() == "||") {
            auto *boolTy = types.create<types::BoolTy>();
            fnTy = types.create<types::FunctionTy>(
                llvm::ArrayRef<types::TypeBase *> { boolTy, boolTy }, boolTy
            );
        }
        if (node->getIdentifier() == "[") {
            auto *u64 = types.create<types::IntTy>(types::IntTy::Unsigned, 64);
            auto *ptrTy = types.create<types::PointerTy>(parent->getType());
            fnTy = types.create<types::FunctionTy>(
                llvm::ArrayRef<types::TypeBase *> { ptrTy, u64 },
                parent->getType()
            );
        }
        if (fnTy) {
            constraints.push_back(
                Constraint::createBind(
                    _cs.getAllocator(), node->getType(), fnTy, node
                )
            );
        }
    }

    /// @brief Handles function calls through function pointers
    void handlePointerCall(glu::ast::CallExpr *node)
    {
        auto *calleeType = node->getCallee()->getType();
        if (!calleeType)
            return;

        auto *actualFnTy = this->actualFnTypeFromCallExpr(node);

        _cs.addConstraint(
            Constraint::createConversion(
                _cs.getAllocator(), node->getCallee(), actualFnTy
            )
        );
    }

    glu::types::FunctionTy *
    actualFnTypeFromCallExpr(glu::ast::CallExpr *node) const
    {
        llvm::SmallVector<glu::types::TypeBase *, 4> argTypes;
        for (auto *arg : node->getArgs()) {
            argTypes.push_back(arg->getType());
        }

        auto &arena = _astContext->getTypesMemoryArena();

        return arena.create<glu::types::FunctionTy>(argTypes, node->getType());
    }
};

void runLocalCSWalker(
    ScopeTable *scope, ast::ASTNode *node, glu::DiagnosticManager &diagManager,
    glu::ast::ASTContext *context, llvm::raw_ostream *dumpConstraints
)
{
    LocalCSWalker(scope, diagManager, context, dumpConstraints).visit(node);
}

} // namespace glu::sema
