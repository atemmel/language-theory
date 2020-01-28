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
}

void printTokens(const Tokens &tokens) {
	for(auto t : tokens) {
		std::cout << (t.type == TokenType::Integer 
				? t.value : static_cast<char>(t.value) ) << ' ';
	}
	std::cout << "\n\n";
}

void buildTree(const TokenIterator first, const TokenIterator last, Node *node) {
	if(first == last) return;

	auto leftIt = std::prev(node->iterator);
	for(; leftIt != first; leftIt--) {
		if(leftIt->type == TokenType::BinaryOperator
				&& highPrecedence(leftIt->value) ) {
			node->left = new Node{leftIt};
			buildTree(first, node->iterator, node->left);
			break;
		}
	}

	if(leftIt == first) {
		node->left = new Node{first};
	}

	auto rightIt = std::next(node->iterator);
	auto firstBinOp = last;
	for(; rightIt != last; rightIt++) {
		if(rightIt->type == TokenType::BinaryOperator
				&& !highPrecedence(rightIt->value) ) {
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
			node->right = new Node{std::prev(last) };
		} else {
			node->right = new Node{firstBinOp};
			buildTree(std::next(node->iterator), last, node->right);
		}
	}
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
		if(std::isdigit(*current) || (*current == '-' && (tokens.empty() || tokens.back().type != TokenType::Integer) ) ) {
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

float eval(Node *node) {
	if(node->iterator->type == TokenType::BinaryOperator) {
		float op1 = eval(node->left);
		float op2 = eval(node->right);
		//float op2 = eval(node->right, node->iterator->value == '-');
		float value = 0;

		switch(node->iterator->value) {
			case '+':
				value = op1 + op2;
				break;
			case '-':
				//value = neg ? op1 + op2 : op1 - op2;
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

	return static_cast<float>(node->iterator->value);
}

void visit(Node *node) {
	std::cout << node->iterator->value << '\n';
	if(node->left && node->right) {
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

float buildTree(Tokens &tokens) {
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
	float value = eval(root);
	destroy(root);

	return value;
}

float parse(const std::string &str) {
	Tokens tokens = tokenize(str);
	if(tokens.empty() ) return std::numeric_limits<float>::max();
	//printTokens(tokens);
	return buildTree(tokens);
}

int main() {
	std::string input;
	std::cout << prefix;
	while(std::getline(std::cin, input) ) {
		float value = parse(input);
		std::cout << " = ";
		if(value == std::numeric_limits<float>::max() ) std::cerr << "ERROR" << '\n';
		else std::cerr << value << '\n';
		std::cout << prefix;
	}
}
