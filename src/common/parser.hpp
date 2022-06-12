
#pragma once

#include "core.hpp"

using InputProcessor = std::function<void(std::vector<std::string>&)>;

/// split line into parts and invoke input processor
void tokenize_and_invoke(std::string input, InputProcessor processor);

/// start the input processor loop
void run_input_processor(InputProcessor processor);

/// assert that the amount of given argumets is correct
void assert_argument_count(std::vector<std::string>& parts, int count);

/// check if command matches
void match_command(std::vector<std::string>& parts, const std::string& command, int count, InputProcessor processor);

