#include "node.hpp"
#include "tokenizer.hpp"

void visit(Node *node) {
	if(!node) {
		std::cerr << "Bad\n";
		return;
	}
	for(auto &c : node->children) {
		c->print();
		visit(c.get() );
	}
}

int main(int argc, char **argv) {
	if(argc < 2) return EXIT_FAILURE;
	std::vector<std::string> args;

	args.resize(argc - 1);
	std::copy(argv + 1, argv + argc, args.begin() );

	Tokenizer tokenizer;
	Tokens tokens = tokenizer.tokenize(args.front() );
	tokenizer.print();

	std::puts("==================");

	Parser parser;
	auto root = parser.parseTokens(std::move(tokens) );
	visit(root.get() );

	return EXIT_SUCCESS;
}
