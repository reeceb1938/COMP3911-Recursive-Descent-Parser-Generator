#ifndef __COMP3931_PARSER_GENERATOR_HEADER__
#define __COMP3931_PARSER_GENERATOR_HEADER__

#include <string>

#include "COMP3931Grammar.hpp"

namespace ParserGenerator {

    class Generator {
    public:
        Generator(Grammar& grammar, std::string output_file_name);
        ~Generator();

    private:
        Grammar& grammar;
        std::string output_file_name;
    };

} // namespace ParserGenerator

#endif
