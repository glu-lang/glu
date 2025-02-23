#ifndef GLU_AST_DECL_IMPORTDECL_HPP
#define GLU_AST_DECL_IMPORTDECL_HPP

#include "ASTNode.hpp"
#include "Types.hpp"

namespace glu::ast {

/// @struct ImportPath
/// @brief Represents the import path as components.
/// For example, for "std::io::{println}", components will contain ["std", "io"]
/// and selectors will contain ["println"].
struct ImportPath {
    std::vector<llvm::StringRef> components;
    std::vector<llvm::StringRef> selectors;
};

/// @class ImportDecl
/// @brief Represents an import declaration in the AST.
///
/// This class inherits from DeclBase and encapsulates the details of an import
/// declaration.
class ImportDecl : public DeclBase {
    ImportPath _importPath;

public:
    /// @brief Constructor for the ImportDecl class.
    /// @param location The source location of the import declaration.
    /// @param parent The parent AST node.
    /// @param importPath The path of the module to import.
    ImportDecl(
        SourceLocation location, ASTNode *parent, ImportPath const &importPath
    )
        : DeclBase(NodeKind::ImportDeclKind, location, parent)
        , _importPath(importPath)
    {
    }

    /// @brief Getter for the import path.
    /// @return Returns the import path.
    ImportPath const &getImportPath() const { return _importPath; }

    /// @brief Static method to check if a node is a ImportDecl.
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::ImportDeclKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_IMPORTDECL_HPP
