#include "TypeLifter.hpp"
#include "AST/Decls.hpp"
#include <llvm/IR/DerivedTypes.h>

namespace glu::irdec {

llvm::StringRef
TypeLifter::copyString(llvm::StringRef str, llvm::BumpPtrAllocator &alloc) const
{
    char *mem = static_cast<char *>(alloc.Allocate(str.size(), 1));
    memcpy(mem, str.data(), str.size());
    return llvm::StringRef(mem, str.size());
}

glu::types::TypeBase *TypeLifter::lift(llvm::Type *type) const
{
    auto &typesArena = _context.getTypesMemoryArena();
    auto &astArena = _context.getASTMemoryArena();

    switch (type->getTypeID()) {
    case llvm::Type::VoidTyID: return typesArena.create<types::VoidTy>();
    case llvm::Type::HalfTyID: return typesArena.create<types::FloatTy>(16);
    case llvm::Type::FloatTyID: return typesArena.create<types::FloatTy>(32);
    case llvm::Type::DoubleTyID: return typesArena.create<types::FloatTy>(64);
    case llvm::Type::X86_FP80TyID: return typesArena.create<types::FloatTy>(80);
    case llvm::Type::FP128TyID: return typesArena.create<types::FloatTy>(128);
    case llvm::Type::PPC_FP128TyID:
        return typesArena.create<types::FloatTy>(128);
    case llvm::Type::IntegerTyID: {
        if (auto ty = llvm::dyn_cast<llvm::IntegerType>(type)) {
            return typesArena.create<types::IntTy>(
                types::IntTy::Signedness::Signed, ty->getIntegerBitWidth()
            );
        }
        return nullptr;
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
        std::vector<ast::FieldDecl *> fieldDecls;
        auto structTy = llvm::dyn_cast<llvm::StructType>(type);
        for (unsigned i = 0; i < structTy->getNumElements(); i++) {
            llvm::Type *fieldTy = structTy->getElementType(i);
            std::string name = ("F" + llvm::Twine(i)).str();
            fieldDecls.push_back(astArena.create<ast::FieldDecl>(
                SourceLocation::invalid,
                copyString(name, astArena.getAllocator()), lift(fieldTy)
            ));
        }
        auto structDecl = astArena.create<ast::StructDecl>(
            _context, SourceLocation::invalid, nullptr,
            copyString(
                structTy->getStructName().str(), astArena.getAllocator()
            ),
            fieldDecls
        );
        return typesArena.create<types::StructTy>(structDecl);
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
}
