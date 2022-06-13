
#include "parser.hpp"

void tokenize_and_invoke(std::string input, InputProcessor processor) {
	std::vector<std::string> parts;

	if (input == "q" | input == "quit") {
		exit(0);
	}

  	std::istringstream iss(input);
	bool str = false;
	std::string tmp;

  	for (char chr : input) {
		if (chr == '"') {str = !str; continue;}
		if (chr != ' ') {tmp += chr; continue;}
		if (str) {tmp += ' '; continue;}

		parts.push_back(tmp);
		tmp = "";
  	}

	if (!tmp.empty()) {
		if (str) {
			logger::warn("Expected closing quote (\") but found end of line!");
		}

		parts.push_back(tmp);
	}

	try {
		processor(parts);
	} catch(std::invalid_argument error) {
		logger::error(error.what(), " or invalid syntax, use 'help' to learn more");
	} catch(int val) {
		// flow control
	}
}

void run_input_processor(InputProcessor processor) {
	while (true) {
		std::string input;
		getline(std::cin, input);
		tokenize_and_invoke(input, processor);
	}
}

void assert_argument_count(std::vector<std::string>& parts, int count) {
	if (parts.size() != count) {
		throw std::invalid_argument("Invalid argument count");
	}
}

void match_command(std::vector<std::string>& parts, const std::string& command, int count, InputProcessor processor) {
	if (parts[0] == command) {
		assert_argument_count(parts, count);

		processor(parts);
		throw 0;
	}
}

