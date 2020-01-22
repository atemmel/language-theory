//1.6 1.7 1.8 Implementation, miljöer

#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>

constexpr static std::array<char, 4> binaryOperators = {
	'+',
	'-',
	'*',
	'/'
};

struct S {
	S(int i ) { std::cout << "New scope size " << i << '\n'; };
	~S() { std::cout << "Scope destroyed\n"; };
};

enum struct TokenType {
	Integer,
	BinaryOperator
};

constexpr static std::array<std::string_view, 2> tokenStrings = {
	"Integer",
	"BinaryOperator"
};

struct Token {
	TokenType type;
	int value;
	int index = -1;
};

using Tokens = std::vector<Token>;
using TokenIterator = Tokens::iterator;

struct Node {
	Node(TokenIterator it) : iterator(it) {
		std::cout << "Node created: " << it->value << "\t\t" << it->index << '\n';
	}
	TokenIterator iterator;
	Node* left = nullptr;
	Node* right = nullptr;

};

constexpr std::string_view prefix = ">>>  ";

void printCarat(int index) {
	for(auto c : prefix) std::cerr << ' ';
	for(int i = 0; i < index; i++) std::cerr << ' ';
	std::cerr << "^\n";
}

bool highPrecedence(int c) {
	return c == '*' || c == '/';
	/*
	switch(c) {
		case '*':
		case '/':
			return true;
		default:
			return false;
	}
	return false;
	*/
}

void printTokens(const Tokens &tokens) {
	for(auto t : tokens) {
		std::cout << t.value << ' ';
	}
	std::cout << "\n\n";
}

template<typename BidirectionalIterator>
void buildTree(const BidirectionalIterator first, const BidirectionalIterator last, Node *node) {
	S s(std::distance(first, last) );
	if(first == last) return;

	auto leftIt = std::prev(node->iterator);
	for(; leftIt != first; leftIt--) {
		if(leftIt->type == TokenType::BinaryOperator
				&& highPrecedence(leftIt->value) ) {
			std::cout << "L1: ";
			node->left = new Node{leftIt};
			buildTree(first, node->iterator, node->left);
			break;
		}
	}

	if(leftIt == first) {
		std::cout << "L2: ";
		node->left = new Node{first};
	}

	auto rightIt = std::next(node->iterator);
	auto firstBinOp = last;
	for(; rightIt != last; rightIt++) {
		if(rightIt->type == TokenType::BinaryOperator
				&& !highPrecedence(rightIt->value) ) {
			std::cout << "R1: ";
			node->right = new Node{rightIt};
			buildTree(std::next(node->iterator), last, node->right);
			break;
		} else if(firstBinOp == last
				&& rightIt->type == TokenType::BinaryOperator
				&& highPrecedence(rightIt->value) ) {
			firstBinOp = rightIt;
		}
	}

	if(rightIt == last) {
		if(firstBinOp == last) {
			std::cout << "R2: ";
			node->right = new Node{std::prev(last) };
		} else {
			std::cout << "R3: ";
			node->right = new Node{firstBinOp};
			buildTree(std::next(node->iterator), last, node->right);
		}
	}
	std::cout << "END\n";
}

Tokens shuntingYard(const Tokens &tokens) {
	Tokens output;
	Tokens stack;

	for(const auto &t : tokens) {
		if(t.type == TokenType::Integer) {
			output.push_back(t);
		}
		else if(t.type == TokenType::BinaryOperator) {
			if(stack.empty() || (!stack.empty() && highPrecedence(stack.back().value ^ highPrecedence(t.value) ) ) ) {
				stack.push_back(t);
			} else {
				output.push_back(stack.back() );
				stack.pop_back();
				stack.push_back(t);
			}
		}
	}

	while(!stack.empty() ) {
		output.push_back(stack.back() );
		stack.pop_back();
	}

	return output;
}

int evalRpn(const Tokens &tokens) {
	std::vector<int> stack;

	for(auto it = tokens.begin(); it != tokens.end(); it++) {
		if(it->type == TokenType::Integer) {
			stack.push_back(it->value);
		} else {
			int op2 = stack.back();
			stack.pop_back();
			int op1 = stack.back();
			stack.pop_back();
			int value = 0;

			switch(it->value) {
				case '+':
					value = op1 + op2;
					break;
				case '-':
					value = op1 - op2;
					break;
				case '*':
					value = op1 * op2;
					break;
				case '/':
					value = op1 / op2;
					break;
			}

			stack.push_back(value);
		}
	}

	return stack.front();
}

Tokens tokenize(const std::string &input) {
	auto current = input.begin();
	auto start = current;
	auto binOp = binaryOperators.begin();
	Tokens tokens;
	Token token;

	TokenType expected = TokenType::Integer;

	auto expecting = [&](TokenType type) {
		if(type == expected) return true;
		printCarat(std::distance(input.begin(), current) );
		return false;
	};

	for(; current < input.end(); current++) {
		while(current != input.end() && std::isspace(*current) ) ++current;
		if(current == input.end() ) break;

		start = current;
		if(std::isdigit(*current) || *current == '-' && (tokens.empty() || tokens.back().type != TokenType::Integer) ) {
			token.index = std::distance(input.begin(), current);
			if(!expecting(TokenType::Integer) ) {
				return Tokens();
			}
			++current;
			while(current != input.end() && std::isdigit(*current) ) ++current;
			token.value = std::stoi(std::string(start, current) );
			token.type = TokenType::Integer;
			tokens.push_back(token);
			--current;
			expected = TokenType::BinaryOperator;
			continue;
		} 

		binOp = std::find(binaryOperators.begin(), binaryOperators.end(), *current);
		if(binOp != binaryOperators.end() ) {
			if(!expecting(TokenType::BinaryOperator) ) {
				return Tokens();
			}
			token.value = *binOp;
			token.type = TokenType::BinaryOperator;
			token.index = std::distance(input.begin(), current);
			tokens.push_back(token);
			expected = TokenType::Integer;
			continue;
		}

	}

	if(!tokens.empty() && expected == TokenType::Integer) {
		printCarat(tokens.back().index);
		return Tokens();
	}

	return tokens;
}

int eval(Node *node) {
	if(node->iterator->type == TokenType::BinaryOperator) {
		int op1 = eval(node->left);
		int op2 = eval(node->right);
		int value = 0;

		switch(node->iterator->value) {
			case '+':
				value = op1 + op2;
				break;
			case '-':
				value = op1 - op2;
				break;
			case '*':
				value = op1 * op2;
				break;
			case '/':
				value = op1 / op2;
				break;
		}

		return value;
	}

	return node->iterator->value;
}

void visit(Node *node) {
	std::cout << node->iterator->value << '\n';
	if(node->left && node->right) {
		//std::cout << node->left->iterator->value << ' ' << node->right->iterator->value << '\n';
		std::cout << "Left: ";
		visit(node->left);
		std::cout << "Right: ";
		visit(node->right);
	}
}

void destroy(Node *node) {
	if(node->left) {
		destroy(node->left);
	}
	if(node->right) {
		destroy(node->right);
	}
	delete node;
}

int buildTree(Tokens &tokens) {
	if(tokens.empty() ) return 0;

	auto it = std::find_if(tokens.begin(), tokens.end(), [](const Token t) {
		return t.type == TokenType::BinaryOperator && !highPrecedence(t.value);
	});
	if(it == tokens.end() ) {
		for(it = std::prev(tokens.end() ); it != tokens.begin(); it--) {
			if(it->type == TokenType::BinaryOperator
				&& highPrecedence(it->value) ) break;
		}
		if(it == tokens.begin() ) it = tokens.end();
	}

	if(it == tokens.end() ) return tokens.front().value;

	Node *root = new Node{it};
	buildTree(tokens.begin(), tokens.end(), root);
	//visit(root);
	int value = eval(root);
	destroy(root);

	return value;
}

int parse(const std::string &str) {
	Tokens tokens = tokenize(str);
	printTokens(tokens);
	//tokens = shuntingYard(tokens);
	//if(tokens.empty() ) return std::numeric_limits<int>::max();

	//return evalRpn(tokens);
	return buildTree(tokens);
}

int main() {
	std::string input;
	std::cout << prefix;
	while(std::getline(std::cin, input) ) {
		int value = parse(input);
		std::cout << " = ";
		if(value == std::numeric_limits<int>::max() ) std::cerr << "ERROR" << '\n';
		else std::cerr << value << '\n';
		std::cout << prefix;
	}
}
