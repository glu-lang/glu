#ifndef GLU_TOOLS_GLULSP_DOCUMENTMANAGER_HPP
#define GLU_TOOLS_GLULSP_DOCUMENTMANAGER_HPP

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <nlohmann/json.hpp>

#include "Basic/SourceManager.hpp"
#include "Basic/Diagnostic.hpp"
#include "AST/Decls.hpp"
#include "AST/ASTContext.hpp"

namespace glu::lsp {

using json = nlohmann::json;

/// @brief Represents a document version
struct DocumentVersion {
    std::string uri;
    std::string content;
    int version;
    
    DocumentVersion(const std::string& uri, const std::string& content, int version)
        : uri(uri), content(content), version(version) {}
};

/// @brief Manages open documents and their analysis results
class DocumentManager {
public:
    DocumentManager();
    
    /// @brief Add or update a document
    void updateDocument(const std::string& uri, const std::string& content, int version);
    
    /// @brief Close a document
    void closeDocument(const std::string& uri);
    
    /// @brief Get document content
    std::string getDocumentContent(const std::string& uri) const;
    
    /// @brief Check if document is open
    bool isDocumentOpen(const std::string& uri) const;
    
    /// @brief Get diagnostics for a document
    std::vector<json> getDiagnostics(const std::string& uri);
    
    /// @brief Get document symbols
    std::vector<json> getDocumentSymbols(const std::string& uri);
    
    /// @brief Get hover information at position
    json getHover(const std::string& uri, int line, int character);
    
private:
    /// @brief Analyze a document and update diagnostics
    void analyzeDocument(const std::string& uri);
    
    /// @brief Convert file path to URI
    std::string filePathToUri(const std::string& path) const;
    
    /// @brief Convert URI to file path
    std::string uriToFilePath(const std::string& uri) const;
    
    /// @brief Create diagnostic JSON from glu diagnostic
    json createDiagnostic(const glu::Diagnostic& diag, const glu::SourceManager& sourceManager);
    
    std::unordered_map<std::string, std::unique_ptr<DocumentVersion>> _documents;
    std::unordered_map<std::string, std::vector<json>> _diagnostics;
    std::unordered_map<std::string, std::unique_ptr<glu::ast::ModuleDecl>> _astCache;
};

} // namespace glu::lsp

#endif // GLU_TOOLS_GLULSP_DOCUMENTMANAGER_HPP