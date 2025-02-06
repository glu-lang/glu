#ifndef GLU_PARSER_HPP
#define GLU_PARSER_HPP

#include "Basic/Tokens.hpp"
#include "Lexer/Scanner.hpp"

namespace glu {
class BisonParser; // Forward declaration of the class generated by Bison
}

#include "Parser.tab.hpp" // Include the generated Bison parser

namespace glu {

class Parser {
    Scanner &scanner;
    std::unique_ptr<BisonParser> parser;

public:
    explicit Parser(Scanner &s, bool debug = false) : scanner(s)
    {
        parser = std::make_unique<BisonParser>(scanner);

        if (debug) {
            parser->set_debug_level(1);
        }
    }

    ~Parser() = default;

    bool parse() { return parser->parse() == 0; }
};

} // namespace glu

#endif
