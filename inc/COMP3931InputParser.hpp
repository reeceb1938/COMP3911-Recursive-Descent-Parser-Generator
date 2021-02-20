#ifndef __COMP3931_INPUT_PARSER_HEADER__
#define __COMP3931_INPUT_PARSER_HEADER__

#include <string>

#include "COMP3911Language.hpp"

namespace ParserGenreator {
    class InputParser {
    public:
        InputParser(std::string input_filename);
        ~InputParser();

    private:
        enum ParseStage {
            STAGE_ERROR,
            STAGE_1_TERMINALS,
            STAGE_2_NONTERMINALS,
            STAGE_3_PRODUCTIONS
        };

        Language language;

        bool parse_terminal_declaration(std::string line);
    };
} // namespace ParserGenreator

#endif
