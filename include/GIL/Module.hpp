#ifndef GLU_GIL_MODULE_HPP_
#define GLU_GIL_MODULE_HPP_

#include "Function.hpp"
#include "Global.hpp"
#include <list>
#include <string>

namespace glu::gil {

/// @brief This class represents modules in our gil intermediate representation
/// Modules are used to automaticaly import all delcarations in a file,
/// see https://glu-lang.org/modules/
class Module {
public:
    using FunctionListType = llvm::iplist<Function>;
    using GlobalListType = llvm::iplist<Global>;

private:
    FunctionListType _functions;
    GlobalListType _globals;
    llvm::StringRef _importName;
    llvm::StringRef _filepath;

public:
    /// @brief Constructor for a Module instance from a name and a filepath
    /// @param importName A string representing the module's import name
    /// @param filepath A string representing the module's file path
    Module(llvm::StringRef importName, llvm::StringRef filepath = "")
        : _importName(importName), _filepath(filepath)
    {
    }

    Module(ast::ModuleDecl *decl)
        : _importName(decl->getImportName()), _filepath(decl->getFilePath())
    {
    }

    /// deleted copy constructor for a const Module instance
    Module(Module const &) = delete;

    /// deleted copy constructor for a Module instance
    Module(Module &) = delete;

    // defined to be used by ilist
    static FunctionListType Module::*getSublistAccess(Function *)
    {
        return &Module::_functions;
    }
    static GlobalListType Module::*getSublistAccess(Global *)
    {
        return &Module::_globals;
    }

    /// @brief Add a function to the module
    Function *addFunction(Function *fn);

    /// @brief Add a global to the module
    Global *addGlobal(Global *global);

    /// @brief Getter for the module import name
    /// @return Returns the module import name as a string
    llvm::StringRef const &getImportName() const { return _importName; }

    /// @brief Getter for the module file path
    /// @return Returns the module file path as a string
    llvm::StringRef const &getFilePath() const { return _filepath; }

    /// @brief Getter for the functions list
    /// @return Returns a ref to the functions list
    FunctionListType &getFunctions() { return _functions; }

    /// @brief Getter for the globals list
    /// @return Returns a ref to the globals list
    GlobalListType &getGlobals() { return _globals; }

    /// @brief Setter for the module import name
    /// @param name A string representing the new module import name
    void setImportName(llvm::StringRef name) { _importName = name; }

    /// @brief deletes a function from the functions list using a pointer
    /// @param f a pointer to the function to delete
    void deleteFunction(Function *f);

    /// @brief clears the functions list
    void clearFunctions() { _functions.clear(); };

    /// @brief Print a human-readable representation of this module to
    /// standard output, for debugging purposes.
    void print();
};

}
#endif /* !GLU_GIL_MODULE_HPP_ */
