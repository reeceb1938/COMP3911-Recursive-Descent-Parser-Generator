#include <fstream>
#include <iterator>
#include <regex>
#include <string>
#include <set>

#include <iostream>

#include "COMP3931InputParser.hpp"
#include "COMP3911Language.hpp"
#include "spdlog/spdlog.h"

using namespace ParserGenreator;

InputParser::InputParser(std::string input_filename) {
    std::ifstream input_file(input_filename);

    if (input_file.is_open()) {
        spdlog::trace("Opened input file: {}", input_filename);

        std::string line;
        ParseStage stage = STAGE_1_TERMINALS;
        while (std::getline(input_file, line) && stage != STAGE_ERROR) {
            // Check for Windows line endings
            if (line.back() == '\r') {
                line.pop_back();
            }

            spdlog::trace("Read line: '{}'", line);

            switch (stage) {
                case STAGE_1_TERMINALS:
                    spdlog::trace("Parsing stage 1 (terminals)");
                    if (parse_terminal_declaration(line)) {
                        stage = STAGE_2_NONTERMINALS;
                    } else {
                        stage = STAGE_ERROR;
                    }
                    break;
                case STAGE_2_NONTERMINALS:
                    spdlog::trace("Parsing stage 2 (non-terminals)");
                    break;
                case STAGE_3_PRODUCTIONS:
                    spdlog::trace("Parsing stage 3 (productions)");
                    break;
                default:
                    spdlog::critical("Unknown parser state. How did we get here?");
            }
        }

        spdlog::trace("Done reading file: {}", input_filename);
    } else {
        spdlog::error("Cannot open input file: {}", input_filename);
    }
}

InputParser::~InputParser() {

}

// Given a line of text from the input parse it as the line that declares
// the temrinals of the grammar as defined in the input file specification
bool InputParser::parse_terminal_declaration(std::string line) {
    std::size_t line_index = 0;
    std::size_t terminal_start_index = 0;
    bool last_char_escape = false;  // Was the previously seen character `\`

    while (line_index <= line.length()) {
        if (line_index == 0) {
            if (line[line_index] != 'T') {
                spdlog::error("Parsing terminal declaration failed. Invalid syntax");
                spdlog::info("Terminal declaraion should start `T:` but found: `{}`", line);
                break;
            }
        } else if(line_index == 1) {
            if (line[line_index] != ':') {
                spdlog::error("Parsing terminal declaration failed. Invalid syntax");
                spdlog::info("Terminal declaraion should start `T:` but found: `{}`", line);
                break;
            }
        } else {
            // Parse each item in the comma seperated list of terminals

            if (line[line_index] == ' ' || line[line_index] == '\t') {

            } else {
                if (last_char_escape == true) {
                    last_char_escape = false;

                    // Replace escaped charatcers
                    if (line[line_index] == '\\') {
                        line.replace(line_index - 1, 2, "\\");
                        line_index--;
                    } else if (line[line_index] == ',') {
                        line.replace(line_index - 1, 2, ",");
                        line_index--;
                    } else {
                        spdlog::error("Unknown escape sequence in terminal list: `{}`", line.substr(line_index - 1, 2));
                        break;
                    }

                    // NOTE: 0 represents no value (instead of -1) because std::size_t is unsigned and it should never be 0 as line[0] == `T`
                    if (terminal_start_index == 0) {
                        terminal_start_index = line_index;
                    }
                } else {
                    if (line[line_index] == '\\') {
                        last_char_escape = true;
                    } else if (line[line_index] == ',' || line[line_index] == '\0') {
                        // End of terminal definition
                        std::size_t terminal_end_index = line.find_first_of(" \t\r,", terminal_start_index + 1);

                        if (terminal_end_index == std::string::npos) {
                            spdlog::error("Internal error. Could not find end of terminal declaration");
                            break;
                        }

                        std::string new_terminal = line.substr(terminal_start_index, terminal_end_index - terminal_start_index);

                        spdlog::trace("Found terminal: `{}` ({} : {})", new_terminal, terminal_start_index, terminal_end_index);

                        std::pair<std::set<std::string>::iterator,bool> ret;
                        ret = language.terminals.insert(new_terminal);

                        if (ret.second == false) {
                            spdlog::warn("Found duplicate definition of terminal `{}`. Ignoring second definition", new_terminal);
                        }

                        terminal_start_index = 0;
                    } else {
                        // NOTE: 0 represents no value (instead of -1) because std::size_t is unsigned and it should never be 0 as line[0] == `T`
                        if (terminal_start_index == 0) {
                            terminal_start_index = line_index;
                        }
                    }
                }
            }
        }

        line_index++;
    }

    return line_index > line.length();
}
