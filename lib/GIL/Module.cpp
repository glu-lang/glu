#include "Module.hpp"
#include <memory>

namespace glu::gil {

Module::Module(std::string moduleName) : _moduleName(moduleName)
{
}

void Module::addFunction(Function &&function)
{
    _declarations.emplace_back(std::move(function));
}

void Module::addModule(Module *module)
{
    _modules.emplace_back(module);
}

void Module::addType(Type *type)
{
    _types.emplace_back(type);
}

Module *Module::getModule(std::string moduleName)
{
    for (auto &it : _modules)
        if (it->_moduleName == moduleName)
            return it;
    return NULL;
}

bool Module::removeModule(std::string moduleName)
{
    for (auto it = _modules.begin(); it != _modules.end(); it++)
        if ((*it)->_moduleName == moduleName) {
            _modules.erase(it);
            return true;
        }
    return false;
}

}
