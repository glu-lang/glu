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
    void addFunction(std::unique_ptr<Function> function);

private:
    std::string _moduleName;
    std::list<std::unique_ptr<Function>> _declarations;
};

}
#endif /* !GLU_GIL_MODULE_HPP_ */
