#include <algorithm>
#include <fstream>
#include <list>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "COMP3931Grammar.hpp"
#include "spdlog/spdlog.h"

using namespace ParserGenreator;

/*
 * EBNFToken Class
 */

EBNFToken::EBNFToken(TokenType type, std::string value) : type(type), value(value) {

}

EBNFToken::~EBNFToken() {
    for (const EBNFToken* child : children) {
        delete child;
    }
}

EBNFToken::TokenType EBNFToken::get_type() {
    return type;
}

std::string EBNFToken::get_value() {
    return value;
}

void EBNFToken::add_child(EBNFToken* new_child) {
    if (new_child == nullptr || new_child == NULL) {
        return;
    }

    children.push_back(new_child);
}

std::vector<EBNFToken*>& EBNFToken::get_children() {
    return children;
}

std::string EBNFToken::to_string() const {
    std::string printable_value = "";

    switch(type) {
        case TokenType::SEQUENCE:
            for (const EBNFToken* child : children) {
                printable_value += child->to_string() + " ";
            }
            break;
        case TokenType::TERMINAL:
            printable_value = value;
            break;
        case TokenType::NONTERMINAL:
            printable_value = value;
            break;
        case TokenType::OR:
            for (const EBNFToken* child : children) {
                printable_value += child->to_string() + " | ";
            }

            break;
        case TokenType::REPEAT:
            printable_value += "{ ";

            for (const EBNFToken* child : children) {
                printable_value += child->to_string() + " ";
            }

            printable_value += " }";
            break;
        case TokenType::OPTIONAL:
            printable_value += "[ ";

            for (const EBNFToken* child : children) {
                printable_value += child->to_string() + " ";
            }

            printable_value += " ]";
            break;
        case TokenType::GROUP:
            printable_value += "( ";

            for (const EBNFToken* child : children) {
                printable_value += child->to_string() + " ";
            }

            printable_value += " )";
            break;
        default:
            spdlog::error("Unkown type of EBNFToken");
            printable_value = "";
    }

    return printable_value;
}

/*
 * Grammar Class
 */

Grammar::Grammar() {

}

Grammar::~Grammar() {
    for (const std::pair<std::string, EBNFToken*>& production : production_rules) {
        if (production.second != nullptr) {
            delete production.second;
        }
    }
}

bool Grammar::input_language_from_file(std::string file_path) {
    std::ifstream input_file(file_path);

    if (input_file.is_open()) {
        spdlog::trace("Opened file {} for parsing as a grammar definition", file_path);

        file_parse_INPUT_FILE(input_file);

        return true;
    } else {
        spdlog::error("Cannot open input file: {}", file_path);
        return false;
    }
}

bool Grammar::add_terminal(std::string new_terminal) {
    if (new_terminal == "eof") {
        spdlog:error("Attempting to add terminal `{}` not allowed. Please see README.md section `Non-Allowed symbols`", new_terminal);
        return false;
    }

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

bool Grammar::add_nonterminal(std::string new_nonterminal) {
    if (new_nonterminal == "eof") {
        spdlog:error("Attempting to add nonterminal `{}` not allowed. Please see README.md section `Non-Allowed symbols`", new_nonterminal);
        return false;
    }

    if (terminals.find(new_nonterminal) != terminals.end()) {
        spdlog::error("Attempting to add nonterminal `{}` but it is already declared as a nonterminal", new_nonterminal);
        return false;
    }

    std::pair<std::set<std::string>::iterator,bool> ret = nonterminals.insert(new_nonterminal);

    if (ret.second == false) {
        spdlog::warn("Found duplicate definition of nonterminal `{}`. Ignoring second definition", new_nonterminal);
    } else {
        // Initialise production map for this nonterminal
        production_rules.insert({new_nonterminal, nullptr});
    }

    return true;
}

bool Grammar::add_production(std::string nonterminal, EBNFToken* new_production) {
    if (start_symbol == "") {
        spdlog::trace("Inferring start symbol as `{}`", nonterminal);
        set_start_symbol(nonterminal);
    }

    // Check nontemrinal is declared in production_rules table
    std::unordered_map<std::string, EBNFToken*>::iterator current_nt = production_rules.find(nonterminal);

    if (current_nt == production_rules.end()) {
        spdlog::error("Attempting to add production for non-existant nonterminal `{}`", nonterminal);
        return false;
    }

    // Add new_production to production_rules
    if (current_nt->second == nullptr) {
        current_nt->second = new_production;
    } else {
        spdlog::warn("Productions for nonterminal `{}` already defined ignoring second set of productions.", nonterminal);
    }

    return true;
}

bool Grammar::set_start_symbol(std::string new_start_symbol) {
    if (nonterminals.find(new_start_symbol) == nonterminals.end()) {
        spdlog::error("Attempting to set start symbol as `{}` but `{}` is not declared as a nonterminal", new_start_symbol, new_start_symbol);
        return false;
    }

    start_symbol = new_start_symbol;

    return true;
}

bool Grammar::is_terminal(std::string to_find) {
    return !(terminals.find(to_find) == terminals.end());
}

bool Grammar::is_nonterminal(std::string to_find) {
    return !(nonterminals.find(to_find) == nonterminals.end());
}

// Determine if the grammar, in its current state, would produce a valid LL(1) parser
bool Grammar::can_produce_ll_parser() {
    bool first_set_success = calculate_first_set();
    bool follow_set_success = calculate_follow_set();

    return first_set_success && follow_set_success;
}

void Grammar::log_grammar() {
    // Log terminals
    std::string tmp;
    for (const std::string terminal : terminals) {
        tmp += "`" + terminal + "`, ";
    }
    spdlog::info("Terminals found: {}", tmp);

    // Log nonterminals
    tmp = "";
    for (const std::string nonterminal : nonterminals) {
        tmp += "`" + nonterminal + "`, ";
    }
    spdlog::info("Nonterminals found: {}", tmp);

    // Log start symbol
    spdlog::info("Start symbol: `{}`", start_symbol);

    // Log productions
    spdlog::info("Production Rules:");

    for (std::pair<std::string, EBNFToken*> production : production_rules) {
        if (production.second != nullptr) {
            spdlog::info("{} ::= {}", production.first, production.second->to_string());
        } else {
            spdlog::info("{} ::=", production.first);
        }
    }

    // Log first set
    spdlog::info("First sets:");

    for (std::pair<std::string, std::set<std::string>> first_set : first_sets) {
        std::string first_set_list = "";

        for (std::string terminal : first_set.second) {
            first_set_list += terminal;
            first_set_list += ", ";
        }

        spdlog::info("First({}) = {}", first_set.first, first_set_list);
    }

    // Log follow set
    spdlog::info("Follow sets:");

    for (std::pair<std::string, std::set<std::string>> follow_set : follow_sets) {
        std::string follow_set_list = "";

        for (std::string terminal : follow_set.second) {
            follow_set_list += terminal;
            follow_set_list += ", ";
        }

        spdlog::info("Follow({}) = {}", follow_set.first, follow_set_list);
    }
}

bool Grammar::file_parse_INPUT_FILE(std::ifstream& input) {
    spdlog::info("Parsing terminals");
    spdlog::trace("Parsing INPUT_FILE");

    if (!file_parse_TERM_DECLAR(input)) {
        return false;
    }

    char c;
    if (input.get(c)) {
        if (c == '\r') {
            // NOTE: Windows line endings
            if (!input.get(c)) {
                spdlog::error("Error reading file after parsing terminal declaration at position {}", input.tellg());
                return false;
            }

            if (c != '\n') {
                spdlog::error("Expected newline character at end of terminal declaration but found `{}` at position {}", c, input.tellg());
                return false;
            }
        }
    } else {
        spdlog::error("Error reading file after parsing terminal declaration at position {}", input.tellg());
        return false;
    }

    spdlog::info("Parsing nonterminals");

    if (!file_parse_NONTERM_DECLAR(input)) {
        return false;
    }

    if (input.get(c)) {
        if (c == '\r') {
            // NOTE: Windows line endings
            if (!input.get(c)) {
                spdlog::error("Error reading file after parsing nonterminal declaration at position {}", input.tellg());
                return false;
            }

            if (c != '\n') {
                spdlog::error("Expected newline character at end of nonterminal declaration but found `{}` at position {}", c, input.tellg());
                return false;
            }
        }
    } else {
        spdlog::error("Error reading file after parsing nonterminal declaration at position {}", input.tellg());
        return false;
    }

    spdlog::info("Parsing productions");

    if (!file_parse_GRAM_DECLAR(input)) {
        return false;
    }

    return true;
}

bool Grammar::file_parse_TERM_DECLAR(std::ifstream& input) {
    spdlog::trace("Parsing TERM_DECLAR");

    char c;
    int i;

    if (!file_parse_check_char(input, 'T')) {
        return false;
    }

    if (!file_parse_check_char(input, ':')) {
        return false;
    }

    std::string new_terminal;

    do {
        new_terminal = "";

        if (!file_parse_skip_white_space(input)) {
            return false;
        }

        i = input.peek();
        if (i == EOF) {
            spdlog::error("Error reading file while parsing terminal declaration at position {}", input.tellg());
            return false;
        } else if (i == '\r' || i == '\n') {
            // End of terminal list
            return true;
        } else if (i == ',') {
            input.get(c);
        }

        if (!file_parse_TERMINAL(input, new_terminal)) {
            return false;
        } else {
            spdlog::trace("Found terminal `{}`", new_terminal);
            add_terminal(new_terminal);
        }
    } while(true);
}

bool Grammar::file_parse_NONTERM_DECLAR(std::ifstream& input) {
    spdlog::trace("Parsing NONTERM_DECLAR");

    char c;
    int i;

    if (!file_parse_check_char(input, 'N')) {
        return false;
    }

    if (!file_parse_check_char(input, 'T')) {
        return false;
    }

    if (!file_parse_check_char(input, ':')) {
        return false;
    }

    std::string new_nonterminal;

    do {
        new_nonterminal = "";

        if (!file_parse_skip_white_space(input)) {
            return false;
        }

        i = input.peek();
        if (i == EOF) {
            spdlog::error("Error reading file while parsing nonterminal declaration at position {}", input.tellg());
            return false;
        } else if (i == '\r' || i == '\n') {
            // End of nonterminal list
            return true;
        } else if (i == ',') {
            input.get(c);
        }

        if (!file_parse_TERMINAL(input, new_nonterminal)) {
            return false;
        } else {
            spdlog::trace("Found nonterminal `{}`", new_nonterminal);
            add_nonterminal(new_nonterminal);
        }
    } while(true);
}

bool Grammar::file_parse_TERMINAL(std::ifstream& input, std::string& terminal) {
    spdlog::trace("Parsing TERMINAL");

    if (!file_parse_skip_white_space(input)) {
        return false;
    }

    char c;
    int i = input.peek();
    bool escaped_char = false;

    while (i != EOF && i != ' ' && i != '\r' && i != '\n') {
        // Check for escaped characters
        if (!escaped_char && i == '\\') {
            escaped_char = true;

            if (!file_parse_check_char(input, '\\')) {
                return false;
            }
        } else if (escaped_char && (i == ',' || i == '{' || i == '}' || i == '[' || i == ']' || i == '(' || i == ')' || i == '\\' || i == '|')) {
            // One of the escaped character
            escaped_char = false;

            if (!input.get(c)) {
                spdlog::error("Error reading file while parsing terminal at position {}", input.tellg());
                return false;
            }

            terminal += c;
        } else if (i == ',' || i == '{' || i == '}' || i == '[' || i == ']' || i == '(' || i == ')' || i == '\\' || i == '|') {
            // These characters can only appear when escaped, which it wasn't so must be the end of the terminal
            return true;
        } else {
            // Normal character
            if (!input.get(c)) {
                spdlog::error("Error reading file while parsing terminal at position {}", input.tellg());
                return false;
            }

            if (escaped_char) {
                spdlog::error("Unknown escaped sequence `\\{}` at position {}", c, input.tellg());
                return false;
            }

            terminal += c;
        }

        i = input.peek();
    }

    return true;
}

bool Grammar::file_parse_GRAM_DECLAR(std::ifstream& input) {
    spdlog::trace("Parsing GRAM_DECLAR");

    int i;

    if (!file_parse_check_char(input, 'P')) {
        return false;
    }

    if (!file_parse_check_char(input, ':')) {
        return false;
    }

    if (!file_parse_skip_white_space(input)) {
        return false;
    }

    i = input.peek();
    if (i == EOF) {
        // No production rules defined
        spdlog::warn("No production rules defined");
        return true;
    } else if (!(i == '\r' || i == '\n')) {
        spdlog::error("Expected newline character for start of productions declaration but found `{}` at position {}", static_cast<char>(i), input.tellg());
        return false;
    }

    if (!file_parse_end_of_line(input)) {
        return false;
    }

    i = input.peek();

    while (i != EOF) {
        if (!file_parse_skip_white_space(input)) {
            return false;
        }

        if (!file_parse_PRODUCTION(input)) {
            return false;
        }

        if (!file_parse_end_of_line(input)) {
            return false;
        }

        i = input.peek();
    }

    return true;
}

bool Grammar::file_parse_PRODUCTION(std::ifstream& input) {
    spdlog::trace("Parsing PRODUCTION");

    std::string new_lhs;
    EBNFToken* new_rhs = nullptr;

    if (!file_parse_LHS(input, new_lhs)) {
        return false;
    }

    spdlog::trace("LHS of production `{}`", new_lhs);

    if (!is_nonterminal(new_lhs)) {
        spdlog::error("Production for undelcared nonterminal `{}` at position", new_lhs, input.tellg());
        return false;
    }

    if (!file_parse_skip_white_space(input)) {
        return false;
    }

    if (!file_parse_check_char(input, ':')) {
        return false;
    }

    if (!file_parse_check_char(input, ':')) {
        return false;
    }

    if (!file_parse_check_char(input, '=')) {
        return false;
    }

    if (!file_parse_skip_white_space(input)) {
        return false;
    }

    new_rhs = new EBNFToken(EBNFToken::TokenType::SEQUENCE, "");

    if (!file_parse_RHS(input, new_rhs)) {
        delete new_rhs;
        return false;
    }

    spdlog::trace("Found production `{} ::= {}`", new_lhs, new_rhs->to_string());

    return add_production(new_lhs, new_rhs);
}

bool Grammar::file_parse_LHS(std::ifstream& input, std::string& new_lhs) {
    spdlog::trace("Parsing LHS");

    return file_parse_TERMINAL(input, new_lhs);
}

bool Grammar::file_parse_RHS(std::ifstream& input, EBNFToken* new_rhs) {
    spdlog::trace("Parsing RHS");

    if (!file_parse_skip_white_space(input)) {
        return false;
    }

    EBNFToken* new_token = new EBNFToken(EBNFToken::TokenType::OR, "");

    if (!file_parse_TERM(input, new_token)) {
        delete new_token;
        return false;
    }

    if (!file_parse_skip_white_space(input)) {
        delete new_token;
        return false;
    }

    int i = input.peek();

    // QUESTION: Shouldn't this be while i == '|'
    while (i == '|') {
        if (!file_parse_check_char(input, '|')) {
            delete new_token;
            return false;
        }

        if (!file_parse_TERM(input, new_token)) {
            delete new_token;
            return false;
        }

        if (!file_parse_skip_white_space(input)) {
            delete new_token;
            return false;
        }

        i = input.peek();
    }

    std::vector<EBNFToken*>& children = new_token->get_children();

    if (children.size() == 1) {
        new_rhs->add_child(children.front());
        children.pop_back();
        delete new_token;
    } else {
        new_rhs->add_child(new_token);
    }

    return true;
}

bool Grammar::file_parse_TERM(std::ifstream& input, EBNFToken* new_rhs) {
    spdlog::trace("Parsing TERM");

    if (!file_parse_skip_white_space(input)) {
        return false;
    }

    EBNFToken* new_token = new EBNFToken(EBNFToken::TokenType::SEQUENCE, "");

    if (!file_parse_FACTOR(input, new_token)) {
        delete new_token;
        return false;
    }

    if (!file_parse_skip_white_space(input)) {
        delete new_token;
        return false;
    }

    int i = input.peek();

    while (i != EOF && i != ' ' && i != '\r' && i != '\n' && i != '|' && i != ')' && i != ']' && i != '}') {
        if (!file_parse_FACTOR(input, new_token)) {
            delete new_token;
            return false;
        }

        if (!file_parse_skip_white_space(input)) {
            delete new_token;
            return false;
        }

        i = input.peek();
    }

    new_rhs->add_child(new_token);

    return true;
}

bool Grammar::file_parse_FACTOR(std::ifstream& input, EBNFToken* new_rhs) {
    spdlog::trace("Parsing FACTOR");

    EBNFToken* new_token = nullptr;

    if (!file_parse_skip_white_space(input)) {
        return false;
    }

    int i = input.peek();

    if (i == EOF) {
        return true;
    } else if (i == '[') {
        spdlog::trace("FACTOR - OPTIONAL");

        if (!file_parse_check_char(input, '[')) {
            return false;
        }

        if (!file_parse_skip_white_space(input)) {
            return false;
        }

        new_token = new EBNFToken(EBNFToken::TokenType::OPTIONAL, "");

        if (!file_parse_RHS(input, new_token)) {
            delete new_token;
            return false;
        }

        if (!file_parse_skip_white_space(input)) {
            delete new_token;
            return false;
        }

        if (!file_parse_check_char(input, ']')) {
            delete new_token;
            return false;
        }

        new_rhs->add_child(new_token);
    } else if (i == '{') {
        spdlog::trace("FACTOR - REPEAT");

        if (!file_parse_check_char(input, '{')) {
            return false;
        }

        if (!file_parse_skip_white_space(input)) {
            return false;
        }

        new_token = new EBNFToken(EBNFToken::TokenType::REPEAT, "");

        if (!file_parse_RHS(input, new_token)) {
            delete new_token;
            return false;
        }

        if (!file_parse_skip_white_space(input)) {
            delete new_token;
            return false;
        }

        if (!file_parse_check_char(input, '}')) {
            delete new_token;
            return false;
        }

        new_rhs->add_child(new_token);
    } else if (i == '(') {
        spdlog::trace("FACTOR - GROUP");

        if (!file_parse_check_char(input, '(')) {
            return false;
        }

        if (!file_parse_skip_white_space(input)) {
            return false;
        }

        new_token = new EBNFToken(EBNFToken::TokenType::GROUP, "");

        if (!file_parse_RHS(input, new_token)) {
            delete new_token;
            return false;
        }

        if (!file_parse_skip_white_space(input)) {
            delete new_token;
            return false;
        }

        if (!file_parse_check_char(input, ')')) {
            delete new_token;
            return false;
        }

        new_rhs->add_child(new_token);
    } else {
        std::string new_token_value = "";

        if (!file_parse_TERMINAL(input, new_token_value)) {
            return false;
        }

        if (!file_parse_skip_white_space(input)) {
            return false;
        }

        spdlog::trace("FACTOR TERMINAL: {}", new_token_value);

        if (is_terminal(new_token_value)) {
            new_token = new EBNFToken(EBNFToken::TokenType::TERMINAL, new_token_value);
            new_rhs->add_child(new_token);
        } else if (is_nonterminal(new_token_value)) {
            new_token = new EBNFToken(EBNFToken::TokenType::NONTERMINAL, new_token_value);
            new_rhs->add_child(new_token);
        } else {
            spdlog::error("Value '{}' used in production is neither a terminal or nonterminal", new_token_value);
            return false;
        }
    }

    return true;
}

bool Grammar::file_parse_skip_white_space(std::ifstream& input) {
    int i = input.peek();
    char c;

    while (i != EOF && i == ' ') {
        input.get(c);
        i = input.peek();
    }

    return true;
}

bool Grammar::file_parse_end_of_line(std::ifstream& input) {
    if (!file_parse_skip_white_space(input)) {
        return false;
    }

    int i = input.peek();
    char c;

    if (i == '\r') {
        // Windows line ending =(
        if (!input.get(c)) {
            spdlog::error("Error reading file at position {}", input.tellg());
            return false;
        }

        if (c != '\r') {
            spdlog::error("Expected return character but found `{}` at position {}", c, input.tellg());
            return false;
        }
    }

    if (!input.get(c)) {
        spdlog::error("Error reading file at position {}", input.tellg());
        return false;
    }

    if (c != '\n') {
        spdlog::error("Expected newline character but found `{}` at position {}", c, input.tellg());
        return false;
    }

    return true;
}

bool Grammar::file_parse_check_char(std::ifstream& input, char character) {
    char c;

    if (!input.get(c)) {
        spdlog::error("Error reading file at position {}", input.tellg());
        return false;
    }

    if (c != character) {
        spdlog::error("Expected `{}` but found `{}` at position {}", character, c, input.tellg());
        return false;
    }

    return true;
}

bool Grammar::calculate_first_set() {
    spdlog::trace("Calculating first set");
    // First(terminal) = {terminal}
    for (std::string terminal : terminals) {
        first_sets.insert({terminal, std::set<std::string>({terminal})});
    }

    // Initlaise empty sets for each of the non-terminals
    for (std::string nonterminal : nonterminals) {
        first_sets.insert({nonterminal, std::set<std::string>()});
    }

    // Compute the First sets
    bool sets_have_changed = true;

    while (sets_have_changed) {
        sets_have_changed = false;

        for (std::pair<std::string, EBNFToken*> ebnf_production : production_rules) {
            std::string production_lhs = ebnf_production.first;
            EBNFToken* productions = ebnf_production.second;

            if (productions == nullptr) {
                continue;
            }

            std::unordered_map<std::string, std::set<std::string>>::iterator old_first_set_it = first_sets.find(production_lhs);
            if (old_first_set_it == first_sets.end()) {
                spdlog::error("Error looking up first set for nonterminal `{}`", production_lhs);
                return false;
            }
            std::set<std::string>& old_first_set = old_first_set_it->second;

            std::set<std::string> new_first_set = calculate_first_terminal(productions);

            // Check if the new first set is the same as the old one
            for (std::string terminal : new_first_set) {
                if (old_first_set.count(terminal) != 1) {
                    sets_have_changed = true;
                    old_first_set.insert(terminal);
                }
            }
        }
    }

    return true;
}

// Calculate the terminals to appear in the first set for nonterminal using its EBNF production tree
// NOTE: This function assumes that the EBNF production tree is valid. There is little to no error checking
std::set<std::string> Grammar::calculate_first_terminal(EBNFToken* ebnf_token) {
    std::set<std::string> local_first_set;

    if (ebnf_token == nullptr) {
        spdlog::error("Internal error. EBNF parser tree invalid while computing first set");
        return local_first_set;
    }

    spdlog::trace("Calculating terminals from `{}` for first set", ebnf_token->to_string());

    std::vector<EBNFToken*>& ebnf_token_children = ebnf_token->get_children();

    switch (ebnf_token->get_type()) {
        case EBNFToken::TokenType::SEQUENCE: {
            std::set<std::string> tmp_set = calculate_first_terminal(ebnf_token_children[0]);
            local_first_set.insert(tmp_set.begin(), tmp_set.end());

            if (tmp_set.count("EPSILON") == 1) {
                // We copied the entire tmp_set to local_first_set but shouldn't have copied EPSILON, if it existed
                local_first_set.erase("EPSILON");
            }

            bool contains_epsilon = tmp_set.count("EPSILON") == 1;
            int i = 0;

            while (contains_epsilon && (i < ebnf_token_children.size() - 1)) {
                tmp_set = calculate_first_terminal(ebnf_token_children[i + 1]);
                local_first_set.insert(tmp_set.begin(), tmp_set.end());

                if (tmp_set.count("EPSILON") == 1) {
                    // We copied the entire tmp_set to local_first_set but shouldn't have copied EPSILON, if it existed
                    local_first_set.erase("EPSILON");
                }

                i++;
                contains_epsilon = tmp_set.count("EPSILON") == 1;
            }

            tmp_set = calculate_first_terminal(ebnf_token_children[ebnf_token_children.size() - 1]);
            if (i == ebnf_token_children.size() - 1 && tmp_set.count("EPSILON") == 1) {
                local_first_set.insert("EPSILON");
            }
        }
        break;
        case EBNFToken::TokenType::TERMINAL:
            local_first_set.insert(ebnf_token->get_value());
            break;
        case EBNFToken::TokenType::NONTERMINAL: {
            // Join the sets of local_first_set and first(NONTERMINAL)
            std::unordered_map<std::string, std::set<std::string>>::iterator current_token_first_set_it = first_sets.find(ebnf_token->get_value());

            if (current_token_first_set_it == first_sets.end()) {
                spdlog::error("Error calculating first sets. Couldn't find first set for `{}`", ebnf_token->get_value());
                return local_first_set;
            }

            local_first_set.insert(current_token_first_set_it->second.begin(), current_token_first_set_it->second.end());
        }
        break;
        case EBNFToken::TokenType::OR:
            for (EBNFToken* new_ebnf_token : ebnf_token_children) {
                std::set<std::string> tmp_set = calculate_first_terminal(new_ebnf_token);
                local_first_set.insert(tmp_set.begin(), tmp_set.end());
            }
            break;
        case EBNFToken::TokenType::REPEAT:
        case EBNFToken::TokenType::OPTIONAL:
            local_first_set = calculate_first_terminal(ebnf_token_children[0]);
            local_first_set.insert("EPSILON");    // Optional and Repeat may become the empty string
            break;
        case EBNFToken::TokenType::GROUP:
            local_first_set = calculate_first_terminal(ebnf_token_children[0]);
            break;
        default:
            spdlog::error("Unkown type of EBNFToken when calculating terminals for first set");
    }

    std::string first_set_list = "";

    for (std::string terminal : local_first_set) {
        first_set_list += terminal;
        first_set_list += ", ";
    }

    spdlog::trace("First({}) = {}", ebnf_token->to_string(), first_set_list);

    return local_first_set;
}

bool Grammar::calculate_follow_set() {
    spdlog::trace("Calculating follow set");

    // Initlaise empty sets for each of the non-terminals
    for (std::string nonterminal : nonterminals) {
        follow_sets.insert({nonterminal, std::set<std::string>()});
    }

    // Follow(S) = eof
    follow_sets.find(start_symbol)->second.insert("eof");

    // Compute the Follow sets
    bool sets_have_changed = true;

    while (sets_have_changed) {
        sets_have_changed = false;

        for (std::pair<std::string, EBNFToken*> ebnf_production : production_rules) {
            std::string production_lhs = ebnf_production.first;
            EBNFToken* productions = ebnf_production.second;

            if (productions == nullptr) {
                continue;
            }

            // Get the follow set of A
            std::unordered_map<std::string, std::set<std::string>>::iterator lhs_follow_set_it = follow_sets.find(production_lhs);
            if (lhs_follow_set_it == follow_sets.end()) {
                spdlog::error("Error looking up follow set for nonterminal `{}`", production_lhs);
                return false;
            }
            std::set<std::string> lhs_follow_set = lhs_follow_set_it->second;

            spdlog::trace("Updating follow sets from production `{} ::= {}`", production_lhs, productions->to_string());

            std::vector<std::set<std::string>> trailer = {};
            trailer.push_back(lhs_follow_set);

            bool did_child_change_sets = calculate_follow_terminal(production_lhs, productions, trailer);

            if (did_child_change_sets == true) {
                sets_have_changed = true;
            }
        }
    }

    return true;
}

// Update the Follow sets from a production (or part of a production)
bool Grammar::calculate_follow_terminal(std::string production_lhs, EBNFToken* ebnf_token, std::vector<std::set<std::string>>& current_trailers) {
    if (ebnf_token == nullptr) {
        spdlog::error("Internal error. EBNF parser tree invalid while computing follow set");
        return false;
    }

    std::vector<EBNFToken*>& ebnf_token_children = ebnf_token->get_children();
    bool has_changed_sets = false;

    switch (ebnf_token->get_type()) {
        case EBNFToken::TokenType::SEQUENCE:
        {
            for (int i = ebnf_token_children.size() - 1; i >= 0; i--) {
                bool did_child_change_sets = calculate_follow_terminal(production_lhs, ebnf_token_children[i], current_trailers);

                if (did_child_change_sets == true) {
                    has_changed_sets = true;
                }
            }
        }
            break;
        case EBNFToken::TokenType::TERMINAL:
        {
            // Get the first set of ebnf_token
            std::unordered_map<std::string, std::set<std::string>>::iterator terminal_first_set_it = first_sets.find(ebnf_token->get_value());
            if (terminal_first_set_it == first_sets.end()) {
                spdlog::error("Error looking up first set for terminal `{}`", ebnf_token->to_string());
                return false;
            }

            // Remove all other trailers and set trailer to the first set of the terminal
            current_trailers.clear();
            current_trailers.push_back(terminal_first_set_it->second);
        }
            break;
        case EBNFToken::TokenType::NONTERMINAL:
        {
            // Get the follow set of NONTERMINAL
            std::unordered_map<std::string, std::set<std::string>>::iterator nonterminal_follow_set_it = follow_sets.find(ebnf_token->get_value());
            if (nonterminal_follow_set_it == follow_sets.end()) {
                spdlog::error("Error looking up follow set for nonterminal `{}`", ebnf_token->to_string());
                return false;
            }
            std::set<std::string>& nonterminal_follow_set = nonterminal_follow_set_it->second;

            // Add the current trailer set to the nonterminal follow set
            // NOTE: We do this item by item manually so we can tell if the Follow set was updated (i.e. a new element was added)
            for (std::set<std::string> current_trailer : current_trailers) {
                for (std::string s : current_trailer) {
                    spdlog::trace("Adding `{}` to Follow({})", s, ebnf_token->to_string());
                    std::pair<std::set<std::string>::iterator, bool> insert_status = nonterminal_follow_set.insert(s);

                    if (insert_status.second == true) {
                        // A new item was inserted, hence the set changed
                        has_changed_sets = true;
                    }
                }
            }

            // Get the first set of NONTERMINAL
            std::unordered_map<std::string, std::set<std::string>>::iterator nonterminal_first_set_it = first_sets.find(ebnf_token->get_value());
            if (nonterminal_first_set_it == first_sets.end()) {
                spdlog::error("Error looking up first set for nonterminal `{}`", ebnf_token->to_string());
                return false;
            }
            std::set<std::string>& nonterminal_first_set = nonterminal_first_set_it->second;

            if (nonterminal_first_set.count("EPSILON") == 1) {
                // Add the first set of the nonterminal (minus epsilon) to each trailer
                for (std::set<std::string>& current_trailer : current_trailers) {
                    current_trailer.insert(nonterminal_first_set.begin(), nonterminal_first_set.end());
                    current_trailer.erase("EPSILON");       // EPSILON shouldn't have been copied
                }
            } else {
                // Remove all other trailers and set trailer to the first set of the nonterminal
                current_trailers.clear();
                current_trailers.push_back(nonterminal_first_set);
            }
        }
            break;
        case EBNFToken::TokenType::OR:
        {
            std::vector<std::set<std::string>> original_trailers = current_trailers;

            for (int i = ebnf_token_children.size() - 1; i >= 0; i--) {
                std::vector<std::set<std::string>> new_trailers = original_trailers;
                bool did_child_change_sets = calculate_follow_terminal(production_lhs, ebnf_token_children[i], new_trailers);

                if (did_child_change_sets == true) {
                    has_changed_sets = true;
                }

                current_trailers.insert(current_trailers.end(), new_trailers.begin(), new_trailers.end());
            }
        }
            break;
        case EBNFToken::TokenType::REPEAT:
        {
            std::vector<std::set<std::string>> original_trailers = current_trailers;

            for (int i = ebnf_token_children.size() - 1; i >= 0; i--) {
                std::vector<std::set<std::string>> new_trailers = original_trailers;
                bool did_child_change_sets = calculate_follow_terminal(production_lhs, ebnf_token_children[i], new_trailers);

                if (did_child_change_sets == true) {
                    has_changed_sets = true;
                }

                current_trailers.insert(current_trailers.end(), new_trailers.begin(), new_trailers.end());
            }
        }
            break;
        case EBNFToken::TokenType::OPTIONAL:
        {
            std::vector<std::set<std::string>> original_trailers = current_trailers;

            for (int i = ebnf_token_children.size() - 1; i >= 0; i--) {
                std::vector<std::set<std::string>> new_trailers = original_trailers;
                bool did_child_change_sets = calculate_follow_terminal(production_lhs, ebnf_token_children[i], new_trailers);

                if (did_child_change_sets == true) {
                    has_changed_sets = true;
                }

                current_trailers.insert(current_trailers.end(), new_trailers.begin(), new_trailers.end());
            }
        }
            break;
        case EBNFToken::TokenType::GROUP:
        {
            std::vector<std::set<std::string>> original_trailers = current_trailers;

            // Check if group can become epsilon
            std::set<std::string> tmp_first_set = calculate_first_terminal(ebnf_token);
            if (tmp_first_set.count("EPSILON") == 0) {
                // The group cannot become epsilon. Hence, the trailer after the group is finished should not contain the trailers from before the group
                current_trailers.clear();
            }

            for (int i = ebnf_token_children.size() - 1; i >= 0; i--) {
                std::vector<std::set<std::string>> new_trailers = current_trailers;
                bool did_child_change_sets = calculate_follow_terminal(production_lhs, ebnf_token_children[i], new_trailers);

                if (did_child_change_sets == true) {
                    has_changed_sets = true;
                }

                current_trailers.insert(current_trailers.end(), new_trailers.begin(), new_trailers.end());
            }
        }
            break;
        default:
            spdlog::error("Unkown type of EBNFToken when calculating follow set");
    }

    return has_changed_sets;
}
