#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>

constexpr std::string_view prefix = ">>>  ";

constexpr static std::array<char, 4> binaryOperators = {
	'+',
	'-',
	'*',
	'/'
};

constexpr static std::array<std::string_view, 2> tokenStrings = {
	"Integer",
	"BinaryOperator"
};

enum struct TokenType {
	Integer,
	BinaryOperator
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

template<typename ForwardIterator, typename UnaryPredicate>
ForwardIterator rfind(const ForwardIterator first, const ForwardIterator last, UnaryPredicate pred) {
	auto rend = std::prev(first);
	auto rbegin = std::prev(last);
	for(; rbegin != rend; rbegin--) {
		if(pred(*rbegin) ) return rbegin;
	}
	return last;
}

void printCarat(int index) {
	for(auto c : prefix) std::cerr << ' ';
	for(int i = 0; i < index; i++) std::cerr << ' ';
	std::cerr << "^\n";
}

bool highPrecedence(int c) {
	return c == '*' || c == '/';
}

float eval(const TokenIterator first, const TokenIterator last) {
	if(first == last) return 0.f;
	if(first + 1 == last) return first->value;

	auto it = rfind(first, last, [](const Token t) {
		return t.type == TokenType::BinaryOperator && !highPrecedence(t.value);
	});

	if(it == last) {
		it = rfind(first, last, [](const Token t) {
			return t.type == TokenType::BinaryOperator && highPrecedence(t.value);
		});
	}

	float op1 = eval(first, it);
	float op2 = eval(std::next(it), last);
	float value;

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

	return value;
}

float eval(Node *node) {
	if(node->iterator->type == TokenType::BinaryOperator) {
		float op1 = eval(node->left);
		float op2 = eval(node->right);
		float value = 0;

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


void printTokens(const Tokens &tokens) {
	for(auto t : tokens) {
		std::cout << (t.type == TokenType::Integer 
				? t.value : static_cast<char>(t.value) ) << ' ';
	}
	std::cout << "\n\n";
}

Node* buildTree(const TokenIterator first, const TokenIterator last) {
	if(first == last) return nullptr;
	if(first + 1 == last) return new Node{first};

	auto it = rfind(first, last, [](const Token t) {
		return t.type == TokenType::BinaryOperator && !highPrecedence(t.value);
	});

	if(it == last) {
		it = rfind(first, last, [](const Token t) {
			return t.type == TokenType::BinaryOperator && highPrecedence(t.value);
		});
	}

	Node *node = new Node{it};
	node->left = buildTree(first, it);
	node->right = buildTree(std::next(it), last);

	return node;
}

float buildTree(Tokens &tokens) {
	if(tokens.empty() ) return 0;
	auto root = buildTree(tokens.begin(), tokens.end() );
	float value = eval(root);
	destroy(root);

	return value;
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

float parse(const std::string &str) {
	Tokens tokens = tokenize(str);
	if(tokens.empty() ) return std::numeric_limits<float>::max();
	//printTokens(tokens);
	//return buildTree(tokens);
	return eval(tokens.begin(), tokens.end() );
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
