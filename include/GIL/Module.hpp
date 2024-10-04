#ifndef GLU_GIL_MODULE_HPP_
#define GLU_GIL_MODULE_HPP_

#include <list>
#include <string>
#include <memory>
#include "Function.hpp"

namespace glu::gil {

class Type;

/// @brief This class represents modules in our gil intermediate representation
/// Modules are used to automaticaly import all delcarations in a file,
/// see https://glu-lang.org/modules/
class Module {
public:
    /// @brief Constructor for a Module instance that initialise its name
    /// @param moduleName A string representing the modules name
    Module(std::string moduleName);

    /// @brief Adds a Function to the Functions dedicated chained list
    /// @param function A pointer to the function that will be added
    void addFunction(Function &&function);

    /// @brief Adds a Module to the Modules dedicated chained list
    /// @param module A pointer to the module that will be added
    void addModule(Module *module);

    /// @brief Adds a Type to the Types dedicated chained list
    /// @param type A pointer to the type that will be added
    void addType(Type *type);

    /// @brief Finds and get the module that corresponds to the given name value
    /// @param moduleName A string that represents the name value
    /// @return Returns a pointer to the module that corresponds to the name
    ///         value or NULL if there's no modules that corresponds
    Module *getModule(std::string moduleName);

private:
    std::string _moduleName;
    std::list<Function> _declarations;
    std::list<Module *> _modules;
    std::list<Type *> _types;
};

}
#endif /* !GLU_GIL_MODULE_HPP_ */
