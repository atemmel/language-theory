#pragma once
#include "tokenizer.hpp"

#include <memory>
#include <vector>
#include <utility>

class Node;
using Child = std::unique_ptr<Node>;

class Node {
public:
	virtual void eval() = 0;
	virtual void print() = 0;
	void addChild(Child child);
	std::vector<Child> children;
};

class NodeProgram : public Node {
public:
	void print() override { std::cout << "Program\n"; }
	void eval() override {};
};

class NodeGrouping : public Node {
public:
	void print() override { std::cout << "Grouping\n"; }
	void eval() override {};
};

class NodeCaseInsensitive: public Node {
public:
	void print() override { std::cout << "CaseInsensitive\n"; }
	void eval() override {};
};

class NodeRepeated: public Node {
public:
	void print() override { std::cout << "Repeated\n"; }
	void eval() override {};
};

class NodeEither: public Node {
public:
	void print() override { std::cout << "Either\n"; }
	void eval() override {};
};
class NodeString: public Node {
public:
	NodeString(const std::string &str) : value(str) {};
	void print() override { std::cout << "String : " << value << '\n'; }
	void eval() override {};
	std::string value;
};

class Parser {
public:
	Child parseTokens(Tokens &&tokens);
private:
	bool end() const;
	Token *getIf(TokenType::Type token);
	Child buildProgram();
	Child buildExpression();
	Child buildString(const std::string &string);

	Tokens tokens;
	TokenIterator iterator;
};
