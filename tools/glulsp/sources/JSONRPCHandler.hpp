#ifndef GLU_TOOLS_GLULSP_JSONRPCHANDLER_HPP
#define GLU_TOOLS_GLULSP_JSONRPCHANDLER_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <functional>
#include <unordered_map>

namespace glu::lsp {

using json = nlohmann::json;

/// @brief JSON-RPC request handler
class JSONRPCHandler {
public:
    using RequestHandler = std::function<json(const json& params)>;
    
    JSONRPCHandler();
    
    /// @brief Register a method handler
    void registerMethod(const std::string& method, RequestHandler handler);
    
    /// @brief Process a JSON-RPC request
    json processRequest(const json& request);
    
    /// @brief Read and process messages from stdin
    void runMessageLoop();
    
private:
    /// @brief Parse Content-Length from headers
    size_t parseContentLength(const std::string& headers);
    
    /// @brief Send JSON response to stdout
    void sendResponse(const json& response);
    
    /// @brief Create error response
    json createErrorResponse(const json& id, int code, const std::string& message);
    
    std::unordered_map<std::string, RequestHandler> _methods;
};

} // namespace glu::lsp

#endif // GLU_TOOLS_GLULSP_JSONRPCHANDLER_HPP