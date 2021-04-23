#include <iostream>
#include <vector>

#include "COMP3931Grammar.hpp"
#include "COMP3931ParserGenerator.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

int main(int argc, char const* argv[]) {
    // Create a logger to stdout and  a logger to a text file
    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    // stdout_sink->set_level(spdlog::level::warn);
    stdout_sink->set_level(spdlog::level::trace);

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_st>("output.log");
    file_sink->set_level(spdlog::level::trace);

    spdlog::sinks_init_list sink_list = {file_sink, stdout_sink};

    spdlog::set_default_logger(std::make_shared<spdlog::logger>("", sink_list.begin(), sink_list.end()));
    spdlog::set_level(spdlog::level::trace);

    spdlog::trace("Setup complete");

    if (argc != 3) {
        spdlog::error("Invalid number of parameters");
        spdlog::info("Correct usage: {} [input file name] [output file name]", argv[0]);
        return 1;
    }

    ParserGenerator::Grammar grammar;
    grammar.input_language_from_file(argv[1]);

    bool valid = grammar.can_produce_ll_parser();
    spdlog::info("Is valid? {}", valid);

    grammar.log_grammar();

    spdlog::info("Generating parser");

    ParserGenerator::Generator pg(grammar, argv[1]);

    // spdlog::warn("Easy padding in numbers like {:08d}", 12);
    // spdlog::critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
    // spdlog::info("Support for floats {:03.2f}", 1.23456);
    // spdlog::info("Positional args are {1} {0}..", "too", "supported");
    // spdlog::info("{:>8} aligned, {:<8} aligned", "right", "left");

    return 0;
}
