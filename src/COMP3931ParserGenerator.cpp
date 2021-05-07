#include <cstdio>
#include <fstream>

#include "COMP3931ParserGenerator.hpp"
#include "COMP3931Grammar.hpp"
#include "spdlog/spdlog.h"

using namespace ParserGenerator;

/*
 * ParserGenerator Class
 */

Generator::Generator(Grammar& grammar, std::string output_file_name) : grammar(grammar), output_file_name(output_file_name) {
    if (!grammar.get_is_final()) {
        grammar.finalize_grammar();
    }

    // Check for First / Follow conflicts
    for (const std::string& nonterminal : grammar.get_nonterminals()) {
        std::set<std::string> first_set = grammar.get_first_set(nonterminal);
        std::set<std::string> follow_set = grammar.get_follow_set(nonterminal);

        if (first_set.count("epsilon") == 1) {
            // Check sets are disjoint
            for (const std::string& symbol : first_set) {
                if (follow_set.count(symbol) != 0) {
                    spdlog::error("First/Follow conflict detected for non-terminal `{}`. The symbol `{}` appears in both the First and Follow set while `epsilon` is also in the First set", nonterminal, symbol);
                    return;
                }
            }
        }
    }

    generate();
}

Generator::~Generator() {

}

bool Generator::generate() {
    bool header_status = generate_header_file();
    bool code_status = generate_source_file();

    if (header_status == false || code_status == false) {
        // Something failed - delete the files as they are invalid
        remove((output_file_name + ".hpp").c_str());
        remove((output_file_name + ".cpp").c_str());
    }

    return false;
}

bool Generator::generate_header_file() {
    spdlog::info("Writing header file to `{}.hpp`", output_file_name);

    std::ofstream header_file(output_file_name + ".hpp");

    if (!header_file) {
        spdlog::error("Could not open file `" + output_file_name + ".hpp` for writing");
        return false;
    }

    // Write an include guard
    header_file << "#ifndef __" << output_file_name << "_HEADER__" << std::endl;
    header_file << "#define __" << output_file_name << "_HEADER__" << std::endl;
    header_file << std::endl;

    // Write header file includes
    header_file << "#include <fstream>" << std::endl;
    header_file << "#include <stdexcept>" << std::endl;
    header_file << "#include <string>" << std::endl;
    header_file << "#include <vector>" << std::endl;
    header_file << std::endl;

    // Start namespace
    header_file << "namespace GeneratedParser {" << std::endl;

    // Write LexerToken class
    header_file << header_lexer_token_class << std::endl << std::endl;

    // Write ViertualLexer class
    header_file << header_virtual_lexer_class << std::endl << std::endl;

    // Write exception classes
    header_file << header_invalid_token_exception_class << std::endl << std::endl;
    header_file << header_internal_error_exception_class << std::endl << std::endl;

    // Write ParseTreeNode class
    header_file << header_parse_tree_node_class << std::endl << std::endl;

    // Write output_file_name class
    header_file << "class " << output_file_name << " {" << std::endl;
    header_file << "\tpublic:" << std::endl;
    header_file << "\t\t" << output_file_name << "(VirtualLexer& lexer);" << std::endl;
    header_file << "\t\t~" << output_file_name << "();" << std::endl;
    header_file << std::endl;
    header_file << "\t\tvoid start_parsing();" << std::endl;
    header_file << "\t\tvoid parse_tree_gnu_plot();" << std::endl;
    header_file << std::endl;
    header_file << "\tprivate:" << std::endl;
    header_file << "\t\tVirtualLexer& lexer;" << std::endl;
    header_file << "\t\tParseTreeNode* parse_tree_root;" << std::endl;
    header_file << std::endl;
    header_file << "\t\tvoid parsing_error(LexerToken& found_token, std::string expected_value);" << std::endl;

    // Insert parsing functions here
    std::unordered_map<std::string, EBNFToken*>& production_rules = grammar.get_all_productions();
    for (std::pair<std::string, EBNFToken*> production : production_rules) {
        header_file << "\t\tvoid parse_" << production.first << "(ParseTreeNode* parse_tree_parent);" << std::endl;
    }
    header_file << std::endl;

    header_file << "};" << std::endl;

    // End namespace
    header_file << "} // namespace GeneratedParser" << std::endl;

    // End include guard
    header_file << std::endl << "#endif" << std::endl;

    return true;
}

bool Generator::generate_source_file() {
    spdlog::info("Writing source code file to `{}.cpp`", output_file_name);

    std::ofstream code_file(output_file_name + ".cpp");

    if (!code_file) {
        spdlog::error("Could not open file `" + output_file_name + ".cpp` for writing");
        return false;
    }

    bool status = false;

    // Write source code file includes
    code_file << "#include <fstream>" << std::endl;
    code_file << "#include <queue>" << std::endl;
    code_file << "#include <stdexcept>" << std::endl;
    code_file << "#include <string>" << std::endl;
    code_file << "#include <vector>" << std::endl;
    code_file << "#include \"" << output_file_name << ".hpp\"" << std::endl;
    code_file << std::endl;

    // Add namespace using directive
    code_file << "using namespace GeneratedParser;" << std::endl << std::endl;

    // Write LexerToken class
    code_file << source_lexer_token_class << std::endl << std::endl;

    // Write exception classes
    code_file << source_invalid_token_exception_class << std::endl << std::endl;
    code_file << source_internal_error_exception_class << std::endl << std::endl;

    // Write ParseTreeNode class
    code_file << source_parse_tree_node_class << std::endl << std::endl;

    // Write output_file_name class
    code_file << output_file_name << "::" << output_file_name << "(VirtualLexer& lexer) : lexer(lexer), parse_tree_root(nullptr) {}" << std::endl;
    code_file << output_file_name << "::~" << output_file_name << "() {}" << std::endl << std::endl;

    // Write start parsing function
    code_file << "void " << output_file_name << "::start_parsing() {" << std::endl;
    code_file << "\tparse_tree_root = new ParseTreeNode(\"\");" << std::endl;
    code_file << "\tparse_" << grammar.get_start_symbol() << "(parse_tree_root);" << std::endl;
    code_file << "}" << std::endl << std::endl;

    // Write parsing error function
    code_file << "void " << output_file_name << "::" << source_parser_error_function << std::endl;
    code_file << std::endl;

    std::unordered_map<std::string, EBNFToken*>& production_rules = grammar.get_all_productions();
    for (std::pair<std::string, EBNFToken*> production : production_rules) {
        code_file << "// " << production.first << " ::= " << production.second->to_string() << std::endl;
        code_file << "void " << output_file_name << "::parse_" << production.first << "(ParseTreeNode* parse_tree_parent) {" << std::endl;
        code_file << "\t// Use peak_next_token() to define next_token reference" << std::endl;
        code_file << "\tLexerToken& next_token = lexer.peak_next_token();" << std::endl;
        code_file << std::endl;

        // Add the code to construct the parse tree

        code_file << "\tParseTreeNode* new_node = new ParseTreeNode(\"" << production.first << "\");" << std::endl;

        code_file << "\tif (parse_tree_parent == nullptr) {" << std::endl;
        code_file << "\t\tdelete new_node;" << std::endl;
        code_file << "\t\tthrow InternalErrorException(\"Parse tree node pointer is nullptr\");" << std::endl;
        code_file << "\t} else {" << std::endl;
        code_file << "\t\tparse_tree_parent->add_child(new_node);" << std::endl;
        code_file << "\t}" << std::endl;
        code_file << std::endl;

        if (production.second == nullptr) {
            spdlog::error("No productions defined for non-terminal `{}`.", production.first);
            return false;
        }

        status = generate_production_code(code_file, production.second, 1);
        code_file << std::endl;

        code_file << "}" << std::endl << std::endl;

        if (status == false) {
            break;
        }
    }
    code_file << std::endl;

    if (status == true) {
        // Write debug printing of parse tree functions
        code_file << "void " << output_file_name << "::parse_tree_gnu_plot() {" << std::endl;
        code_file << "\tstd::ofstream file = std::ofstream(\"parse-tree.out\");" << std::endl;
        code_file << "\tif (!file) {" << std::endl;
        code_file << "\t\treturn;" << std::endl;
        code_file << "\t}" << std::endl << std::endl;
        code_file << "\tint node_id_counter = 1;" << std::endl;
        code_file << "\tstd::queue<std::pair<int, ParseTreeNode*>> node_queue;" << std::endl;
        code_file << "\tnode_queue.push(std::pair<int, ParseTreeNode*>(-1, parse_tree_root));" << std::endl;
        code_file << std::endl;
        code_file << "\twhile (!node_queue.empty()) {" << std::endl;
        code_file << "\t\tstd::pair<int, ParseTreeNode*> current_node = node_queue.front();" << std::endl << std::endl;
        code_file << "\t\tif (current_node.first == -1) {" << std::endl;
        code_file << "\t\t\tfile << node_id_counter << \" NaN \" << std::endl;" << std::endl;
        code_file << "\t\t} else {" << std::endl;
        code_file << "\t\t\tfile << node_id_counter << \" \" << current_node.first << \" \" << current_node.second->get_token() << std::endl;" << std::endl;
        code_file << "\t\t}" << std::endl << std::endl;
        code_file << "\t\t for (ParseTreeNode* child : current_node.second->get_children()) {" << std::endl;
        code_file << "\t\t\t" << "node_queue.push(std::pair<int, ParseTreeNode*>(node_id_counter, child));" << std::endl;
        code_file << "\t\t}" << std::endl << std::endl;
        code_file << "\t\tnode_id_counter++;" << std::endl;
        code_file << "\t\tnode_queue.pop();" << std::endl;
        code_file << "\t}" << std::endl;
        code_file << "}" << std::endl << std::endl;
    }

    return status;
}

bool Generator::generate_production_code(std::ofstream& code_file, EBNFToken* ebnf_token, int indentation_level) {
    if (ebnf_token == nullptr) {
        spdlog::error("Internal error. EBNF parser tree invalid while generating parser code");
        return false;
    }

    bool success = false;

    spdlog::trace("Generating code from `{}`", ebnf_token->to_string());

    std::vector<EBNFToken*>& ebnf_token_children = ebnf_token->get_children();

    switch (ebnf_token->get_type()) {
        case EBNFToken::TokenType::SEQUENCE:
            for (int i = 0; i < ebnf_token_children.size(); i++) {
                success = generate_production_code(code_file, ebnf_token_children[i], indentation_level);

                code_file << std::endl;

                if (success == false) {
                    break;
                }
            }
            break;
        case EBNFToken::TokenType::TERMINAL:
            // Handle cases of epsilon, string_literal, identifier, integer_constant
            if (ebnf_token->get_value() == "epsilon") {
                code_file << "// Produces epsilon so do nothing" << std::endl;
                code_file << "new_node->add_child(new ParseTreeNode(\"epsilon\"));" << std::endl;
                success = true;
            } else {
                indent(code_file, indentation_level);
                code_file << "next_token = lexer.get_next_token();" << std::endl;
                indent(code_file, indentation_level);

                if (ebnf_token->get_value() == "numeric_constant") {
                   code_file << "if (next_token.get_token_type() == \"NUMERIC_CONSTANT\") {" << std::endl;
                   indent(code_file, indentation_level + 1);
                   code_file << "ParseTreeNode* tmp_node = new ParseTreeNode(\"NUMERIC_CONSTANT\");" << std::endl;
                   indent(code_file, indentation_level + 1);
                   code_file << "tmp_node->add_child(new ParseTreeNode(next_token.get_lexeme()));" << std::endl;
                   indent(code_file, indentation_level + 1);
                   code_file << "new_node->add_child(tmp_node);" << std::endl;
                   indent(code_file, indentation_level);
                   code_file << "} else {" << std::endl;
                   indent(code_file, indentation_level + 1);
                   code_file << "parsing_error(next_token, \"" << ebnf_token->get_value() << "\");" << std::endl;
                   indent(code_file, indentation_level);
                   code_file << "}" << std::endl;
               } else if (ebnf_token->get_value() == "string_literal") {
                   code_file << "if (next_token.get_token_type() == \"STRING_LITERAL\") {" << std::endl;
                   indent(code_file, indentation_level + 1);
                   code_file << "ParseTreeNode* tmp_node = new ParseTreeNode(\"STRING_LITERAL\");" << std::endl;
                   indent(code_file, indentation_level + 1);
                   code_file << "tmp_node->add_child(new ParseTreeNode(next_token.get_lexeme()));" << std::endl;
                   indent(code_file, indentation_level + 1);
                   code_file << "new_node->add_child(tmp_node);" << std::endl;
                   indent(code_file, indentation_level);
                   code_file << "} else {" << std::endl;
                   indent(code_file, indentation_level + 1);
                   code_file << "parsing_error(next_token, \"" << ebnf_token->get_value() << "\");" << std::endl;
                   indent(code_file, indentation_level);
                   code_file << "}" << std::endl;
               } else if (ebnf_token->get_value() == "identifier") {
                   code_file << "if (next_token.get_token_type() == \"IDENTIFIER\") {" << std::endl;
                   indent(code_file, indentation_level + 1);
                   code_file << "ParseTreeNode* tmp_node = new ParseTreeNode(\"IDENTIFIER\");" << std::endl;
                   indent(code_file, indentation_level + 1);
                   code_file << "tmp_node->add_child(new ParseTreeNode(next_token.get_lexeme()));" << std::endl;
                   indent(code_file, indentation_level + 1);
                   code_file << "new_node->add_child(tmp_node);" << std::endl;
                   indent(code_file, indentation_level);
                   code_file << "} else {" << std::endl;
                   indent(code_file, indentation_level + 1);
                   code_file << "parsing_error(next_token, \"" << ebnf_token->get_value() << "\");" << std::endl;
                   indent(code_file, indentation_level);
                   code_file << "}" << std::endl;
               } else {
                   code_file << "if (next_token.get_lexeme() == \"" << ebnf_token->get_value() << "\") {" << std::endl;
                   indent(code_file, indentation_level + 1);
                   code_file << "new_node->add_child(new ParseTreeNode(\"" << ebnf_token->get_value() << "\"));" << std::endl;
                   indent(code_file, indentation_level);
                   code_file << "} else {" << std::endl;
                   indent(code_file, indentation_level + 1);
                   code_file << "parsing_error(next_token, \"" << ebnf_token->get_value() << "\");" << std::endl;
                   indent(code_file, indentation_level);
                   code_file << "}" << std::endl;
               }
               success = true;
            }
            break;
        case EBNFToken::TokenType::NONTERMINAL:
            indent(code_file, indentation_level);
            code_file << "parse_" << ebnf_token->get_value() << "(new_node);";
            success = true;
            break;
        case EBNFToken::TokenType::OR: {
            // Check for first / first conflicts
            std::set<std::set<std::string>> all_first_sets = std::set<std::set<std::string>>();
            std::set<std::string> first_set;
            for (int i = 0; i < ebnf_token_children.size(); i++) {
                first_set = grammar.calculate_first_set(ebnf_token_children[i]);

                // Check the sets are disjoint
                for (const std::set<std::string>& other_first_set : all_first_sets) {
                    for (const std::string& symbol : first_set) {
                        if (other_first_set.count(symbol) != 0) {
                            // Sets are not disjoint: symbol is in both first_set and other_first_set
                            spdlog::error("First/First conflict detected for `{}`. Symbol `{}` appears in more than one First set.", ebnf_token->to_string(), symbol);
                            return false;
                        }
                    }
                }

                all_first_sets.insert(first_set);
            }

            // Generate the approriate code
            bool is_first = true;
            indent(code_file, indentation_level);
            code_file << "next_token = lexer.peak_next_token();" << std::endl;
            for (int i = 0; i < ebnf_token_children.size(); i++) {
                first_set = grammar.calculate_first_set(ebnf_token_children[i]);
                first_set.erase("epsilon");     // Remove epsilon because it doesn't actually appear in the input stream

                if (first_set.size() == 0) {
                    continue;
                }

                if (is_first == true) {
                    indent(code_file, indentation_level);
                    code_file << "if (";
                    is_first = false;
                } else {
                    code_file << " else if (";
                }

                int j = 0;
                for (std::string val : first_set) {
                    if (val == "numeric_constant") {
                        code_file << "next_token.get_token_type() == \"NUMERIC_CONSTANT\"";
                    } else if (val == "string_literal") {
                        code_file << "next_token.get_token_type() == \"STRING_LITERAL\"";
                    } else if (val == "identifier") {
                        code_file << "next_token.get_token_type() == \"IDENTIFIER\"";
                    } else {
                        code_file << "next_token.get_lexeme() == \"" << val << "\"";
                    }

                    if (j == first_set.size() - 1) {
                        code_file << ") {" << std::endl;
                        success = generate_production_code(code_file, ebnf_token_children[i], indentation_level + 1);
                        indent(code_file, indentation_level);
                        code_file << "}";

                        if (success == false) {
                            return false;
                        }
                    } else {
                        code_file << " || ";
                    }
                    j++;
                }
            }

            first_set = grammar.calculate_first_set(ebnf_token);
            if (first_set.count("epsilon") == 0) {
                code_file << " else {" << std::endl;
                indent(code_file, indentation_level + 1);
                code_file << "parsing_error(next_token, \"" << ebnf_token->to_string() << "\");" << std::endl;
                indent(code_file, indentation_level);
                code_file << "}" << std::endl;
            } else {
                code_file << " else {" << std::endl;
                indent(code_file, indentation_level + 1);
                code_file << "new_node->add_child(new ParseTreeNode(\"epsilon\"));" << std::endl;
                indent(code_file, indentation_level);
                code_file << "}" << std::endl;
            }

            }
            break;
        case EBNFToken::TokenType::REPEAT: {
            std::set<std::string> first_set = grammar.calculate_first_set(ebnf_token_children[0]);
            first_set.erase("epsilon");     // Remove epsilon because it doesn't actually appear in the input stream

            if (first_set.size() == 0) {
                success = true;
                break;
            }

            indent(code_file, indentation_level);
            code_file << "next_token = lexer.peak_next_token();" << std::endl;
            indent(code_file, indentation_level);
            code_file << "while (";

            int j = 0;
            for (std::string val : first_set) {
                if (val == "numeric_constant") {
                    code_file << "next_token.get_token_type() == \"NUMERIC_CONSTANT\"";
                } else if (val == "string_literal") {
                    code_file << "next_token.get_token_type() == \"STRING_LITERAL\"";
                } else if (val == "identifier") {
                    code_file << "next_token.get_token_type() == \"IDENTIFIER\"";
                } else {
                    code_file << "next_token.get_lexeme() == \"" << val << "\"";
                }

                if (j == first_set.size() - 1) {
                    code_file << ") {" << std::endl;
                    success = generate_production_code(code_file, ebnf_token_children[0], indentation_level + 1);
                    indent(code_file, indentation_level + 1);
                    code_file << "next_token = lexer.peak_next_token();" << std::endl;
                    indent(code_file, indentation_level);
                    code_file << "}";

                    if (success == false) {
                        return false;
                    }
                } else {
                    code_file << " || ";
                }
                j++;
            }

            code_file << std::endl;

            success = true;
            }
            break;
        case EBNFToken::TokenType::OPTIONAL: {
            std::set<std::string> first_set = grammar.calculate_first_set(ebnf_token_children[0]);
            first_set.erase("epsilon");     // Remove epsilon because it doesn't actually appear in the input stream

            if (first_set.size() == 0) {
                success = true;
                break;
            }

            indent(code_file, indentation_level);
            code_file << "next_token = lexer.peak_next_token();" << std::endl;
            indent(code_file, indentation_level);
            code_file << "if (";

            int j = 0;
            for (std::string val : first_set) {
                if (val == "numeric_constant") {
                    code_file << "next_token.get_token_type() == \"NUMERIC_CONSTANT\"";
                } else if (val == "string_literal") {
                    code_file << "next_token.get_token_type() == \"STRING_LITERAL\"";
                } else if (val == "identifier") {
                    code_file << "next_token.get_token_type() == \"IDENTIFIER\"";
                } else {
                    code_file << "next_token.get_lexeme() == \"" << val << "\"";
                }

                if (j == first_set.size() - 1) {
                    code_file << ") {" << std::endl;
                    success = generate_production_code(code_file, ebnf_token_children[0], indentation_level + 1);
                    indent(code_file, indentation_level);
                    code_file << "}";

                    if (success == false) {
                        return false;
                    }
                } else {
                    code_file << " || ";
                }
                j++;
            }

            code_file << std::endl;

            }
            break;
        case EBNFToken::TokenType::GROUP:
            for (int i = 0; i < ebnf_token_children.size(); i++) {
                success = generate_production_code(code_file, ebnf_token_children[i], indentation_level);

                code_file << std::endl;

                if (success == false) {
                    break;
                }
            }
            break;
        default:
            spdlog::error("Unkown type of EBNFToken when generating code for parser");
    }

    return success;
}

void Generator::indent(std::ofstream& file, int level) {
    for (int i = 0; i < level; i++) {
        file << "\t";
    }
}
