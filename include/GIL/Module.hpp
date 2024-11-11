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
    using FunctionListType = llvm::iplist<Function>;

    std::string _moduleName;
    std::list<Function> _functions;

public:
    /// @brief Constructor for a Module instance that initialise its name
    /// @param moduleName A string representing the modules name
    Module(std::string moduleName) : _moduleName(moduleName) { };

    /// @brief Adds a Function to the Functions dedicated chained list
    /// @param name A string representing the function name
    /// @param type A pointer to the function type
    void addFunction(
        std::string name, glu::types::FunctionTy *type,
        std::list<BasicBlock> basicBlocks
    );

    /// @brief Getter for constant a pointer on a function in the functions list
    /// @param name A string representing the function name
    /// @return Returns a pointer to the function if it exists, nullptr
    /// otherwise
    Function const *getFunction(std::string name);

    /// @brief Getter for the module name
    /// @return Returns the module name as a string
    std::string const &getName() const { return _moduleName; };
};

}
#endif /* !GLU_GIL_MODULE_HPP_ */
