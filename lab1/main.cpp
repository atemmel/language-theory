#include "node.hpp"
#include "tokenizer.hpp"

struct Scope {
	Scope() { ++depth; }
	~Scope() { --depth; }
	static size_t depth;
};

size_t Scope::depth = 0;

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

	Tokenizer tokenizer;
	Tokens tokens = tokenizer.tokenize(args.front() );
	//tokenizer.print();

	std::cout << args.front() << '\n';
	puts(std::string(args.front().size(), '=').c_str() );

	Parser parser;
	auto root = parser.parseTokens(std::move(tokens) );
	visit(root.get() );

	return EXIT_SUCCESS;
}
