#ifndef GLU_GIL_MODULES_HPP_
#define GLU_GIL_MODULES_HPP_

#include <list>
#include <string>

namespace glu::gil {

/// @brief This class represents modules in our gil intermediate representation.
/// Modules are used to automaticaly import all delcarations in a file.
/// See https://glu-lang.org/modules/
class GILModule {
public:
    /// @brief Constructor for a GILModule instance that initialise its name.
    /// @param moduleName A string representing the modules name.
    GILModule(std::string moduleName);


private:
    std::string _moduleName;
    // std::list<DeclBase> _declarations;
};

}
#endif /* !GLU_GIL_MODULES_HPP_ */
