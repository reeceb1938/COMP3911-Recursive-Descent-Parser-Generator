#ifndef __COMP3931_GRAMMAR_HEADER__
#define __COMP3931_GRAMMAR_HEADER__

#include <list>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "COMP3931EBNFToken.hpp"

namespace ParserGenreator {

    // Class to represnet the defined grammar
    class Grammar {
    public:
        Grammar();
        ~Grammar();

        bool input_language_from_file(std::string file_path);
        bool add_terminal(std::string new_terminal);
        bool add_nonterminal(std::string new_nonterminal);
        bool add_production(std::string nonterminal, EBNFToken* new_production);
        bool set_start_symbol(std::string new_start_symbol);

        bool is_terminal(std::string to_find);
        bool is_nonterminal(std::string to_find);

        bool can_produce_ll_parser();

        void log_grammar();

    private:
        std::set<std::string> terminals;
        std::set<std::string> nonterminals;
        std::unordered_map<std::string, EBNFToken*> production_rules;
        std::string start_symbol;
        std::unordered_map<std::string, std::set<std::string>> first_sets;
        std::unordered_map<std::string, std::set<std::string>> follow_sets;

        // Functions to parse a grammar input file
        bool file_parse_INPUT_FILE(std::ifstream& input);
        bool file_parse_TERM_DECLAR(std::ifstream& input);
        bool file_parse_NONTERM_DECLAR(std::ifstream& input);
        bool file_parse_TERMINAL(std::ifstream& input, std::string& terminal);
        bool file_parse_GRAM_DECLAR(std::ifstream& input);
        bool file_parse_PRODUCTION(std::ifstream& input);
        bool file_parse_LHS(std::ifstream& input, std::string& new_lhs);
        bool file_parse_RHS(std::ifstream& input, EBNFToken* new_rhs);
        bool file_parse_TERM(std::ifstream& input, EBNFToken* new_rhs);
        bool file_parse_FACTOR(std::ifstream& input, EBNFToken* new_rhs);

        bool file_parse_skip_white_space(std::ifstream& input);
        bool file_parse_end_of_line(std::ifstream& input);
        bool file_parse_check_char(std::ifstream& input, char character);

        // Functions to validate the grammar can be used to produce an LL(1) parser
        bool calculate_first_set();
        // Calculate the terminals that need to be added to the first set for a particular nonterminal
        std::set<std::string> calculate_first_terminal(EBNFToken* ebnf_token);

        bool calculate_follow_set();
        // Calculate the terminals that need to be added to the follow set for a particular nonterminal
        bool calculate_follow_terminal(std::string production_lhs, EBNFToken* ebnf_token, std::vector<std::set<std::string>>& current_trailers);
    };

} // namespace ParserGenreator

#endif
