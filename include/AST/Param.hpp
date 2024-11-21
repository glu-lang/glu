#ifndef GLU_AST_PARAM_HPP
#define GLU_AST_PARAM_HPP

#include "Types/TypeBase.hpp"
#include <string>

namespace glu::ast {

struct Param {
    std::string name;
    glu::types::TypeBase *type;

    Param(std::string name, glu::types::TypeBase *type)
        : name(std::move(name)), type(type)
    {
    }
};

} // namespace glu::ast

#endif // GLU_AST_PARAM_HPP
