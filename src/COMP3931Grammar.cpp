#include <fstream>
#include <map>
#include <set>
#include <string>

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

std::list<EBNFToken*>& EBNFToken::get_children() {
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
    if (terminals.find(new_nonterminal) != terminals.end()) {
        spdlog::error("Attempting to add terminal `{}` but it is already declared as a nonterminal", new_nonterminal);
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

void Grammar::log_language() {
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

    log_language();

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

    std::list<EBNFToken*>& children = new_token->get_children();

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