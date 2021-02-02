#include "node.hpp"

#include <string_view>

constexpr std::string_view Blue = "\x1B[93m";
constexpr std::string_view Cyan = "\x1B[95m";
constexpr std::string_view Reset = "\x1B[0m";

void visit(Node *node) {
	Scope scope;
	if(!node) {
		std::cerr << "Bad\n";
		return;
	}
	for(size_t i = 0; i < scope.depth - 1; i++) {
		std::cout << "  ";
	}
	node->print();
	for(auto &c : node->children) {
		visit(c.get() );
	}
}

int main(int argc, char **argv) {
	if(argc < 2) return EXIT_FAILURE;
	std::vector<std::string> args;

	args.resize(argc - 1);
	std::copy(argv + 1, argv + argc, args.begin() );

	std::string input;
	if(!std::getline(std::cin, input) ) {
		return EXIT_FAILURE;
	}

	Tokenizer tokenizer;
	Tokens tokens = tokenizer.tokenize(args.front() );
	//tokenizer.print();

	std::cout << args.front() << '\n';
	puts(std::string(args.front().size(), '=').c_str() );

	Parser parser;
	auto root = parser.parseTokens(std::move(tokens) );
	if(!root) {
		parser.printErr();
		return EXIT_FAILURE;
	} else {
		visit(root.get() );
	}
	
	state.strBegin = state.resBegin = state.resEnd = input.cbegin();
	state.strEnd = input.cend();

	bool result = root->eval();
	std::cout << std::string(input.cbegin(), state.resBegin);
	int i = 1;
	while(result) {
		std::cout << (i++ % 2 ? Blue : Cyan) 
			<< std::string(state.resBegin, state.resEnd) << Reset;
		auto it = state.resBegin = state.resEnd;
		result = root->eval();
		std::cout << std::string(it, state.resBegin);
	}
	std::cout << std::string(state.resEnd, input.cend() ) << '\n';

	return EXIT_SUCCESS;
}
