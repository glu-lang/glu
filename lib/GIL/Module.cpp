#include "Module.hpp"
#include <memory>

namespace glu::gil {

Module::Module(std::string moduleName) : _moduleName(moduleName)
{
}

void Module::addFunction(std::unique_ptr<Function> function)
{
    _declarations.emplace_back(std::move(function));
}

}
