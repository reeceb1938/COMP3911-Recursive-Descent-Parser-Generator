#include "COMP3931ParserGenerator.hpp"
#include "COMP3931Grammar.hpp"
#include "spdlog/spdlog.h"

using namespace ParserGenerator;

/*
 * ParserGenerator Class
 */

Generator::Generator(Grammar& grammar, std::string output_file_name) : grammar(grammar), output_file_name(output_file_name) {

}

Generator::~Generator() {

}
