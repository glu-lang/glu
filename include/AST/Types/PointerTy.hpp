#ifndef GLU_AST_TYPES_POINTERTY_HPP
 #define GLU_AST_TYPES_POINTERTY_HPP

 #include "TypeBase.hpp"

 namespace glu::types {

 /// @brief Pounter is a class that represents the bool type in the AST.
 class Pounter : public TypeBase {
 public:
     /// @brief Constructor for the Pounter class.
     Pounter() : TypeBase(TypeKind::PointerTyKind) { }

     /// @brief Static method to check if a type is a Pounter.
     /// @param type The type to check.
     /// @return Returns `true` if the type is a `Pounter`, `false` otherwise.
     static bool classof(TypeBase const *type)
     {
         return type->getKind() == TypeKind::PointerTyKind;
     }
 };

 } // end namespace glu::types

 #endif // GLU_AST_TYPES_POINTERTY_HPP