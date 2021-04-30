#ifndef __TEST_APP__
#define __TEST_APP__

#include <string>
#include <vector>

#include "JACKCompiler.hpp"

class CustomLexerToken : public GeneratedParser::LexerToken {
public:
        CustomLexerToken(std::string lexeme, std::string token_type, int line_number, int char_position, std::string file_name) : LexerToken(lexeme, token_type, line_number, char_position, file_name), next(nullptr), prev(nullptr) {}
        ~CustomLexerToken() {}

        CustomLexerToken* next;
        CustomLexerToken* prev;
};

class CustomJACKLexer : public GeneratedParser::VirtualLexer {
public:
    CustomJACKLexer(std::string file_name);
    ~CustomJACKLexer();

    GeneratedParser::LexerToken& get_next_token();
    GeneratedParser::LexerToken& peak_next_token();

private:
    std::string file_name;
    CustomLexerToken* token_list_head;
    CustomLexerToken* token_list_tail;
    CustomLexerToken* next_token;

    void scan_file();
    void add_new_token(std::string lexeme, std::string type, int lineNumber, int startingCharPos);
    std::string get_keyword_type(std::string lexeme);

    // Store JACK keywords in lists
    const static std::vector<std::string> constructKeywords;
    const static std::vector<std::string> typeKeywords;
    const static std::vector<std::string> variableKeywords;
    const static std::vector<std::string> controlKeywords;
    const static std::vector<std::string> referenceKeywords;
    const static std::vector<std::string> valueKeywords;
};

// Error classes

class FileNotFoundException : public std::runtime_error {
public:
    FileNotFoundException(const char* message) : runtime_error(message) {};
    FileNotFoundException(std::string message) : runtime_error(message) {};
};

class UnexpectedEndOfFileException : public std::runtime_error {
public:
    UnexpectedEndOfFileException(const char* message) : runtime_error(message) {};
    UnexpectedEndOfFileException(std::string message) : runtime_error(message) {};
};

class UnknownSymbolException : public std::runtime_error {
public:
    UnknownSymbolException(const char* message) : runtime_error(message) {};
    UnknownSymbolException(std::string message) : runtime_error(message) {};
};

#endif
