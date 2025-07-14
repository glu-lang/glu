#ifndef GLU_GIL_MODULE_HPP_
#define GLU_GIL_MODULE_HPP_

#include "Function.hpp"
#include <list>
#include <string>

namespace glu::gil {

/// @brief This class represents modules in our gil intermediate representation
/// Modules are used to automaticaly import all delcarations in a file,
/// see https://glu-lang.org/modules/
class Module {
public:
    using FunctionListType = llvm::iplist<Function>;

private:
    FunctionListType _functions;
    llvm::StringRef _name;

public:
    /// @brief Constructor for a Module instance that initialise its name
    /// @param name A string representing the modules name
    Module(llvm::StringRef name) : _name(name) { };

    /// deleted copy constructor for a const Module instance
    Module(Module const &) = delete;

    /// deleted copy constructor for a Module instance
    Module(Module &) = delete;

    // defined to be used by ilist
    static FunctionListType Module::*getSublistAccess(Function *)
    {
        return &Module::_functions;
    }

    /// @brief Add a function to the module
    Function *addFunction(Function *fn);

    /// @brief Getter for constant a pointer on a function in the functions list
    /// @param name A string representing the function name
    /// @return Returns a pointer to the function if it exists, nullptr
    /// otherwise
    Function const *getFunction(llvm::StringRef name) const;

    /// @brief Getter for the module name
    /// @return Returns the module name as a string
    llvm::StringRef const &getName() const { return _name; };

    /// @brief Getter for the functions list
    /// @return Returns a ref to the functions list
    FunctionListType &getFunctions() { return _functions; }

    /// @brief Setter for the module name
    /// @param name A string representing the new module name
    void setName(llvm::StringRef name) { _name = name; }

    /// @brief deletes a function from the functions list using a pointer
    /// @param f a pointer to the function to delete
    void deleteFunction(Function *f);

    /// @brief clears the functions list
    void clearFunctions() { _functions.clear(); };
};

}
#endif /* !GLU_GIL_MODULE_HPP_ */
