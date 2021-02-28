#include <map>
#include <set>
#include <string>

#include "COMP3911Language.hpp"
#include "spdlog/spdlog.h"

using namespace ParserGenreator;

Language::Language() {

}

Language::~Language() {

}

bool Language::validate_sets() {
    return false;
}

// TODO: Make this functions output cleaner 
void Language::log_language() {
    std::string tmp;
    for (const std::string terminal : terminals) {
        tmp += "`" + terminal + "`, ";
    }
    spdlog::info("Terminals found: {}", tmp);

    tmp = "";
    for (const std::string nonterminal : nonterminals) {
        tmp += "`" + nonterminal + "`, ";
    }
    spdlog::info("Nonterminals found: {}", tmp);

    spdlog::info("Productions found:");
    for (const std::pair<std::string, std::set<std::string>>& pair : production_rules) {
        tmp = pair.first + " ::= ";

        for (const std::string production : pair.second) {
            tmp += "`" + production + "` | ";
        }

        spdlog::info(tmp);
    }
}
