#ifndef GLU_AST_TYPES_HPP
#define GLU_AST_TYPES_HPP

#include "Types/BoolTy.hpp"
#include "Types/CharTy.hpp"
#include "Types/DynamicArrayTy.hpp"
#include "Types/EnumTy.hpp"
#include "Types/FloatTy.hpp"
#include "Types/FunctionTy.hpp"
#include "Types/IntTy.hpp"
#include "Types/PointerTy.hpp"
#include "Types/StaticArrayTy.hpp"
#include "Types/StructTy.hpp"
#include "Types/TypeAliasTy.hpp"
#include "Types/TypeVariableTy.hpp"
#include "Types/UnresolvedNameTy.hpp"

// Include the TypeVisitor after all the types
#include "Types/TypeVisitor.hpp"

#endif // GLU_AST_TYPES_HPP
