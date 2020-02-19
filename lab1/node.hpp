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
	operator bool() const;
	bool operator<(Span rhs) const;
	bool operator==(Span rhs) const;
};

namespace State {
	extern unsigned caseInsDepth;
	extern Iterator strBegin;
	extern Iterator strEnd;
	extern std::vector<std::vector<Span> > groupings;
};

class Node {
public:
	virtual void print() = 0;
	virtual Span eval(Span span) = 0;
	void addChild(Child child);
	std::vector<Child> children;
};

class NodeSequence : public Node {
public:
	void print() override { std::cout << "Sequence\n"; }
	Span eval(Span span) override;
};

class NodeSelectionGroup : public Node {
public:
	void print() override { std::cout << "SelectionGroup : " << value << '\n'; }
	Span eval(Span span) override;
	int value = 0;
	int currentIndex = 0;
};

class NodeGrouping : public Node {
public:
	void print() override { std::cout << "Grouping\n"; }
	Span eval(Span span) override;
	int index = 0;
};

class NodeCaseInsensitive: public Node {
public:
	void print() override { std::cout << "CaseInsensitive\n"; }
	Span eval(Span span) override;
};

class NodeRepeated: public Node {
public:
	void print() override { std::cout << "Repeated\n"; }
	Span eval(Span span) override;
};

class NodeEither: public Node {
public:
	void print() override { std::cout << "Either\n"; }
	Span eval(Span span) override;
};

class NodeCounter : public Node {
public:
	void print() override { std::cout << "Counter : " << value << '\n'; }
	Span eval(Span span) override;
	int value = 0;
};

class NodeString: public Node {
public:
	NodeString(const std::string &str) : value(str) {};
	void print() override { std::cout << "String : " << value << '\n'; }
	Span eval(Span span) override;
	std::string value;
};

class NodeWildcard: public Node {
public:
	void print() override { std::cout << "Wildcard\n"; }
	Span eval(Span span) override;
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
