#include <fstream>
#include <iostream>
#include <Evaluator.hpp>

#include "Lexer.hpp"
#include "Parser.hpp"

struct Arguments final
{
	Arguments(const int argc, char** argv)
	{
		for (auto i = 1; i < argc; ++i) {
			const std::string_view arg = argv[i];

			if (arg == "-l") {
				showLexerOutput = true;
			}
			else if (arg == "-h") {
				showHelpMessage = true;
			}
			else if (i == 1) {
				filename = arg;
			}
			else {
				showHelpMessage = true;
			}
		}
	}

	std::string filename = "";
	bool showLexerOutput = false;
	bool showHelpMessage = false;
};

void printHelp(int argc, char** argv)
{
    std::cout << 
		"USL - useless scripting language.\n\n"
        "Usage:\n"
		"\t"	"usl [<file>] [option]...\n\n"
        "Options:\n"
        "\t"	"-l\tShow lexer output\n"
        "\t"	"-h\tShow this message\n";
}

int main(const int argc, char** argv)
{
	// Handle console arguments
	const Arguments arguments(argc, argv);

    if (arguments.showHelpMessage || argc <= 1) {
        printHelp(argc, argv);
        return 0;
    }

	std::ifstream file(arguments.filename);
	if (!file.is_open()) {
		std::cerr << "Unable to open file: " << arguments.filename << std::endl;
		return 1;
	}

	// Read program text
	std::string text;

	file.seekg(0, std::ios::end);
	text.reserve(file.tellg());
	file.seekg(0, std::ios::beg);

	text.assign(
		std::istreambuf_iterator<char>(file),
		std::istreambuf_iterator<char>());

	try {
		// Generate tokens
		app::Lexer lexer;
		const auto& tokens = lexer.run(text);

		/*if (arguments.showLexerOutput) {
			for (const auto& token : tokens) {
				std::cout << "{ token: \"" << token.second << 
					"\",\n  type: " << static_cast<size_t>(token.first) << 
					" }\n\n";
			}
		}*/

		// Parse tokens
		app::Parser parser;
		parser.parse(tokens);

        app::Evaluator evaluator;
		evaluator.eval({
			std::string_view{"test_var"},
			app::opcode::DECL,
			std::string_view{"test_var"},
			123.0,
			10.0,
			app::opcode::ADD,
			app::opcode::ASSIGN,
			std::string_view{"test_var"},
			10.0,
			std::string_view{"test_var"},
			app::opcode::MUL,
			app::opcode::ASSIGN,
			std::string_view{"test_var"},
			app::opcode::DEREF,
			std::string{""},
			app::opcode::ADD,
			std::string{"1330.000000"},
			app::opcode::EQ,
			false,
			app::opcode::AND,
       });
	}
	catch (const std::runtime_error& e) {
		std::cout << "ERR: " << e.what() << std::endl;
		return 1;
	}
    
    return 0;
}