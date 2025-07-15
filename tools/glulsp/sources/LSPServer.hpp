#ifndef GLU_TOOLS_GLULSP_LSPSERVER_HPP
#define GLU_TOOLS_GLULSP_LSPSERVER_HPP

#include "JSONRPCHandler.hpp"
#include "DocumentManager.hpp"
#include <memory>

namespace glu::lsp {

/// @brief Main LSP server implementation
class LSPServer {
public:
    LSPServer();
    
    /// @brief Start the server
    void run();
    
private:
    /// @brief Handle initialize request
    json handleInitialize(const json& params);
    
    /// @brief Handle initialized notification
    json handleInitialized(const json& params);
    
    /// @brief Handle shutdown request
    json handleShutdown(const json& params);
    
    /// @brief Handle exit notification
    json handleExit(const json& params);
    
    /// @brief Handle textDocument/didOpen notification
    json handleDidOpen(const json& params);
    
    /// @brief Handle textDocument/didChange notification
    json handleDidChange(const json& params);
    
    /// @brief Handle textDocument/didClose notification
    json handleDidClose(const json& params);
    
    /// @brief Handle textDocument/documentSymbol request
    json handleDocumentSymbol(const json& params);
    
    /// @brief Handle textDocument/hover request
    json handleHover(const json& params);
    
    /// @brief Send diagnostics to client
    void sendDiagnostics(const std::string& uri);
    
    /// @brief Send notification to client
    void sendNotification(const std::string& method, const json& params);
    
    std::unique_ptr<JSONRPCHandler> _jsonRpcHandler;
    std::unique_ptr<DocumentManager> _documentManager;
    bool _shutdownRequested = false;
};

} // namespace glu::lsp

#endif // GLU_TOOLS_GLULSP_LSPSERVER_HPP