#ifndef __COMP3931_GRAMMAR_NEW_HEADER__
#define __COMP3931_GRAMMAR_NEW_HEADER__

#include <list>
#include <string>
#include <set>
#include <unordered_map>

namespace ParserGenreator {

    // Class to represent items in the parse tree of an EBNF production rule
    class EBNFToken {
    public:
        enum TokenType {
            SEQUENCE,
            TERMINAL,
            NONTERMINAL,
            OR,
            REPEAT,
            OPTIONAL,
            GROUP
        };

        EBNFToken(TokenType type, std::string value);
        ~EBNFToken();

        TokenType get_type();
        std::string get_value();

        void add_child(EBNFToken* new_child);
        std::list<EBNFToken*>& get_children();

        std::string to_string() const;

    private:
        TokenType type;
        std::string value;
        std::list<EBNFToken*> children;
    };

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

        void log_language();

    private:
        std::set<std::string> terminals;
        std::set<std::string> nonterminals;
        std::unordered_map<std::string, EBNFToken*> production_rules;
        std::string start_symbol;

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
    };

} // namespace ParserGenreator

#endif
