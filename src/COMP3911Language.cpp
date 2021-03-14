#include <map>
#include <set>
#include <string>

#include "COMP3911Language.hpp"
#include "spdlog/spdlog.h"

using namespace ParserGenreator;

Language::Language() {
    // Add pre-defined components
    add_terminal("identifier");
    add_terminal("integer_constant");
    add_terminal("string_literal");
    add_nonterminal("EPSILON");
    add_production("EPSILON", "");
    start_symbol = ""; // Clear start_symbol after addition of production
}

Language::~Language() {

}

bool Language::add_terminal(std::string new_terminal) {
    if (nonterminals.find(new_terminal) != nonterminals.end()) {
        spdlog::error("Attempting to add terminal `{}` but it is already declared as a nonterminal", new_terminal);
        return false;
    }

    std::pair<std::set<std::string>::iterator,bool> ret = terminals.insert(new_terminal);

    if (ret.second == false) {
        spdlog::warn("Found duplicate definition of terminal `{}`. Ignoring second definition", new_terminal);
    }

    return true;
}

bool Language::add_nonterminal(std::string new_nonterminal) {
    if (terminals.find(new_nonterminal) != terminals.end()) {
        spdlog::error("Attempting to add terminal `{}` but it is already declared as a nonterminal", new_nonterminal);
        return false;
    }

    std::pair<std::set<std::string>::iterator,bool> ret = nonterminals.insert(new_nonterminal);

    if (ret.second == false) {
        spdlog::warn("Found duplicate definition of nonterminal `{}`. Ignoring second definition", new_nonterminal);
    } else {
        // Initialise production map for this nonterminal
        production_rules.insert({new_nonterminal, std::set<std::string>()});
    }

    return true;
}

bool Language::add_production(std::string nonterminal, std::string new_production) {
    if (start_symbol == "") {
        spdlog::trace("Inferring start symbol as `{}`", nonterminal);
        set_start_symbol(nonterminal);
    }

    if (production_rules.find(nonterminal) == production_rules.end()) {
        // nonterminal is not declared
        spdlog::error("Found production rule `{} ::= {}` for undeclared nonterminal {}. Ignoring production rule", nonterminal, new_production, nonterminal);
        return false;
    }

    // TODO: Actually parse new_production as EBNF into a meaningful structure

    std::pair<std::set<std::string>::iterator,bool> ret = production_rules.at(nonterminal).insert(new_production);

    if (ret.second == false) {
        spdlog::warn("Found duplicate definition of production `{} ::= {}`. Ignoring second definition", nonterminal, new_production);
    }

    return true;
}

bool Language::set_start_symbol(std::string new_start_symbol) {
    if (nonterminals.find(new_start_symbol) == nonterminals.end()) {
        spdlog::error("Attempting to set start symbol as `{}` but `{}` is not declared as a nonterminal", new_start_symbol, new_start_symbol);
        return false;
    }

    start_symbol = new_start_symbol;

    return true;
}

// Perform checks on the langauge definition to ensure it is valid
bool Language::validate_language() {
    // TODO: Check each nonterminal has production rules defined

    // TODO: Check for left recursion in productions

    // TODO: Check for common prefixes in productions

    // TODO: Check for first / follow conflicts in productions

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

    spdlog::info("Start symbol: `{}`", start_symbol);

    spdlog::info("Productions found:");
    for (const std::pair<std::string, std::set<std::string>>& pair : production_rules) {
        tmp = pair.first + " ::= ";

        for (const std::string production : pair.second) {
            tmp += "`" + production + "` | ";
        }

        spdlog::info(tmp);
    }
}

EBNFToken* Language::parse_ebnf(std::string& raw_ebnf, std::size_t& ebnf_index) {
    // Remove leading whitespace
    while (ebnf_index < raw_ebnf.end() && (raw_ebnf[ebnf_index] == " " || raw_ebnf[ebnf_index] == "\t")) {
        ebnf_index++;
    }

    if (ebnf_index >= raw_ebnf.end()) {
        spdlog::warn("Production `{}` is all whitespace");
        // NOTE: This also occurs if the value of ebnf_index given on function call is too big
        // TODO: Do we add a epsilon produciton here?
        return nullptr;
    }

    EBNFToken* new_token = nullptr;
    EBNFToken* child_token = nullptr;

    if (raw_ebnf[ebnf_index] == "|") {
        // Middle of OR rule
        child_token = parse_ebnf(raw_ebnf, ebnf_index++);

        if (child_token == nullptr) {
            // Do nothing
        } else {
            new_token = new EBNFToken(EBNFToken::TokenType::OR, "");
            new_token->add_child(child_token);
        }
    } else if (raw_ebnf[ebnf_index] == "{") {
        // Begining of REPEAT rule
        child_token = parse_ebnf(raw_ebnf, ebnf_index++);

        if (child_token == nullptr) {
            // Do nothing
        } else if (raw_ebnf[ebnf_index] != "}") {
            spdlog:error("Missing `}` in production rule");
            delete child_token;
        } else {
            new_token = new EBNFToken(EBNFToken::TokenType::REPEAT, "");
            new_token->add_child(child_token);
        }
    } else if (raw_ebnf[ebnf_index] == "[") {
        // Begining of OPTIONAL rule
        child_token = parse_ebnf(raw_ebnf, ebnf_index++);

        if (child_token == nullptr) {
            // Do nothing
        } else if (raw_ebnf[ebnf_index] != "]") {
            spdlog:error("Missing `]` in production rule");
            delete child_token;
        } else {
            new_token = new EBNFToken(EBNFToken::TokenType::OPTIONAL, "");
            new_token->add_child(child_token);
        }
    } else if (raw_ebnf[ebnf_index] == "(") {
        // Begining of GROUP rule
        child_token = parse_ebnf(raw_ebnf, ebnf_index++);

        if (child_token == nullptr) {
            // Do nothing
        } else if (raw_ebnf[ebnf_index] != ")") {
            spdlog:error("Missing `)` in production rule");
            delete child_token;
        } else {
            new_token = new EBNFToken(EBNFToken::TokenType::OPTIONAL, "");
            new_token->add_child(child_token);
        }
    } else {
        // Begining of an identifier (terminal or nontemrinal)
        // NOTE: This could be either of:
        //      - RHS | RHS
        //      - RHS RHS
        //      - identifier
        child_token = parse_ebnf(raw_ebnf, ebnf_index);
    }

    return new_token;
}

EBNFToken::EBNFToken(TokenType type) : type(type) {

}

EBNFToken::~EBNFToken() {
    for (const EBNFToken* child : children) {
        delete child;
    }
}

EBNFToken::TokenType EBNFToken::get_type() {
    return type;
}

void EBNFToken::set_Type(TokenType new_type) {
    type = new_type;
}

EBNFToken* EBNFToken::add_child(TokenType new_token_type) {
    EBNFToken* new_token = new EBNFToken(new_token_type);
    children.push_back(new_token);
    return new_token;
}

void EBNFToken::add_child(EBNFToken* new_child) {
    children.push_back(new_child);
}

std::list<EBNFToken*>& EBNFToken::get_children() {
    return children;
}
