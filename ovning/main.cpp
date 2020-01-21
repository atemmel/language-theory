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
	bool visited = false;
};

using Tokens = std::vector<Token>;
using TokenIterator = Tokens::iterator;

struct Node {
	Node(TokenIterator it) : iterator(it) {
		it->visited = true;
		std::cout << "Node created: " << it->value << "\t\t" << it->index << '\t' << it->visited << '\n';
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
	switch(c) {
		case '*':
		case '/':
			return true;
		default:
			return false;
	}
	return false;
}

void printTokens(const Tokens &tokens) {
	for(auto t : tokens) {
		std::cout << t.value << ' ';
	}
	std::cout << "\n\n";
}

template<typename BidirectionalIterator>
void buildTree(const BidirectionalIterator first, const BidirectionalIterator last, Node *node) {
	if(first == last) return;

	//Seek right
	auto rightIt = std::next(node->iterator);
	auto binOpLowPrec = last;
	auto binOp = last;
	auto integer = last;
	for(; rightIt < last; rightIt++) {
		//std::cout << "VAL: " << rightIt->value << '\n';
		if(binOpLowPrec == last 
				&& !rightIt->visited
				&& rightIt->type == TokenType::BinaryOperator 
				&& !highPrecedence(rightIt->value) ) {
			binOpLowPrec = rightIt;
			break;
		}
		if(binOp == last 
				&& !rightIt->visited
				&& rightIt->type == TokenType::BinaryOperator
				&& highPrecedence(rightIt->value) ) {
			binOp = rightIt;
			//break;
		}
		if(integer == last 
				&& !rightIt->visited
				&& rightIt->type == TokenType::Integer) {
			integer = rightIt;
			//break;
		}
	}

	auto it = last;
	if(binOpLowPrec != last) {
		it = binOpLowPrec;
	} else if(binOp != last) {
		it = binOp;
	} else if(integer != last) {
		it = integer;
	}

	if(it != last) {
		std::cout << "Right ";
		node->right = new Node(it);
		buildTree(node->iterator, last, node->right);
	}

	//Seek left
	auto leftIt = std::prev(node->iterator);
	for(; leftIt != first; leftIt--) {
		if(leftIt->type == TokenType::BinaryOperator && !leftIt->visited) {
			node->left = new Node{leftIt};
			buildTree(first, leftIt, node->left);
			//std::cout << "DIST: " << std::distance(node->iterator, last) << '\n';
			std::cout << "BUILDING NEW TREE RIGHT: " << node->left->iterator->value << '\n';
			buildTree(first, last, node->left);
			return;
		}
	}

	if(leftIt == first) {
		//std::cout << "Ran out of space\n";
		while(leftIt != last && leftIt->visited) ++leftIt;
		if(leftIt == last) return;
		std::cout << "Left ";
		node->left = new Node{leftIt};
	}

	/*
	if(rightIt == last) {
		rightIt = std::next(node->iterator);
		for(; rightIt < last; rightIt++) {
			if(rightIt->type == TokenType::BinaryOperator) {
				node->right = new Node{rightIt};
				buildTree(std::next(node->iterator), last, node->right);
				return;
			}
		}
	}

	if(rightIt == last) {
		rightIt = std::next(node->iterator);
		for(; rightIt < last; rightIt++) {
			if(rightIt->type == TokenType::Integer) {
				node->right = new Node{rightIt};
				buildTree(std::next(node->iterator), last, node->right);
				return;
			}
		}
	}

	if(rightIt == last) {
		node->right = new Node{std::prev(rightIt)};
	}
	*/
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
		visit(node->left);
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
		it = std::find_if(tokens.begin(), tokens.end(), [](const Token t) {
			return t.type == TokenType::BinaryOperator;
		});
	}

	if(it == tokens.end() ) return tokens.front().value;

	Node *root = new Node{it};
	buildTree(tokens.begin(), tokens.end(), root);
	visit(root);
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
