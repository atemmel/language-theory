#pragma once
#include "tokenizer.hpp"

#include <algorithm>
#include <memory>
#include <vector>
#include <utility>

class Node;
using Child = std::unique_ptr<Node>;
using Iterator = std::string::const_iterator;

struct Scope {
	Scope();
	~Scope();
	static unsigned depth;
};

struct Span {
	Iterator first, last;
};

struct State {
	std::vector<Span> groupings;
	unsigned caseInsDepth;
	unsigned lastGrouping = 0;
	Iterator strBegin;
	Iterator strEnd;
	Iterator resBegin;
	Iterator resEnd;
	bool cameFromWildcard = false;
	bool wasGreedy = false;
};

extern State state;

class Node {
public:
	virtual void print() = 0;
	virtual bool eval() = 0;
	void addChild(Child child);
	std::vector<Child> children;
};

bool evalPlane(std::vector<Child> &plane);

class NodeSequence : public Node {
public:
	void print() override { std::cout << "Sequence\n"; }
	bool eval() override;
};

class NodeSelectionGroup : public Node {
public:
	void print() override { std::cout << "SelectionGroup : " << value << '\n'; }
	bool eval() override;
	int value = 0;
};

class NodeGrouping : public Node {
public:
	void print() override { std::cout << "Grouping\n"; }
	bool eval() override;
	int index = 0;
};

class NodeCaseInsensitive: public Node {
public:
	void print() override { std::cout << "CaseInsensitive\n"; }
	bool eval() override;
};

class NodeRepeated: public Node {
public:
	void print() override { std::cout << "Repeated\n"; }
	bool eval() override;
};

class NodeEither: public Node {
public:
	void print() override { std::cout << "Either\n"; }
	bool eval() override;
};

class NodeCounter : public Node {
public:
	void print() override { std::cout << "Counter : " << value << '\n'; }
	bool eval() override;
	int value = 0;
};

class NodeString: public Node {
public:
	NodeString(const std::string &str) : value(str) {};
	void print() override { std::cout << "String : " << value << '\n'; }
	bool eval() override;
	std::string value;
};

class NodeWildcard: public Node {
public:
	void print() override { std::cout << "Wildcard\n"; }
	bool eval() override;
};

class Parser {
public:
	Child parseTokens(Tokens &&tokens);
	void printErr() const;
private:
	bool end() const;
	Token *getIf(TokenType::Type token);
	Child buildSequence();
	Child buildBinExpression(Child &child);
	Child buildUnExpression(Child &child);
	Child buildValue();
	Child buildSelectionGroup();
	Child buildGrouping();
	Child buildInsensitive(Child &child);
	Child buildRepeated(Child &child);
	Child buildEither(Child &child);
	Child buildCounter(Child &child);
	Child buildString();
	Child buildWildcard();

	Tokens tokens;
	TokenIterator iterator;
	bool mayStar = true;
};
