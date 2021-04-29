#ifndef __COMP3931_PARSER_GENERATOR_HEADER__
#define __COMP3931_PARSER_GENERATOR_HEADER__

#include <string>

#include "COMP3931Grammar.hpp"

namespace ParserGenerator {

    // The below string define parts of the outputted header and source code files
    // The R prefix shows the strings are raw strings
    // The V0G0N delimiter is an escape sequence that allows the use of ) in the raw sequence
    const std::string header_lexer_token_class =
R"V0G0N(class LexerToken {
    public:
        LexerToken(std::string lexeme, std::string token_type, int line_number, int char_position, std::string file_name);
        ~LexerToken();

        std::string get_lexeme();
        std::string get_token_type();
        int get_line_number();
        int get_char_position();
        std::string get_file_name();

    protected:
        std::string lexeme;
        std::string token_type;
        int line_number;
        int char_position;
        std::string file_name;
};)V0G0N";

    const std::string source_lexer_token_class =
R"V0G0N(LexerToken::LexerToken(std::string lexeme, std::string token_type, int line_number, int char_position, std::string file_name) : lexeme(lexeme), token_type(token_type), line_number(line_number), char_position(char_position), file_name(file_name) {}
LexerToken::~LexerToken() {}

std::string LexerToken::get_lexeme() { return lexeme; }

std::string LexerToken::get_token_type() { return token_type; }

int LexerToken::get_line_number() { return line_number; }

int LexerToken::get_char_position() { return char_position; })V0G0N";

    const std::string header_virtual_lexer_class =
R"V0G0N(class VirtualLexer {
    public:
        virtual LexerToken& get_next_token() = 0;
        virtual LexerToken& peak_next_token() = 0;
};)V0G0N";

    const std::string header_invalid_token_exception_class =
R"V0G0N(class InvalidTokenException : public std::runtime_error {
    public:
        InvalidTokenException(const char* message);
        InvalidTokenException(std::string message);
        ~InvalidTokenException();
};)V0G0N";

    const std::string source_invalid_token_exception_class =
R"V0G0N(InvalidTokenException::InvalidTokenException(const char* message) : runtime_error(message) {}

InvalidTokenException::InvalidTokenException(std::string message) : runtime_error(message) {}

InvalidTokenException::~InvalidTokenException() {})V0G0N";

    const std::string header_internal_error_exception_class =
R"V0G0N(class InternalErrorException : public std::runtime_error {
public:
    InternalErrorException(const char* message);
    InternalErrorException(std::string message);
    ~InternalErrorException();
};)V0G0N";

    const std::string source_internal_error_exception_class =
R"V0G0N(InternalErrorException::InternalErrorException(const char* message) : runtime_error(message) {}

InternalErrorException::InternalErrorException(std::string message) : runtime_error(message) {}

InternalErrorException::~InternalErrorException() {})V0G0N";

    const std::string header_parse_tree_node_class =
R"V0G0N(class ParseTreeNode {
    public:
        ParseTreeNode(std::string token);
        ~ParseTreeNode();

        std::string get_token();
        void add_child(ParseTreeNode* new_child);
        std::vector<ParseTreeNode*>& get_children();

    private:
        std::string token;
        std::vector<ParseTreeNode*> children;
};)V0G0N";

    const std::string source_parse_tree_node_class =
R"V0G0N(ParseTreeNode::ParseTreeNode(std::string token) : token(token) {}

ParseTreeNode::~ParseTreeNode() {
    for (const ParseTreeNode* child : children) {
        delete child;
    }
}

std::string ParseTreeNode::get_token() { return token; }

void ParseTreeNode::add_child(ParseTreeNode* new_child) {
    if (new_child == nullptr || new_child == NULL) {
        return;
    }

    children.push_back(new_child);
}

std::vector<ParseTreeNode*>& ParseTreeNode::get_children() { return children; })V0G0N";

    const std::string source_parser_error_function =
R"V0G0N(parsing_error(LexerToken& found_token, std::string expected_value) {
    throw InvalidTokenException("Line " + std::to_string(found_token.get_line_number()) + ":" + std::to_string(found_token.get_char_position()) + " Parsing error: expected `" + expected_value + "` but found `" + found_token.get_lexeme() + "`");
})V0G0N";

    // Class to generate code files for a recursive descent parser from a grammar
    class Generator {
    public:
        Generator(Grammar& grammar, std::string output_file_name);
        ~Generator();

    private:
        Grammar& grammar;
        std::string output_file_name;

        bool generate();
        bool generate_header_file();
        bool generate_source_file();
        bool generate_production_code(std::ofstream& code_file, EBNFToken* ebnf_token, int indentation_level);

        // Add the indentation before a line of code
        void indent(std::ofstream& file, int level);
    };

} // namespace ParserGenerator

#endif
