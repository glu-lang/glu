#include "Basic/Diagnostic.hpp"
#include "ConstraintSystem.hpp"
#include "TypeMapper.hpp"

namespace glu::sema {

class TypeVariableTyMapper : public glu::sema::TypeMapper<TypeVariableTyMapper>,
                             public glu::types::TypeVisitor<
                                 TypeVariableTyMapper, glu::types::TypeBase *> {
    glu::DiagnosticManager &_diagManager;
    Solution *_solution;
    glu::ast::ASTContext *_context;

public:
    TypeVariableTyMapper(
        Solution *solution, glu::DiagnosticManager &diagManager,
        glu::ast::ASTContext *context
    )
        : _diagManager(diagManager), _solution(solution), _context(context)
    {
    }

    glu::types::TypeBase *visitTypeBase(glu::types::TypeBase *type)
    {
        return type;
    }

    glu::types::TypeBase *visitTypeVariableTy(glu::types::TypeVariableTy *type)
    {
        auto mappedType = _solution->getTypeFor(type);
        if (!mappedType) {
            _diagManager.error(
                SourceLocation::invalid,
                // TODO: Use a TypeVisitor to print properly the type
                std::string("Type variable mapping not found for TypeVaribleTy")
            );
            return type;
        }
        return mappedType;
    }

    glu::types::TypeBase *visitFunctionTy(glu::types::FunctionTy *type)
    {
        glu::types::TypeBase *returnType = visit(type->getReturnType());
        std::vector<glu::types::TypeBase *> params;

        for (glu::types::TypeBase *paramType : type->getParameters())
            params.push_back(visit(paramType));

        return _context->getTypesMemoryArena().create<glu::types::FunctionTy>(
            params, returnType
        );
    }

    types::TypeBase *visitPointerTy(types::PointerTy *type)
    {
        glu::types::TypeBase *pointeeType = visit(type->getPointee());
        return _context->getTypesMemoryArena().create<glu::types::PointerTy>(
            pointeeType
        );
    }

    types::TypeBase *visitTypeAliasTy(types::TypeAliasTy *type)
    {
        glu::types::TypeBase *aliasedType = visit(type->getWrappedType());
        return _context->getTypesMemoryArena().create<glu::types::TypeAliasTy>(
            aliasedType, type->getName(), type->getLocation()
        );
    }

    types::TypeBase *visitStaticArrayTy(types::StaticArrayTy *type)
    {
        glu::types::TypeBase *elementType = visit(type->getDataType());
        return _context->getTypesMemoryArena()
            .create<glu::types::StaticArrayTy>(elementType, type->getSize());
    }

    types::TypeBase *visitDynamicArrayTy(types::DynamicArrayTy *type)
    {
        glu::types::TypeBase *elementType = visit(type->getDataType());
        return _context->getTypesMemoryArena()
            .create<glu::types::DynamicArrayTy>(elementType);
    }

    using glu::types::TypeVisitor<
        TypeVariableTyMapper, glu::types::TypeBase *>::visit;
    using glu::sema::TypeMapper<TypeVariableTyMapper>::visit;

    glu::types::TypeBase *mapType(glu::types::TypeBase *type)
    {
        return visit(type);
    }
};

void ConstraintSystem::mapTypeVariables(SolutionResult &solutionRes)
{
    if (solutionRes.isAmbigous()) {
        _diagManager.error(
            SourceLocation::invalid,
            "Ambiguous type variable mapping found, cannot resolve."
        );
        return;
    }

    Solution *solution = solutionRes.getBestSolution();

    if (!solution) {
        _diagManager.error(
            SourceLocation::invalid,
            "No best solution available for type variable mapping."
        );
        return;
    }
    TypeVariableTyMapper mapper(solution, _diagManager, _context);

    mapper.visit(_scopeTable->getGlobalScope()->getModule());
}

} // namespace glu::sema
