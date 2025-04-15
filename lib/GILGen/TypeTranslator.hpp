#ifndef GLU_GILGEN_TYPE_TRANSLATOR_HPP
#define GLU_GILGEN_TYPE_TRANSLATOR_HPP

#include "AST/Types.hpp"
#include "GIL/Type.hpp"
#include "llvm/Support/ErrorHandling.h"

namespace glu::gilgen {

/// @brief TypeTranslator is a visitor that converts AST types to GIL types.
class TypeTranslator : public types::TypeVisitor<TypeTranslator, gil::Type> {
public:
    TypeTranslator() { }

    gil::Type visitTypeBase([[maybe_unused]] types::TypeBase *type)
    {
        llvm_unreachable("Should not be called");
    }

// Define a macro to generate visit methods for each type
#define TYPE(NAME) gil::Type visit##NAME(types::NAME *type);
// Include the type definitions to generate method declarations
#include "AST/Types/TypeKind.def"
};

} // namespace glu::gilgen

#endif // GLU_GILGEN_TYPE_TRANSLATOR_HPP
