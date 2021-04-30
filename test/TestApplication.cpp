#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "TestApplication.hpp"
#include "JACKCompiler.hpp"

// Declare all of the keyword lists
const std::vector<std::string> CustomJACKLexer::constructKeywords = {"class", "constructor", "method", "function"};
const std::vector<std::string> CustomJACKLexer::typeKeywords = {"int", "boolean", "char", "void"};
const std::vector<std::string> CustomJACKLexer::variableKeywords = {"var", "let", "static", "field"};
const std::vector<std::string> CustomJACKLexer::controlKeywords = {"do", "if", "else", "while", "return"};
const std::vector<std::string> CustomJACKLexer::referenceKeywords = {"this"};
const std::vector<std::string> CustomJACKLexer::valueKeywords = {"true", "false", "null"};

CustomJACKLexer::CustomJACKLexer(std::string file_name) : file_name(file_name), token_list_head(nullptr), token_list_tail(nullptr), next_token(nullptr) {
    scan_file();
}

CustomJACKLexer::~CustomJACKLexer() {
    // Destroy list of tokens
    if (token_list_head != nullptr) {
        CustomLexerToken* token = token_list_head;
        CustomLexerToken* next_token = token_list_head->next;
        while (token != nullptr) {
            // std::cout << "KILL: " << token->get_lexeme() << std::endl;
            delete token;
            token = next_token;
            if (next_token != nullptr) {
                next_token = next_token->next;
            }
        }
    }
}

GeneratedParser::LexerToken& CustomJACKLexer::get_next_token() {
    CustomLexerToken* old_next = next_token;
    if (next_token != nullptr) {
        next_token = next_token->next;
        return *old_next;
    }
    return *token_list_tail;
}

GeneratedParser::LexerToken& CustomJACKLexer::peak_next_token() {
    return *next_token;
}

void CustomJACKLexer::scan_file() {
    std::fstream file(file_name, std::fstream::in);

    if(!file.is_open()) {
        throw FileNotFoundException("Unable to open " + file_name);
    }

    char c;
    std::string buffer;
    int lineNumber = 1;
    int charPos = 0;

    while(file.get(c)) {
        charPos++;

        // Check if character is whitespace
        if(c == ' ' || c == '\t' || c == '\r') {
            continue;
        } else if(c == '\n') {
            lineNumber++;
            charPos = 0;
        }

        // Check if character is a comment
         else if(c == '/') {
            char c2 = file.peek();

            if(c2 == '/') {
                // Single line comment
                while(file.get(c2)) {
                    charPos++;

                    if(c2 == '\n') {
                        lineNumber++;
                        charPos = 0;
                        break;
                    }
                }
                continue;
            } else if(c2 == '*') {
                // Multi-line comment
                while(file.get(c2)) {
                    charPos++;

                    if(c2 == '*') {
                        file.get(c2);
                        charPos++;

                        if(c2 == '/') {
                            // End of comment
                            break;
                        }
                    } else if(c2 == '\n') {
                        lineNumber++;
                        charPos = 0;
                    }
                }
                continue;
            } else {
                // A random / symbol
                buffer += c;
                add_new_token(buffer, "MATH_OPERATOR_SYMBOL", lineNumber, charPos);
                buffer.clear();
                continue;
            }
        }

        // Check if character is a "
        else if(c == '"') {
            int startingLineNumber = lineNumber;
            int startingCharPos = charPos;

            while(file.get(c)) {
                charPos++;

                if(c == '"') {
                    break;
                } else if(c == '\n') {
                    lineNumber++;
                    charPos = 0;
                }
                buffer += c;
            }

            if(file.eof()) {
                // Unexpected end of file
                // "(" + filename + ") line:" + std::to_string(lineNumber) + " pos:" + std::to_string(startingCharacterPosition)
                throw UnexpectedEndOfFileException("(" + file_name + ") line:" + std::to_string(startingLineNumber) + " pos:" + std::to_string(startingCharPos) + " Lexer error: Unexpected end of file while scanning string");
            } else {
                add_new_token(buffer, "STRING_LITERAL", startingLineNumber, startingCharPos);
                buffer.clear();
            }

            continue;
        }

        // Check if character is a letter or _ (start of identifier or keyword)
        else if(isalpha(c) || c == '_') {
            buffer += c;

            int c2 = file.peek();
            int startingCharPos = charPos;

            while(isdigit(c2) || isalpha(c2) || c2 == '_') {
                file.get(c);
                charPos++;
                buffer += c;
                c2 = file.peek();
            }

            add_new_token(buffer, get_keyword_type(buffer), lineNumber, startingCharPos);
            buffer.clear();
        }

        // Check if character is a digit
        else if(isdigit(c)) {
            buffer += c;

            int c2 = file.peek();
            int startingCharPos = charPos;

            while(isdigit(c2)) {
                file.get(c);
                charPos++;
                buffer += c;
                c2 = file.peek();
            }

            add_new_token(buffer, "NUMERIC_CONSTANT", lineNumber, startingCharPos);
            buffer.clear();
        }

        // Character is a symbol
        else if(c == '(' || c == ')' || c == '[' || c ==']' || c == '{' || c =='}') {
            buffer += c;
            add_new_token(buffer, "BRACKET_SYMBOL", lineNumber, charPos);
            buffer.clear();
        } else if(c == ',') {
            buffer += c;
            add_new_token(buffer, "LIST_SEPERATOR_SYMBOL", lineNumber, charPos);
            buffer.clear();
        } else if(c == ';') {
            buffer += c;
            add_new_token(buffer, "STATEMENT_TERMINATE_SYMBOL", lineNumber, charPos);
            buffer.clear();
        } else if(c == '=') {
            buffer += c;
            add_new_token(buffer, "ASSIGN_COMP_OPERATOR_SYMBOL", lineNumber, charPos);
            buffer.clear();
        } else if(c == '.') {
            buffer += c;
            add_new_token(buffer, "CLASS_MEMBER_SYMBOL", lineNumber, charPos);
            buffer.clear();
        } else if(c == '+' || c == '-' || c == '*' || c == '/') {
            buffer += c;
            add_new_token(buffer, "MATH_OPERATOR_SYMBOL", lineNumber, charPos);
            buffer.clear();
        } else if(c == '&' || c == '|' || c == '~' || c == '<' || c == '>') {
            buffer += c;
            add_new_token(buffer, "LOGIC_OPERATOR_SYMBOL", lineNumber, charPos);
            buffer.clear();
        }

        // Unknown symbol
        else {
            throw UnknownSymbolException("(" + file_name + ") line:" + std::to_string(lineNumber) + " pos:" + std::to_string(charPos) + " Lexer error: Unrecognised symbol " + std::string(1, c));
        }
    }

    add_new_token("", "EOF", lineNumber, charPos);

    file.close();
}

void CustomJACKLexer::add_new_token(std::string lexeme, std::string type, int lineNumber, int startingCharPos) {
    CustomLexerToken* new_token = new CustomLexerToken(lexeme, type, lineNumber, startingCharPos, file_name);

    // Add new token to the list of tokens
    if(token_list_head == nullptr) {
        token_list_head = new_token;
        token_list_tail = new_token;
        next_token = new_token;
    } else {
        new_token->prev = token_list_tail;
        token_list_tail->next = new_token;
        token_list_tail = new_token;
    }
}

std::string CustomJACKLexer::get_keyword_type(std::string lexeme) {
    if(std::find(std::begin(constructKeywords), std::end(constructKeywords),
        lexeme) != std::end(constructKeywords)) {
        return "CONSTRUCT_STATEMENT";
    } else if(std::find(std::begin(typeKeywords), std::end(typeKeywords),
        lexeme) != std::end(typeKeywords)) {
        return "TYPE_STATEMENT";
    } else if(std::find(std::begin(variableKeywords), std::end(variableKeywords),
        lexeme) != std::end(variableKeywords)) {
        return "VARIABLE_STATEMENT";
    } else if(std::find(std::begin(controlKeywords), std::end(controlKeywords),
        lexeme) != std::end(controlKeywords)) {
        return "CONTROL_STATEMENT";
    } else if(std::find(std::begin(referenceKeywords), std::end(referenceKeywords),
        lexeme) != std::end(referenceKeywords)) {
        return "THIS_REFERENCE";
    } else if(std::find(std::begin(valueKeywords), std::end(valueKeywords),
        lexeme) != std::end(valueKeywords)) {
        if(lexeme == "null") {
            return "NULL_CONSTANT";
        } else {
            return "BOOLEAN_CONSTANT";
        }
    }
    return "IDENTIFIER";
}

int main(int argc, const char* argv[]) {

    if (argc != 2) {
        std::cout << "Incorrect usage. Expected 1 parameter" << std::endl;
        return -1;
    }

    CustomJACKLexer lexer(argv[1]);
    GeneratedParser::JACKCompiler parser(lexer);
    std::cout << "Starting parsing" << std::endl;
    parser.start_parsing();
    std::cout << "Done parsing. Outputting parse tree to file for use with GNUPlot" << std::endl;
    parser.parse_tree_gnu_plot();

    return 0;
}
