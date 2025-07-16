#include "DocumentManager.hpp"
#include "Lexer/Scanner.hpp"
#include "Parser/Parser.hpp"
#include "Sema/CSWalker.hpp"
#include <nlohmann/json.hpp>
#include <regex>
#include <fstream>

using json = nlohmann::json;

namespace glu::lsp {

DocumentManager::DocumentManager() {}

void DocumentManager::updateDocument(const std::string& uri, const std::string& content, int version) {
    _documents[uri] = std::make_unique<DocumentVersion>(uri, content, version);
    analyzeDocument(uri);
}

void DocumentManager::closeDocument(const std::string& uri) {
    _documents.erase(uri);
    _diagnostics.erase(uri);
    _astCache.erase(uri);
}

std::string DocumentManager::getDocumentContent(const std::string& uri) const {
    auto it = _documents.find(uri);
    if (it != _documents.end()) {
        return it->second->content;
    }
    return "";
}

bool DocumentManager::isDocumentOpen(const std::string& uri) const {
    return _documents.find(uri) != _documents.end();
}

std::vector<json> DocumentManager::getDiagnostics(const std::string& uri) {
    auto it = _diagnostics.find(uri);
    if (it != _diagnostics.end()) {
        return it->second;
    }
    return {};
}

std::vector<json> DocumentManager::getDocumentSymbols(const std::string& uri) {
    std::vector<json> symbols;
    
    auto astIt = _astCache.find(uri);
    if (astIt == _astCache.end()) {
        return symbols;
    }
    
    auto* module = astIt->second.get();
    if (!module) {
        return symbols;
    }
    
    // Extract function symbols
    for (auto* decl : module->getDecls()) {
        if (auto* funcDecl = llvm::dyn_cast<glu::ast::FunctionDecl>(decl)) {
            json symbol = {
                {"name", funcDecl->getName()},
                {"kind", 12}, // Function
                {"location", {
                    {"uri", uri},
                    {"range", {
                        {"start", {{"line", 0}, {"character", 0}}},
                        {"end", {{"line", 0}, {"character", 0}}}
                    }}
                }}
            };
            symbols.push_back(symbol);
        }
    }
    
    return symbols;
}

json DocumentManager::getHover(const std::string& uri, int line, int character) {
    // For now, return empty hover
    return json();
}

void DocumentManager::analyzeDocument(const std::string& uri) {
    auto it = _documents.find(uri);
    if (it == _documents.end()) {
        return;
    }
    
    const std::string& content = it->second->content;
    _diagnostics[uri].clear();
    
    try {
        // Create a temporary file for analysis
        std::string tempPath = "/tmp/glulsp_temp.glu";
        std::ofstream tempFile(tempPath);
        tempFile << content;
        tempFile.close();
        
        // Initialize analysis components
        glu::SourceManager sourceManager;
        glu::DiagnosticManager diagManager(sourceManager);
        glu::ast::ASTContext context(&sourceManager);
        
        auto fileID = sourceManager.loadFile(tempPath.c_str());
        if (!fileID) {
            return;
        }
        
        // Tokenize and parse
        glu::Scanner scanner(sourceManager.getBuffer(*fileID));
        glu::Parser parser(scanner, context, sourceManager, diagManager);
        
        if (parser.parse()) {
            auto ast = llvm::cast<glu::ast::ModuleDecl>(parser.getAST());
            if (ast) {
                // Store AST for symbol analysis
                _astCache[uri] = std::unique_ptr<glu::ast::ModuleDecl>(ast);
                
                // Run semantic analysis
                sema::constrainAST(ast, diagManager);
            }
        }
        
        // Convert diagnostics to LSP format
        auto& diagnostics = diagManager.getMessages();
        for (const auto& diag : diagnostics) {
            json lspDiag = createDiagnostic(diag, sourceManager);
            _diagnostics[uri].push_back(lspDiag);
        }
        
        // Clean up temp file
        std::remove(tempPath.c_str());
        
    } catch (const std::exception& e) {
        // Add error diagnostic
        json errorDiag = {
            {"range", {
                {"start", {{"line", 0}, {"character", 0}}},
                {"end", {{"line", 0}, {"character", 0}}}
            }},
            {"severity", 1}, // Error
            {"message", std::string("Analysis error: ") + e.what()}
        };
        _diagnostics[uri].push_back(errorDiag);
    }
}

std::string DocumentManager::filePathToUri(const std::string& path) const {
    return "file://" + path;
}

std::string DocumentManager::uriToFilePath(const std::string& uri) const {
    if (uri.substr(0, 7) == "file://") {
        return uri.substr(7);
    }
    return uri;
}

json DocumentManager::createDiagnostic(const glu::Diagnostic& diag, const glu::SourceManager& sourceManager) {
    int severity = 1; // Error by default
    if (diag.getSeverity() == glu::DiagnosticSeverity::Warning) {
        severity = 2; // Warning
    }
    
    // For now, create a simple diagnostic at line 0
    // In a real implementation, we'd get the actual location from the diagnostic
    return json{
        {"range", {
            {"start", {{"line", 0}, {"character", 0}}},
            {"end", {{"line", 0}, {"character", 0}}}
        }},
        {"severity", severity},
        {"message", diag.getMessage()}
    };
}

} // namespace glu::lsp