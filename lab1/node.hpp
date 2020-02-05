#pragma once
#include "tokenizer.hpp"

#include <algorithm>
#include <memory>
#include <vector>
#include <utility>

class Node;
using Child = std::unique_ptr<Node>;
using Iterator = std::string::const_iterator;

struct Span { 
	Iterator first, last; 
	bool operator<(Span rhs) const {
		return first < rhs.first;
	}
	bool operator==(Span rhs) const {
		return first == rhs.first && last == rhs.last;
	}
	bool isSubset(Span span) const {
		return first >= span.first && last <= span.last;
	}
};

std::ostream &operator<<(std::ostream &os, Span s);

using Spans = std::vector<Span>;

Spans removeSubsets(const Spans &spans, const Spans &filter);
Spans merge(const Spans &spans1, const Spans &spans2);

class Node {
public:
	virtual void print() = 0;
	virtual Spans eval(Iterator first, Iterator last) = 0;
	void addChild(Child child);
	std::vector<Child> children;
};

class NodeProgram : public Node {
public:
	void print() override { std::cout << "Program\n"; }
	Spans eval(Iterator first, Iterator last) override { 
		Spans superset;
		for(auto &child : children) {
			Spans subset = child->eval(first, last);
			superset = merge(superset, subset);
		}
		return superset;
	}
};

class NodeSelectionGroup : public Node {
public:
	void print() override { std::cout << "SelectionGroup : " << value << '\n'; }
	Spans eval(Iterator first, Iterator last) override { 
		return Spans();
	}
	int value = 0;
};

class NodeGrouping : public Node {
public:
	void print() override { std::cout << "Grouping\n"; }
	Spans eval(Iterator first, Iterator last) override { 
		return Spans(); 
	}
};

class NodeCaseInsensitive: public Node {
public:
	void print() override { std::cout << "CaseInsensitive\n"; }
	Spans eval(Iterator first, Iterator last) override { 
		return Spans(); 
	}
};

class NodeRepeated: public Node {
public:
	void print() override { std::cout << "Repeated\n"; }
	Spans eval(Iterator first, Iterator last) override { 
		Spans spans = children.front()->eval(first, last);
		Spans output;

		//Edge case
		if(spans.size() == std::distance(first, last) ) {
			output.push_back({first, last});
			return output;
		}

		for(auto s : spans) {
			if(s.last != last && *std::prev(s.last) == *s.last++) {
				output.push_back(s);
			}
		}
		return output; 
	}
};

class NodeEither: public Node {
public:
	void print() override { std::cout << "Either\n"; }
	Spans eval(Iterator first, Iterator last) override { 
		Spans sum;
		for(auto &child : children) {
			Spans result = child->eval(first, last);
			sum = merge(sum, result);
		}
		sum.erase(std::unique(sum.begin(), sum.end() ), sum.end() );
		return sum; 
	}
};

class NodeCounter : public Node {
public:
	void print() override { std::cout << "Counter : " << value << '\n'; }
	Spans eval(Iterator first, Iterator last) override { return Spans(); }
	int value = 0;
};

class NodeString: public Node {
public:
	NodeString(const std::string &str) : value(str) {};
	void print() override { std::cout << "String : " << value << '\n'; }
	Spans eval(Iterator first, Iterator last) override { 
		Spans matches;
		for(; first <= last; first++) {
			auto it = std::search(first, last, std::boyer_moore_searcher(
			   value.cbegin(), value.cend() ) );
			if(it != last) {
				matches.push_back({it, it + value.size()} );
			}
			first = it;
		}
		return matches;
	}
	std::string value;
};

class NodeWildcard: public Node {
public:
	void print() override { std::cout << "Wildcard\n"; }
	Spans eval(Iterator first, Iterator last) override { 
		Spans result;
		for(; first != last; first++) {
			result.push_back({first, std::next(first) });
		}
		return result; 
	}
};

class Parser {
public:
	Child parseTokens(Tokens &&tokens);
private:
	bool end() const;
	Token *getIf(TokenType::Type token);
	Child buildProgram();
	Child buildExpression();
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
};
