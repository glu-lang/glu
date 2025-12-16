#include "AST/Decls.hpp"
#include "ModuleLifter.hpp"

#include <llvm/IR/DerivedTypes.h>

namespace glu::irdec {

class TypeLifter {

    ModuleLiftingContext &_ctx;
    glu::ast::ASTContext &_context;

public:
    TypeLifter(ModuleLiftingContext &ctx) : _ctx(ctx), _context(ctx.ast) { }

    glu::types::StructTy *handleStructType(llvm::StructType *structTy)
    {
        auto &astArena = _context.getASTMemoryArena();
        if (auto *structDecl = llvm::dyn_cast_if_present<ast::StructDecl>(
                _ctx.typeCache[structTy]
            )) {
            return structDecl->getType();
        }

        std::vector<ast::FieldDecl *> fieldDecls;
        for (unsigned i = 0; i < structTy->getNumElements(); i++) {
            llvm::Type *fieldTy = structTy->getElementType(i);
            std::string name = ("field" + llvm::Twine(i)).str();
            fieldDecls.push_back(astArena.create<ast::FieldDecl>(
                SourceLocation::invalid,
                copyString(name, astArena.getAllocator()), lift(fieldTy)
            ));
        }
        auto structDecl = astArena.create<ast::StructDecl>(
            _context, SourceLocation::invalid, nullptr,
            structTy->isLiteral()
                ? ""
                : copyString(
                      structTy->getStructName().str(), astArena.getAllocator()
                  ),
            fieldDecls
        );
        _ctx.typeCache[structTy] = structDecl;
        _ctx.addToNamespace(nullptr, structDecl);
        return structDecl->getType();
    }

    /// @brief Lift an LLVM type to a GLU type
    /// @param type The LLVM type to lift
    /// @return The lifted GLU type, or nullptr if the type could not be lifted
    glu::types::TypeBase *lift(llvm::Type *type)
    {
        auto &typesArena = _context.getTypesMemoryArena();

        switch (type->getTypeID()) {
        case llvm::Type::VoidTyID: return typesArena.create<types::VoidTy>();
        case llvm::Type::HalfTyID: return typesArena.create<types::FloatTy>(16);
        case llvm::Type::FloatTyID:
            return typesArena.create<types::FloatTy>(32);
        case llvm::Type::DoubleTyID:
            return typesArena.create<types::FloatTy>(64);
        case llvm::Type::X86_FP80TyID:
            return typesArena.create<types::FloatTy>(80);
        case llvm::Type::FP128TyID:
            return typesArena.create<types::FloatTy>(128);
        case llvm::Type::PPC_FP128TyID:
            return typesArena.create<types::FloatTy>(128);
        case llvm::Type::IntegerTyID: {
            return typesArena.create<types::IntTy>(
                types::IntTy::Signedness::Signed,
                llvm::cast<llvm::IntegerType>(type)->getIntegerBitWidth()
            );
        }
        case llvm::Type::PointerTyID:
            return typesArena.create<types::PointerTy>(
                typesArena.create<types::CharTy>()
            );
        case llvm::Type::ArrayTyID: {
            auto arrayTy = llvm::cast<llvm::ArrayType>(type);
            return typesArena.create<types::StaticArrayTy>(
                lift(arrayTy->getElementType()), arrayTy->getNumElements()
            );
        }
        case llvm::Type::StructTyID: {
            return handleStructType(llvm::cast<llvm::StructType>(type));
        }
        case llvm::Type::FunctionTyID: {
            auto funcTy = llvm::cast<llvm::FunctionType>(type);
            std::vector<types::TypeBase *> paramTypes;
            for (unsigned i = 0; i < funcTy->getNumParams(); i++) {
                paramTypes.push_back(lift(funcTy->getParamType(i)));
            }
            return typesArena.create<types::FunctionTy>(
                paramTypes, lift(funcTy->getReturnType()), funcTy->isVarArg()
            );
        }
        default: return nullptr;
        }
    }
};

glu::types::TypeBase *lift(llvm::Type *type, ModuleLiftingContext &ctx)
{
    return TypeLifter(ctx).lift(type);
}
}
