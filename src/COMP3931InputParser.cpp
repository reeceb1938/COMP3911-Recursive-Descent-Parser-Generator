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
                    if (parse_nonterminal_declaration(line)) {
                        stage = STAGE_3_PRODUCTIONS;
                    } else {
                        stage = STAGE_ERROR;
                    }
                    break;
                case STAGE_3_PRODUCTIONS:
                    spdlog::trace("Parsing stage 3 (productions)");
                    if (line == "P:") {
                        stage = STAGE_3_PRODUCTION_RULES;
                    } else {
                        spdlog::error("Expected begining of production declaration `P:` but found {}", line);
                        stage = STAGE_ERROR;
                    }
                    break;
                case STAGE_3_PRODUCTION_RULES:
                    spdlog::trace("Parsing production: {}", line);
                    if (parse_production_rule(line)) {
                        // Do nothing, there can be infintely many production rules then EOF
                    } else {
                        stage = STAGE_ERROR;
                    }
                    break;
                default:
                    spdlog::critical("Unknown parser state. How did we get here?");
                    stage = STAGE_ERROR;
            }
        }

        spdlog::trace("Done reading file: {}", input_filename);

        language.log_language();
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
    // NOTE: terminal_start_index = 0 represents no value (instead of -1) because std::size_t is unsigned and it should never be 0 as line[0] == `T`
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
                            terminal_end_index = line.length();
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

// Given a line of text from the input parse it as the line that declares
// the nonterminals of the grammar as defined in the input file specification
bool InputParser::parse_nonterminal_declaration(std::string line) {
    std::size_t line_index = 0;
    // NOTE: nonterminal_start_index = 0 represents no value (instead of -1) because std::size_t is unsigned and it should never be 0 as line[0] == `T`
    std::size_t nonterminal_start_index = 0;
    bool last_char_escape = false;  // Was the previously seen character `\`

    while (line_index <= line.length()) {
        if (line_index == 0) {
            if (line[line_index] != 'N') {
                spdlog::error("Parsing nonterminal declaration failed. Invalid syntax");
                spdlog::info("Terminal declaraion should start `NT:` but found: `{}`", line);
                break;
            }
        } else if (line_index == 1) {
            if (line[line_index] != 'T') {
                spdlog::error("Parsing nonterminal declaration failed. Invalid syntax");
                spdlog::info("Terminal declaraion should start `NT:` but found: `{}`", line);
                break;
            }
        } else if(line_index == 2) {
            if (line[line_index] != ':') {
                spdlog::error("Parsing nonterminal declaration failed. Invalid syntax");
                spdlog::info("Terminal declaraion should start `NT:` but found: `{}`", line);
                break;
            }
        } else {
            // Parse each item in the comma seperated list of nonterminals

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
                        spdlog::error("Unknown escape sequence in nonterminal list: `{}`", line.substr(line_index - 1, 2));
                        break;
                    }

                    if (nonterminal_start_index == 0) {
                        nonterminal_start_index = line_index;
                    }
                } else {
                    if (line[line_index] == '\\') {
                        last_char_escape = true;
                    } else if (line[line_index] == ',' || line[line_index] == '\0') {
                        // End of terminal definition
                        std::size_t nonterminal_end_index = line.find_first_of(" \t\r,", nonterminal_start_index + 1);

                        if (nonterminal_end_index == std::string::npos) {
                            nonterminal_end_index = line.length();
                        }

                        std::string new_nonterminal = line.substr(nonterminal_start_index, nonterminal_end_index - nonterminal_start_index);

                        spdlog::trace("Found nonterminal: `{}` ({} : {})", new_nonterminal, nonterminal_start_index, nonterminal_end_index);

                        std::pair<std::set<std::string>::iterator,bool> ret;
                        ret = language.nonterminals.insert(new_nonterminal);

                        if (ret.second == false) {
                            spdlog::warn("Found duplicate definition of nonterminal `{}`. Ignoring second definition", new_nonterminal);
                        }

                        nonterminal_start_index = 0;
                    } else {
                        if (nonterminal_start_index == 0) {
                            nonterminal_start_index = line_index;
                        }
                    }
                }
            }
        }

        line_index++;
    }

    return line_index > line.length();
}

// Given a line of text from the input parse it as a line that declares a
// single production rule of the grammar in the format defined in the input
// file specification
bool InputParser::parse_production_rule(std::string line) {
    std::size_t line_index = 0;
    // NOTE: production_start_index = 0 represents no value (instead of -1) because std::size_t is unsigned and it should never be 0 as line starts `[NONTERMINAL] ::=`
    std::size_t production_start_index = 0;
    std::size_t production_end_index = 0;
    bool last_char_escape = false;  // Was the previously seen character `\`
    bool parsed_lhs = false;  // Have we completed the LHS (i.e. the nonterminal before the ::)
    std::string lhs_nonterminal;  // The nonterminal A in `A ::= B | C | a`

    while (line_index <= line.length()) {
        if (!parsed_lhs) {
            std::size_t end_of_lhs_index = line.find_first_of(":");

            if (end_of_lhs_index == std::string::npos) {
                spdlog::error("Could not find `::=` in declaration of production rule `{}`", line);
                break;
            }

            // Check for existance of `::=`
            if (end_of_lhs_index + 3 > line.length()) {
                spdlog::error("Could not find `::=` in declaration of production rule `{}`", line);
                break;
            }

            if (line[end_of_lhs_index] != ':' || line[end_of_lhs_index + 1] != ':' || line[end_of_lhs_index + 2] != '=') {
                spdlog::error("Could not find `::=` in declaration of production rule `{}`", line);
                break;
            }

            // Trim LHS nonterminal
            production_start_index = line.find_first_not_of(" \t");
            production_end_index = line.find_last_not_of(" \t", end_of_lhs_index - 1);

            if (production_start_index == std::string::npos || production_start_index == end_of_lhs_index) {
                spdlog::error("Empty LHS in production rule `{}`", line);
                break;
            } else if (production_end_index == std::string::npos) {
                // Production is written as `A::=` with no space between nontermianl and `::=`
                production_end_index = end_of_lhs_index;
            }

            lhs_nonterminal = line.substr(production_start_index, production_end_index - production_start_index + 1);

            spdlog::trace("LHS of production `{}` is `{}`", line, lhs_nonterminal);

            // Add the LHS to the map of productions after checking it hasn't already been defined

            if (language.production_rules.count(lhs_nonterminal) != 0) {
                spdlog::error("Redefinition of productions for nonterminal `{}`", lhs_nonterminal);
                break;
            }

            language.production_rules.insert({lhs_nonterminal, std::set<std::string>()});

            line_index = end_of_lhs_index + 2;  // +2 because we already checked ::= was present
            parsed_lhs = true;
        } else {
            // Parse each production in the bar (|) seperated list of productions

            if (line[line_index] == ' ' || line[line_index] == '\t') {

            } else {
                if (last_char_escape == true) {
                    last_char_escape = false;

                    // Replace escaped charatcers
                    if (line[line_index] == '\\') {
                        line.replace(line_index - 1, 2, "\\");
                        line_index--;
                    } else if (line[line_index] == '|') {
                        line.replace(line_index - 1, 2, "|");
                        line_index--;
                    } else {
                        spdlog::error("Unknown escape sequence in nonterminal list: `{}`", line.substr(line_index - 1, 2));
                        break;
                    }

                    if (production_start_index == 0) {
                        production_start_index = line_index;
                    }
                } else {
                    if (line[line_index] == '\\') {
                        last_char_escape = true;
                    } else if (line[line_index] == '|' || line[line_index] == '\0') {
                        // End of production definition
                        std::size_t production_end_index = line.find_last_not_of(" \t\r", line_index - 1);

                        if (production_end_index == std::string::npos) {
                            production_end_index = line.length();
                        }

                        std::string new_production = line.substr(production_start_index, production_end_index - production_start_index + 1);

                        spdlog::trace("Found production: `{} ::= {}` ({} : {})", lhs_nonterminal, new_production, production_start_index, production_end_index);

                        std::pair<std::set<std::string>::iterator,bool> ret;
                        ret = language.production_rules.at(lhs_nonterminal).insert(new_production);

                        if (ret.second == false) {
                            spdlog::warn("Found duplicate definition of production `{} ::= {}`. Ignoring second definition", lhs_nonterminal, new_production);
                        }

                        production_start_index = 0;
                    } else {
                        if (production_start_index == 0) {
                            production_start_index = line_index;
                        }
                    }
                }
            }
        }

        line_index++;
    }

    return line_index > line.length();
}
