#include "node.hpp"

unsigned Scope::depth = 0;

Scope::Scope() {
	++depth;
}

Scope::~Scope() {
	--depth;
}

namespace State {
	bool justDidGroup = false;
	unsigned index = 0;
	unsigned justDidIndex = -1;
	unsigned caseInsDepth = 0;
	Iterator strBegin;
	Iterator strEnd;
	std::vector<Span> groupings;
};

Span::operator bool() const {
	return std::distance(first, last) != 0;
}

bool Span::operator<(Span rhs) const {
	return first < rhs.first;
}

bool Span::operator==(Span rhs) const {
	return first == rhs.first
		&& last == rhs.last;
}

void Node::addChild(Child child) {
	children.push_back(std::move(child) );
}

Span NodeSequence::eval(Span span) {
	Span lhs = children.front()->eval(span);
	auto checkState = [](Iterator newLast, Iterator totalLast) {
		if(State::justDidGroup && State::justDidIndex == State::index) {
			State::justDidGroup = false;
			if(State::groupings[State::index].first <= newLast) {
				State::groupings[State::index].last = newLast;
			} else {
				State::groupings[State::index].last = State::groupings[State::index].first
					= totalLast;
			}
		}
	};

	auto die = [](Span newSpan) {
		if(State::justDidGroup) {
			State::justDidGroup = false;
			State::groupings[State::index] = newSpan;
		}
	};

	if(!lhs || children.size() == 1) {
		return lhs;
	}

	//Edge case, ".*str"
	if(lhs.last == span.last && lhs.first < lhs.last) {
		Span rhs = children.back()->eval(span);
		if(!rhs) {
			die(rhs);
			return {
				span.last,
				span.last
			};
		}

		//Edge case, ".*Waterloo"
		if(lhs.first == rhs.first) {
			rhs = children.back()->eval({rhs.last, span.last});
			if(!rhs) {
				die(rhs);
				return rhs;
			}
		}

		Span prev;
		while(rhs) {
			prev = rhs;
			rhs = children.back()->eval({rhs.last, span.last});
		}

		checkState(prev.first, span.last);
		return {lhs.first, prev.last};
	}

	span.first = lhs.last;

	Span rhs = children.back()->eval(span);
	if(!rhs) {
		return rhs;
	}


	//Edge case, "str.*"
	if(rhs == span) {
		return {lhs.first, span.last};
	}


	for(; rhs; rhs = children.back()->eval({rhs.last, span.last} ) ) {
		for(; lhs; lhs = children.front()->eval({lhs.last, span.last}) ) {
			if(lhs.last == rhs.first) {
				return {
					lhs.first,
					rhs.last
				};
			}
		}
		lhs = children.front()->eval(span);
	}

	return {
		span.last,
		span.last
	};
}

Span NodeSelectionGroup::eval(Span span) {
	if(value == 0) {
		return children.front()->eval(span);
	} else if(value > State::groupings.size() ) {
		return {
			span.last,
			span.last
		};
	}

	State::groupings[value - 1] = {span.last, span.last};

	State::index = value - 1;
	Span result = children.front()->eval(span);

	return State::groupings[value - 1];
}

Span NodeGrouping::eval(Span span) {
	Span result = children.front()->eval(span);
	State::groupings[index] = result;
	State::justDidGroup = true;
	State::justDidIndex = index;
	return result;
}

Span NodeCaseInsensitive::eval(Span span) {
	State::caseInsDepth++;
	Span output = children.front()->eval(span);
	State::caseInsDepth--;
	return output;
}

Span NodeRepeated::eval(Span span) {
	Span total = children.front()->eval(span);
	while(total) {
		span.first = total.last;
		Span next = children.front()->eval(span);
		int i = 0;
		for(;next; i++) {
			if(next.first == total.last) {
				span.first = total.last = next.last;
			} else {
				break;
			}

			next = children.front()->eval(span);
		}

		if(i > 0) {
			return total;
		}

		total = children.front()->eval(span);
	}

	return {
		span.last,
		span.last
	};
}

Span NodeEither::eval(Span span) {
	Span lhs = children.front()->eval(span);
	Span rhs = children.back()->eval(span);

	if(lhs) {
		if(rhs) {
			return lhs < rhs ? lhs : rhs;
		} else {
			return lhs;
		}
	}

	if(rhs) {
		return rhs;
	}

	return {
		span.last,
		span.last
	};
}

Span NodeCounter::eval(Span span) {
	Span total = children.front()->eval(span);

	if(!total) {
		return total;
	}

	span.first = total.last;
	int i = 1;
	for(; i < value; i++) {
		Span part = children.front()->eval(span);
		if(!part) {
			return part;
		}
		if(part.first != total.last) {
			return eval({total.last, span.last});
		}
		span.first = total.last = part.last;
	}

	if(i == value) {
		return total;
	}

	return {
		span.last,
		span.last
	};
}


Span NodeString::eval(Span span) {
	std::string lower;
	Span input = span;
	if(State::caseInsDepth > 0) {
		std::transform(value.begin(), value.end(), value.begin(), ::tolower);
		lower.assign(span.first, span.last);
		std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
		input = {
			lower.cbegin(), 
			lower.cend()
		};
	}

	auto it = std::search(input.first, input.last,
			std::boyer_moore_searcher(
				value.begin(), value.end() ) );
	if(State::caseInsDepth > 0) {
		it = span.first + std::distance(lower.cbegin(), it);
	}

	if(it == span.last) {
		return {span.last, span.last};
	}

	return {it, it + value.size() };
}

Span NodeWildcard::eval(Span span) {
	if(!span) {
		return span;
	}
	return {span.first, std::next(span.first) };
}

Child Parser::parseTokens(Tokens &&tokens) {
	this->tokens = std::move(tokens);
	this->iterator = this->tokens.begin();
	auto seq = buildSequence();
	if(end() ) {
		return seq;
	}
	return nullptr;
}

void Parser::printErr() const {
	std::cerr << iterator - tokens.begin() << '\n';
}

bool Parser::end() const {
	return iterator == tokens.end();
}

Token *Parser::getIf(TokenType::Type token) {
	if(iterator == tokens.end() || iterator->type != token) {
		return nullptr;
	}
	return &*(iterator++);
}

Child Parser::buildSequence() {
	Child sequence = std::make_unique<NodeSequence>();
	while(!end() ) {
		if(sequence->children.size() == 2) {
			Child parent = std::make_unique<NodeSequence>();
			parent->addChild(std::move(sequence) );
			sequence = std::move(parent);

		}
		Child child = buildValue();
		if(!child) {
			child = buildSelectionGroup();
			if(child) {
				if(!end() ) {
					return nullptr;
				}
				//Lite bakvänt, men men
				child->addChild(std::move(sequence) );
				return child;
			}
		}

		if(!child) {
			if(sequence->children.empty() ) {
				return nullptr;
			}
			return sequence;
		}

		Child unexpr = buildUnExpression(child);
		while(unexpr) {
			child = std::move(unexpr);
			unexpr = buildUnExpression(child);
		}

		sequence->addChild(std::move(child) );

		Child binexpr = buildBinExpression(sequence);
		if(binexpr)  {
			return binexpr;
		}
	}

	if(sequence->children.empty() ) {
		return nullptr;
	}
	return sequence;
}

Child Parser::buildBinExpression(Child &child) {
	return buildEither(child);
}

Child Parser::buildUnExpression(Child &child) {
	Child parent = buildInsensitive(child);
	if(parent) {
		return parent;
	}

	if(mayStar) {
		parent = buildRepeated(child);
		if(parent) {
			mayStar = false;
			return parent;
		}
	}

	parent = buildCounter(child);
	if(parent) {
		return parent;
	}

	return nullptr;
}

Child Parser::buildValue() {
	Child child = buildString();
	if(!child) {
		child = buildWildcard();
	} 

	if(!child) {
		child = buildGrouping();
	}

	if(child) {
		mayStar = true;
	}

	return child;
}

Child Parser::buildSelectionGroup() {
	Token *token = getIf(TokenType::SelectionGroup);
	if(!token) {
		return nullptr;
	}
	auto selGroup = std::make_unique<NodeSelectionGroup>();
	try {
		selGroup->value = std::stoi(token->value);
	} catch(...) {
		return nullptr;
	}
	return selGroup;
}

Child Parser::buildGrouping() {
	if(!getIf(TokenType::LParen) ) {
		return nullptr;
	}
	Child child = buildSequence();
	if(!child || !getIf(TokenType::RParen) ) {
		return nullptr;
	}

	std::unique_ptr<NodeGrouping> parent(new NodeGrouping() );
	parent->index = State::groupings.size();
	State::groupings.emplace_back();

	parent->addChild(std::move(child) );

	return parent;
}

Child Parser::buildInsensitive(Child &child) {
	if(!getIf(TokenType::CaseInsensitive) ) return nullptr;
	Child caseIns = std::make_unique<NodeCaseInsensitive>();
	caseIns->addChild(std::move(child) );
	return caseIns;
}

Child Parser::buildRepeated(Child &child) {
	if(!getIf(TokenType::Repeated) ) return nullptr;
	Child repeat = std::make_unique<NodeRepeated>();
	repeat->addChild(std::move(child) );
	return repeat;
}

Child Parser::buildEither(Child &child) {
	if(!getIf(TokenType::Either) ) return nullptr;
	Child either = std::make_unique<NodeEither>();
	auto rhs = buildSequence();
	if(!rhs) {
		return nullptr;
	}
	either->addChild(std::move(child) );
	either->addChild(std::move(rhs) );
	return either;
}

Child Parser::buildCounter(Child &child) {
	Token *token = getIf(TokenType::Counter);
	if(!token) {
		return nullptr;
	}
	auto counter = std::make_unique<NodeCounter>();
	try {
		counter->value = std::stoi(token->value);
	} catch(...) {
		return nullptr;
	}
	counter->addChild(std::move(child) );
	return counter;
}

Child Parser::buildString() {
	Token *token = getIf(TokenType::String);
	if(!token) {
		return nullptr;
	}
	return std::make_unique<NodeString>(token->value);
}

Child Parser::buildWildcard() {
	return getIf(TokenType::Wildcard) ? std::make_unique<NodeWildcard>() : nullptr;
}
