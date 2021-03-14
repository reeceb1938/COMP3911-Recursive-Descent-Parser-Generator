#ifndef __COMP3931_LANGUAGE_HEADER__
#define __COMP3931_LANGUAGE_HEADER__

#include <string>
#include <set>
#include <unordered_map>
#include <list>

namespace ParserGenreator {
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

        EBNFToken(TokenType type);
        ~EBNFToken();

        TokenType get_type();
        void set_type(TokenType new_type);

        EBNFToken* add_child(TokenType new_token_type, std::string value);
        void add_child(EBNFToken* new_child);
        std::list<EBNFToken*>& get_children();

    private:
        TokenType type;
        std::string value;
        std::list<EBNFToken*> children;
    };

    class Language {
    public:
        Language();
        ~Language();

        bool add_terminal(std::string new_terminal);
        bool add_nonterminal(std::string new_nonterminal);
        bool add_production(std::string nonterminal, std::string new_production);
        bool set_start_symbol(std::string new_start_symbol);

        // Perform checks on the langauge definition to ensure it is valid
        bool validate_language();

        // Output language to log
        void log_language();

    private:
        std::set<std::string> terminals;
        std::set<std::string> nonterminals;
        std::unordered_map<std::string, std::set<std::string>> production_rules;
        std::string start_symbol;

        EBNFToken* parse_ebnf(std::string& raw_ebnf, std::size_t ebnf_index = 0);
    };
} // namespace ParserGenreator

#endif
