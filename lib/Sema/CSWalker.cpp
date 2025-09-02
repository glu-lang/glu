#include "Sema/CSWalker.hpp"
#include "AST/ASTWalker.hpp"
#include "AST/Exprs.hpp"
#include "AST/Types.hpp"
#include "Sema/ConstraintSystem.hpp"

#include "UnresolvedNameTyMapper.hpp"

#include <variant>

namespace glu::sema {

class LocalCSWalker : public glu::ast::ASTWalker<LocalCSWalker, void> {
    ConstraintSystem _cs;
    glu::DiagnosticManager &_diagManager;
    glu::ast::ASTContext *_astContext;

public:
    LocalCSWalker(
        ScopeTable *scope, glu::DiagnosticManager &diagManager,
        glu::ast::ASTContext *context, bool enableConstraintLogging = false,
        glu::SourceManager *sourceManager = nullptr
    )
        : _cs(scope, diagManager, context, enableConstraintLogging, llvm::errs(), sourceManager)
        , _diagManager(diagManager)
        , _astContext(context)
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

    void postVisitVarLetDecl(glu::ast::VarLetDecl *varLet)
    {
        auto *varType = varLet->getType();
        auto *value = varLet->getValue();
        if (!value)
            return;
        auto *valueType = value->getType();

        if (!varType) {
            auto *typeVar = _astContext->getTypesMemoryArena()
                                .create<glu::types::TypeVariableTy>();
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

    void postVisitUnaryOpExpr(glu::ast::UnaryOpExpr *node)
    {
        auto *operandTy = node->getOperand()->getType();
        auto *resultTy = node->getType();

        auto &arena = node->getModule()->getContext()->getTypesMemoryArena();

        auto *expectedFnTy = arena.create<glu::types::FunctionTy>(
            llvm::ArrayRef<glu::types::TypeBase *> { operandTy }, resultTy
        );

        generateConversionConstraint(
            node->getOperator()->getType(), expectedFnTy, node
        );
    }

    void postVisitCallExpr(glu::ast::CallExpr *node)
    {
        auto *refExpr = llvm::dyn_cast<glu::ast::RefExpr>(node->getCallee());
        if (!refExpr)
            return handlePointerCall(node);

        auto *expectedFnTy = this->expectedFnTypeFromCallExpr(node);

        generateConversionConstraint(
            node->getCallee()->getType(), expectedFnTy, node
        );
    }

    void postVisitStructInitializerExpr(glu::ast::StructInitializerExpr *node)
    {
        // TODO: Maybe nothing to do ?
    }

    void postVisitBinaryOpExpr(glu::ast::BinaryOpExpr *node)
    {
        auto *typesArena
            = &node->getModule()->getContext()->getTypesMemoryArena();

        auto *lhs = node->getLeftOperand();
        auto *rhs = node->getRightOperand();
        auto *resultTy = node->getType();

        auto *expectedFnTy = typesArena->create<glu::types::FunctionTy>(
            llvm::ArrayRef<glu::types::TypeBase *> { lhs->getType(),
                                                     rhs->getType() },
            resultTy
        );

        generateConversionConstraint(
            node->getOperator()->getType(), expectedFnTy, node
        );
    }

    void postVisitRefExpr(glu::ast::RefExpr *node)
    {
        llvm::StringRef name = node->getIdentifier();

        auto *item = _cs.getScopeTable()->lookupItem(name);
        llvm::ArrayRef<glu::ast::DeclBase *> decls
            = item ? item->decls : llvm::ArrayRef<glu::ast::DeclBase *>();

        llvm::SmallVector<Constraint *, 4> constraints;

        for (auto *decl : decls) {
            if (auto *fnDecl = llvm::dyn_cast<glu::ast::FunctionDecl>(decl)) {
                constraints.push_back(
                    Constraint::createBindOverload(
                        _cs.getAllocator(), node->getType(), fnDecl, node
                    )
                );
            } else if (auto *varDecl
                       = llvm::dyn_cast<glu::ast::VarLetDecl>(decl)) {
                constraints.push_back(
                    Constraint::createConversion(
                        _cs.getAllocator(), varDecl->getType(), node->getType(),
                        node
                    )
                );
                node->setVariable(varDecl);
            }
        }

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
                    + node->getIdentifier().str() + "'"
            );
        }
    }

private:
    /// @brief Handles function calls through function pointers
    void handlePointerCall(glu::ast::CallExpr *node)
    {
        auto *calleeType = node->getCallee()->getType();
        if (!calleeType)
            return;

        auto *expectedFnTy = this->expectedFnTypeFromCallExpr(node);

        _cs.addConstraint(
            Constraint::createConversion(
                _cs.getAllocator(), calleeType, expectedFnTy, node
            )
        );
    }

    glu::types::FunctionTy *
    expectedFnTypeFromCallExpr(glu::ast::CallExpr *node) const
    {
        llvm::SmallVector<glu::types::TypeBase *, 4> argTypes;
        for (auto *arg : node->getArgs()) {
            argTypes.push_back(arg->getType());
        }

        auto &arena = _astContext->getTypesMemoryArena();
        return arena.create<glu::types::FunctionTy>(argTypes, node->getType());
    }

    void generateConversionConstraint(
        glu::types::TypeBase *fromTy, glu::types::TypeBase *toTy,
        glu::ast::ExprBase *anchor
    )
    {
        _cs.addConstraint(
            Constraint::createConversion(
                _cs.getAllocator(), fromTy, toTy, anchor
            )
        );
    }
};

class GlobalCSWalker : public glu::ast::ASTWalker<GlobalCSWalker, void> {
    llvm::BumpPtrAllocator _scopeTableAllocator;
    ScopeTable *_scopeTable;
    glu::DiagnosticManager &_diagManager;
    glu::ast::ASTContext *_context;
    bool _constraintLoggingEnabled;
    glu::SourceManager *_sourceManager;

public:
    GlobalCSWalker(
        glu::DiagnosticManager &diagManager, glu::ast::ASTContext *context,
        glu::SourceManager *sourceManager = nullptr
    )
        : _diagManager(diagManager), _context(context), _constraintLoggingEnabled(false),
          _sourceManager(sourceManager)
    {
    }

    void setConstraintLogging(bool enabled) {
        _constraintLoggingEnabled = enabled;
    }

    void preVisitModuleDecl(glu::ast::ModuleDecl *node)
    {
        _scopeTable = new (_scopeTableAllocator) ScopeTable(node);
        UnresolvedNameTyMapper mapper(*_scopeTable, _diagManager, _context);

        mapper.visit(node);
    }

    void postVisitModuleDecl([[maybe_unused]] glu::ast::ModuleDecl *node)
    {
        _scopeTable = _scopeTable->getParent();
    }

    void preVisitFunctionDecl(glu::ast::FunctionDecl *node)
    {
        _scopeTable = new (_scopeTableAllocator) ScopeTable(_scopeTable, node);
    }

    void postVisitFunctionDecl([[maybe_unused]] glu::ast::FunctionDecl *node)
    {
        _scopeTable = _scopeTable->getParent();
    }

    void preVisitCompoundStmt(glu::ast::CompoundStmt *node)
    {
        _scopeTable = new (_scopeTableAllocator) ScopeTable(_scopeTable, node);
    }

    void postVisitCompoundStmt([[maybe_unused]] glu::ast::CompoundStmt *node)
    {
        assert(_scopeTable->getParent() && "Cannot pop global scope");
        _scopeTable = _scopeTable->getParent();
    }

    void preVisitForStmt(glu::ast::ForStmt *node)
    {
        _scopeTable = new (_scopeTableAllocator) ScopeTable(_scopeTable, node);
    }

    void postVisitForStmt([[maybe_unused]] glu::ast::ForStmt *node)
    {
        _scopeTable = _scopeTable->getParent();
    }

    void postVisitVarLetDecl(glu::ast::VarLetDecl *node)
    {
        _scopeTable->insertItem(node->getName(), node);
    }

    void preVisitStmtBase(glu::ast::StmtBase *node)
    {
        ScopeTable local(_scopeTable, node);
        LocalCSWalker walker(&local, _diagManager, _context, _constraintLoggingEnabled, _sourceManager);
        walker.visit(node);
    }
};

void constrainAST(
    glu::ast::ModuleDecl *module, glu::DiagnosticManager &diagManager,
    bool enableConstraintLogging
)
{
    GlobalCSWalker walker(diagManager, module->getContext(), &diagManager.getSourceManager());
    walker.setConstraintLogging(enableConstraintLogging);
    walker.visit(module);
}
}
