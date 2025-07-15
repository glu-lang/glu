#include "LSPServer.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        glu::lsp::LSPServer server;
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}