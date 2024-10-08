#ifndef GLU_AST_TYPES_TYPEBASE_HPP_
#define GLU_AST_TYPES_TYPEBASE_HPP_

#include <string>

namespace glu::types {

/// @brief This enum represents the binding Type of a type, which corresponds to
///        whether this type is constant or variable.
enum BindingType {
    /// @brief The type is variable
    VAR,
    /// @brief The type is constant
    CONST
};

/// @brief Base class for every Type definition. Contains the most basic
///        elements each types should at least have.
class TypeBase {
protected:
    BindingType const _binding;
    std::string const _name;
    size_t const _size;

public:
    /// @brief Virtual destructor for polymorphisme
    virtual ~TypeBase() = default;

    /// @brief Virtual getter for the size of the Type.
    /// @return Returns the size of the type as a size_t.
    virtual inline size_t getSize() const
    {
        return _size;
    }

    /// @brief Virtual getter for the Binding of the Type.
    /// @return Returns the Binding of the Type as a BindingType.
    virtual inline BindingType getBinding() const
    {
        return _binding;
    }

    /// @brief Virtual getter for the Name of the Type.
    /// @return Returns the Name of the Type as a String.
    virtual inline std::string getName() const
    {
        return _name;
    }
};

}
#endif /* !GLU_AST_TYPES_TYPEBASE_HPP_ */
