#ifndef GLU_AST_DECL_MODULEDECL_HPP
#define GLU_AST_DECL_MODULEDECL_HPP

#include "ASTNode.hpp"

#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>

namespace glu::ast {
/// @class ModuleDecl
/// @brief Represents a module declaration in the AST.
///
/// This class inherits from DeclBase and encapsulates the details of a module
/// declaration.
class ModuleDecl : public DeclBase {
    llvm::StringRef _name;
    llvm::ArrayRef<DeclBase *> _decls;

public:
    /// @brief Constructor for the ModuleDecl class.
    /// @param location The source location of the module declaration.
    /// @param name The name of the module.
    /// @param decls A vector of declarations within the module.
    ModuleDecl(
        SourceLocation location, llvm::StringRef name,
        llvm::ArrayRef<DeclBase *> decls
    )
        : DeclBase(NodeKind::ModuleDeclKind, location, nullptr)
        , _name(name)
        , _decls(decls)
    {
    }

    /// @brief Getter for the name of the module.
    /// @return Returns the name of the module.
    llvm::StringRef getName() const { return _name; }

    /// @brief Getter for the declarations within the module.
    /// @return Returns a vector of declarations within the module.
    llvm::ArrayRef<DeclBase *> getDecls() const { return _decls; }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::ModuleDeclKind;
    }
};

}

#endif // GLU_AST_DECL_MODULEDECL_HPP