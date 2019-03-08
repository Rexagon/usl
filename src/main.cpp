#include <iostream>

void printHelp(int argc, char** argv)
{
    std::cout 
        << "USL - useless scripting language." << std::endl << std::endl
        << "Usage:" << std::endl
        << "\t" << "usl <file> [option]..." << std::endl << std::endl
        << "Options:" << std::endl
        << "\t" << "-f <filename>\tExecute specified file" << std::endl
        << "\t" << "-h\t\tShow this screen" << std::endl
        << std::endl;
}

int main(int argc, char** argv)
{
    if (argc <= 1) {
        printHelp(argc, argv);
        return 0;
    }

    for (int i = 1; i < argc; ++i) {
		std::string_view arg = argv[i];

		if (arg == "-f" && ++i < argc) {
			
		}
		else {
			printHelp(argc, argv);
		}
    }
    
    return 0;
}