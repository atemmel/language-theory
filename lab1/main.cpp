#include "node.hpp"

#include <string_view>

constexpr std::string_view Blue = "\x1B[34m";
constexpr std::string_view Cyan = "\x1B[36m";
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


	std::string input = "Waterloo I was defeated, you won the war \
Waterloo promise to love you for ever more \
Waterloo couldn't escape if I wanted to \
Waterloo knowing my fate is to be with you \
Waterloo finally facing my Waterloo";
	State::strBegin = input.begin();
	State::strEnd = input.end();

	puts(std::string(args.front().size(), '=').c_str() );

	Span span = root->eval({input.cbegin(), input.cend()});

	for(auto & vec : State::groupings) {
		std::cout << std::string(vec.first, vec.last) 
			<< "  ";
	}
	std::cout << '\n';
	puts(std::string(args.front().size(), '=').c_str() );

	std::cout << std::string(input.cbegin(), span.first) 
		<< Blue << std::string(span.first, span.last) << Reset;
	int i = 0;
	while(span) {
		Span prev = span;
		span = root->eval({span.last, input.cend()});
		//if(prev.last >= span.first || span.first > span.last) {
		//break;
		//}
		std::cout << std::string(prev.last, span.first)
			<< (i++ % 2 ? Blue : Cyan) << std::string(span.first, span.last) << Reset;
	}
	std::cout << '\n';

	/*
	State state;
	auto spans = root->eval(input.begin(), input.end(), state);
	for(auto s : spans) {
		std::cout << '{' << s << "} ";
		std::cout << std::string(s.first, s.last) << ' ';
	}
	std::cout << '\n';

	puts(std::string(args.front().size(), '=').c_str() );

	auto it = input.cbegin();
	int i = 0;
	for(auto s : spans) {
		if(it > s.first) {
			continue;
		}
		std::cout << std::string(it, s.first) << (i % 2 ? Blue : Cyan) 
			<< std::string(s.first, s.last) << Reset;
		it = s.last;
		i++;
	}
	std::cout << std::string(it, input.cend() ) << '\n';

	puts(std::string(args.front().size(), '=').c_str() );
	*/

	return EXIT_SUCCESS;
}
