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

        // Check that the sets of terminals and nonterminals are disjoint
        bool validate_sets();

        // Output language to log
        void log_language();
    };
} // namespace ParserGenreator

#endif
