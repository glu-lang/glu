#ifndef GLU_IRGEN_MANGLING_HPP
#define GLU_IRGEN_MANGLING_HPP

#include "Context.hpp"

#include "AST/Decls.hpp"
#include "GIL/Type.hpp"
#include "Types.hpp"
#include "Types/TypeVisitor.hpp"

#include <sstream>

namespace glu::irgen {

class Mangler {
    std::ostringstream ss;

public:
    std::string str() const { return ss.str(); }

    Mangler &operator<<(std::string const &str)
    {
        ss << str.size();
        ss << str;
        return *this;
    }

    Mangler &operator<<(char c)
    {
        ss << c;
        return *this;
    }

    Mangler &operator<<(size_t n)
    {
        ss << n;
        return *this;
    }

    // modules always start with digits, always end with a letter
    Mangler &operator<<(ast::ModuleDecl *module)
    {
        auto moduleName = module->getImportName();
        for (auto component : llvm::make_range(
                 llvm::sys::path::begin(moduleName),
                 llvm::sys::path::end(moduleName)
             )) {
            *this << component.str();
        }
        return *this;
    }

    Mangler &operator<<(glu::types::TypeBase *type);

    Mangler &operator<<(ast::FunctionDecl *fn)
    {
        ss << "$GLU$";
        *this << fn->getModule();
        *this << fn->getName().str();
        *this << fn->getType();
        return *this;
    }
};

// types always start with a letter (not R), can end with a digit
class TypeMangler
    : public glu::types::TypeVisitor<TypeMangler, void, Mangler &> {

public:
    void visitTypeBase(
        [[maybe_unused]] glu::types::TypeBase *type, [[maybe_unused]] Mangler &m
    )
    {
        assert(false && "Unknown type kind");
    }

    void visitVoidTy([[maybe_unused]] glu::types::VoidTy *type, Mangler &m)
    {
        m << 'v';
    }

    void visitBoolTy([[maybe_unused]] glu::types::BoolTy *type, Mangler &m)
    {
        m << 'b';
    }

    void visitCharTy([[maybe_unused]] glu::types::CharTy *type, Mangler &m)
    {
        m << 'c';
    }

    void visitDynamicArrayTy(
        [[maybe_unused]] glu::types::DynamicArrayTy *type, Mangler &m
    )
    {
        m << 'D';
    }

    void visitEnumTy(glu::types::EnumTy *type, Mangler &m)
    {
        m << 'T' << type->getDecl()->getModule() << type->getName().str()
          << 'E';
    }

    void visitIntTy(glu::types::IntTy *type, Mangler &m)
    {
        if (type->isSigned()) {
            m << 'i';
        } else {
            m << 'u';
        }
        m << (size_t) type->getBitWidth();
    }

    void visitFloatTy(glu::types::FloatTy *type, Mangler &m)
    {
        m << 'f' << (size_t) type->getBitWidth();
    }

    void visitFunctionTy(glu::types::FunctionTy *type, Mangler &m)
    {
        m << 'F';
        visit(type->getReturnType(), m);
        for (auto *param : type->getParameters()) {
            visit(param, m);
        }
        m << 'R';
    }

    void visitPointerTy(glu::types::PointerTy *type, Mangler &m)
    {
        m << 'P';
        visit(type->getPointee(), m);
    }

    void visitStaticArrayTy(glu::types::StaticArrayTy *type, Mangler &m)
    {
        m << 'A' << type->getSize();
        visit(type->getDataType(), m);
    }

    void visitTypeAliasTy(glu::types::TypeAliasTy *type, Mangler &m)
    {
        visit(type->getWrappedType(), m);
    }

    void visitStructTy(glu::types::StructTy *type, Mangler &m)
    {
        m << 'T' << type->getDecl()->getModule() << type->getName().str()
          << 'S';
    }
};

inline Mangler &Mangler::operator<<(glu::types::TypeBase *type)
{
    TypeMangler tm;
    tm.visit(type, *this);
    return *this;
}

std::string mangleFunctionName(ast::FunctionDecl *fn)
{
    Mangler m;
    m << fn;
    return m.str();
}

} // namespace glu::irgen

#endif // GLU_IRGEN_MANGLING_HPP
