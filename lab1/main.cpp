#include "tokenizer.hpp"

int main(int argc, char **argv) {
	if(argc < 2) return EXIT_FAILURE;
	std::vector<std::string> args;

	args.resize(argc - 1);
	std::copy(argv + 1, argv + argc, args.begin() );

	Tokenizer tokenizer;
	auto tokens = tokenizer.tokenize(args.front() );
	tokenizer.print();

	std::puts("==================");

	return EXIT_SUCCESS;
}
