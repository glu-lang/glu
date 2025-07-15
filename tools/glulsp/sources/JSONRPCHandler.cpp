#include "JSONRPCHandler.hpp"
#include <iostream>
#include <sstream>
#include <regex>

namespace glu::lsp {

JSONRPCHandler::JSONRPCHandler() {}

void JSONRPCHandler::registerMethod(const std::string& method, RequestHandler handler) {
    _methods[method] = handler;
}

json JSONRPCHandler::processRequest(const json& request) {
    if (!request.contains("jsonrpc") || request["jsonrpc"] != "2.0") {
        return createErrorResponse(
            request.value("id", nullptr),
            -32600,
            "Invalid JSON-RPC version"
        );
    }
    
    if (!request.contains("method")) {
        return createErrorResponse(
            request.value("id", nullptr),
            -32600,
            "Missing method"
        );
    }
    
    std::string method = request["method"];
    json id = request.value("id", nullptr);
    json params = request.value("params", json::object());
    
    // Handle notifications (no id)
    if (id.is_null()) {
        if (_methods.find(method) != _methods.end()) {
            _methods[method](params);
        }
        return json(); // No response for notifications
    }
    
    // Handle requests
    if (_methods.find(method) == _methods.end()) {
        return createErrorResponse(id, -32601, "Method not found");
    }
    
    try {
        json result = _methods[method](params);
        return json{
            {"jsonrpc", "2.0"},
            {"id", id},
            {"result", result}
        };
    } catch (const std::exception& e) {
        return createErrorResponse(id, -32603, e.what());
    }
}

void JSONRPCHandler::runMessageLoop() {
    std::string line;
    while (std::getline(std::cin, line)) {
        // Remove carriage return if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        // Parse Content-Length header
        size_t contentLength = parseContentLength(line);
        if (contentLength == 0) continue;
        
        // Read until we get an empty line (end of headers)
        while (std::getline(std::cin, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (line.empty()) break;
        }
        
        // Read the content
        std::string content(contentLength, '\0');
        std::cin.read(&content[0], contentLength);
        
        try {
            json request = json::parse(content);
            json response = processRequest(request);
            
            if (!response.empty()) {
                sendResponse(response);
            }
        } catch (const std::exception& e) {
            json error = createErrorResponse(
                nullptr,
                -32700,
                "Parse error: " + std::string(e.what())
            );
            sendResponse(error);
        }
    }
}

size_t JSONRPCHandler::parseContentLength(const std::string& headers) {
    std::regex contentLengthRegex(R"(Content-Length:\s*(\d+))");
    std::smatch match;
    
    if (std::regex_search(headers, match, contentLengthRegex)) {
        return std::stoul(match[1].str());
    }
    
    return 0;
}

void JSONRPCHandler::sendResponse(const json& response) {
    std::string content = response.dump();
    std::cout << "Content-Length: " << content.length() << "\r\n\r\n";
    std::cout << content;
    std::cout.flush();
}

json JSONRPCHandler::createErrorResponse(const json& id, int code, const std::string& message) {
    return json{
        {"jsonrpc", "2.0"},
        {"id", id},
        {"error", {
            {"code", code},
            {"message", message}
        }}
    };
}

} // namespace glu::lsp