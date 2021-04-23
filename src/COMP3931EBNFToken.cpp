#include <string>
#include <vector>

#include "COMP3931EBNFToken.hpp"
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
