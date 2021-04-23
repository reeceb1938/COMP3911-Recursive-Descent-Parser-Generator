#ifndef __COMP3931_EBNFTOKEN_HEADER__
#define __COMP3931_EBNFTOKEN_HEADER__

#include <string>
#include <vector>

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
        std::vector<EBNFToken*>& get_children();

        std::string to_string() const;

    private:
        TokenType type;
        std::string value;
        std::vector<EBNFToken*> children;
    };
} // namespace ParserGenreator

#endif
