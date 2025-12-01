#ifndef TEST_PARSER_COMMON_HPP
#define TEST_PARSER_COMMON_HPP

#include "Parser.hpp"
#include "Scanner.hpp"

#include <gtest/gtest.h>
#include <memory>

#define PREP_PARSER(str)                                            \
    std::unique_ptr<llvm::MemoryBuffer> buf(                        \
        llvm::MemoryBuffer::getMemBufferCopy(str)                   \
    );                                                              \
    glu::SourceManager sm;                                          \
    glu::ast::ASTContext context(&sm);                              \
    glu::Scanner scanner(buf.get(), context.getScannerAllocator()); \
    glu::DiagnosticManager diag(sm);                                \
    sm.loadBuffer(std::move(buf), "main.glu");                      \
    glu::Parser parser(scanner, context, sm, diag /*, 1*/)

#define PREP_MAIN_PARSER(str) PREP_PARSER("func main() {" str "}")

#endif // TEST_PARSER_COMMON_HPP
