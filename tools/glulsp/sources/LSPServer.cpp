#include "LSPServer.hpp"
#include <iostream>

namespace glu::lsp {

LSPServer::LSPServer() 
    : _jsonRpcHandler(std::make_unique<JSONRPCHandler>()),
      _documentManager(std::make_unique<DocumentManager>()) {
    
    // Register LSP methods
    _jsonRpcHandler->registerMethod("initialize", [this](const json& params) {
        return handleInitialize(params);
    });
    
    _jsonRpcHandler->registerMethod("initialized", [this](const json& params) {
        return handleInitialized(params);
    });
    
    _jsonRpcHandler->registerMethod("shutdown", [this](const json& params) {
        return handleShutdown(params);
    });
    
    _jsonRpcHandler->registerMethod("exit", [this](const json& params) {
        return handleExit(params);
    });
    
    _jsonRpcHandler->registerMethod("textDocument/didOpen", [this](const json& params) {
        return handleDidOpen(params);
    });
    
    _jsonRpcHandler->registerMethod("textDocument/didChange", [this](const json& params) {
        return handleDidChange(params);
    });
    
    _jsonRpcHandler->registerMethod("textDocument/didClose", [this](const json& params) {
        return handleDidClose(params);
    });
    
    _jsonRpcHandler->registerMethod("textDocument/documentSymbol", [this](const json& params) {
        return handleDocumentSymbol(params);
    });
    
    _jsonRpcHandler->registerMethod("textDocument/hover", [this](const json& params) {
        return handleHover(params);
    });
}

void LSPServer::run() {
    _jsonRpcHandler->runMessageLoop();
}

json LSPServer::handleInitialize(const json& params) {
    return json{
        {"capabilities", {
            {"textDocumentSync", 1}, // Full sync
            {"hoverProvider", true},
            {"documentSymbolProvider", true},
            {"diagnosticProvider", {
                {"interFileDependencies", false},
                {"workspaceDiagnostics", false}
            }}
        }},
        {"serverInfo", {
            {"name", "Glu Language Server"},
            {"version", "0.1.0"}
        }}
    };
}

json LSPServer::handleInitialized(const json& params) {
    // Client has been initialized, ready for requests
    return json();
}

json LSPServer::handleShutdown(const json& params) {
    _shutdownRequested = true;
    return json();
}

json LSPServer::handleExit(const json& params) {
    exit(_shutdownRequested ? 0 : 1);
    return json();
}

json LSPServer::handleDidOpen(const json& params) {
    auto textDocument = params["textDocument"];
    std::string uri = textDocument["uri"];
    std::string text = textDocument["text"];
    int version = textDocument["version"];
    
    _documentManager->updateDocument(uri, text, version);
    sendDiagnostics(uri);
    
    return json();
}

json LSPServer::handleDidChange(const json& params) {
    auto textDocument = params["textDocument"];
    std::string uri = textDocument["uri"];
    int version = textDocument["version"];
    
    // Get the new content from the first content change
    auto contentChanges = params["contentChanges"];
    if (!contentChanges.empty()) {
        std::string text = contentChanges[0]["text"];
        _documentManager->updateDocument(uri, text, version);
        sendDiagnostics(uri);
    }
    
    return json();
}

json LSPServer::handleDidClose(const json& params) {
    auto textDocument = params["textDocument"];
    std::string uri = textDocument["uri"];
    
    _documentManager->closeDocument(uri);
    
    return json();
}

json LSPServer::handleDocumentSymbol(const json& params) {
    auto textDocument = params["textDocument"];
    std::string uri = textDocument["uri"];
    
    return _documentManager->getDocumentSymbols(uri);
}

json LSPServer::handleHover(const json& params) {
    auto textDocument = params["textDocument"];
    std::string uri = textDocument["uri"];
    
    auto position = params["position"];
    int line = position["line"];
    int character = position["character"];
    
    return _documentManager->getHover(uri, line, character);
}

void LSPServer::sendDiagnostics(const std::string& uri) {
    auto diagnostics = _documentManager->getDiagnostics(uri);
    
    json params = {
        {"uri", uri},
        {"diagnostics", diagnostics}
    };
    
    sendNotification("textDocument/publishDiagnostics", params);
}

void LSPServer::sendNotification(const std::string& method, const json& params) {
    json notification = {
        {"jsonrpc", "2.0"},
        {"method", method},
        {"params", params}
    };
    
    std::string content = notification.dump();
    std::cout << "Content-Length: " << content.length() << "\r\n\r\n";
    std::cout << content;
    std::cout.flush();
}

} // namespace glu::lsp