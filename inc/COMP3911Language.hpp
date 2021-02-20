#ifndef __COMP3931_LANGUAGE_HEADER__
#define __COMP3931_LANGUAGE_HEADER__

#include <string>
#include <set>
#include <unordered_map>

namespace ParserGenreator {
    class Language {
    public:
        Language();
        ~Language();
        std::set<std::string> terminals;
        std::set<std::string> nonterminals;
        std::unordered_map<std::string, std::set<std::string>> production_rules; 
    };
} // namespace ParserGenreator

#endif
