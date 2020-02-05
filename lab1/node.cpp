#include "node.hpp"

std::ostream &operator<<(std::ostream &os, Span s) {
	return os << std::distance(s.first, s.last);
}

Spans removeSubsets(const Spans &spans, const Spans &filter) {
	if(filter.empty() ) return spans;
	Spans output;
	for(auto s : spans) {
		for(auto t : filter) {
			if(!s.isSubset(t) ) {
				output.push_back(s);
			}
		}
	}
	return output;
}

Spans merge(const Spans &spans1, const Spans &spans2) {
	Spans clean1 = removeSubsets(spans1, spans2);
	Spans clean2 = removeSubsets(spans2, clean1);
	Spans set(clean1.size() + clean2.size() );
	std::merge(clean1.cbegin(), clean1.cend(), clean2.cbegin(),
			clean2.cend(), set.begin() );
	return set;
}

void Node::addChild(Child child) {
	children.push_back(std::move(child) );
}

Child Parser::parseTokens(Tokens &&tokens) {
	this->tokens = std::move(tokens);
	this->iterator = this->tokens.begin();
	return buildProgram();
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

Child Parser::buildProgram() {
	auto program = std::make_unique<NodeProgram>();
	for(;;) {
		Child child = buildExpression();
		if(!child) {
			child = buildSelectionGroup();
		}
		if(!child) {
			return nullptr;
		}
		program->addChild(std::move(child) );
		if(end() ) break;
	}
	return program;
}

Child Parser::buildExpression() {
	Child child = buildString();
	if(child) {
		Child parent = buildInsensitive(child);
		if(!parent) {
			parent = buildRepeated(child);
		}
		if(!parent) {
			parent = buildEither(child);
		}
		if(parent) {
			return parent;
		}
		return child;
	} 

	child = buildWildcard();
	if(child) {
		Child parent = buildRepeated(child);
		if(!parent) {
			parent = buildCounter(child);
		}
		if(!parent) {
			parent = buildEither(child);
		}
		if(parent) {
			return parent;
		}
		return child;
	}

	child = buildGrouping();
	if(child) {
		return child;
	}

	return nullptr;
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
	Child child = buildExpression();
	if(!child || !getIf(TokenType::RParen) ) {
		return nullptr;
	}
	Child parent = std::make_unique<NodeGrouping>();
	parent->addChild(std::move(child) );

	Child grandparent = buildRepeated(parent);
	if(!grandparent) {
		grandparent = buildInsensitive(parent);
	}
	if(!grandparent) {
		grandparent = buildEither(parent);
	}
	if(grandparent) {
		return grandparent;
	}
	return parent;
}

Child Parser::buildInsensitive(Child &child) {
	if(!getIf(TokenType::CaseInsensitive) ) return nullptr;
	Child caseIns = std::make_unique<NodeCaseInsensitive>();
	caseIns->addChild(std::move(child) );

	Child grandparent = buildRepeated(caseIns);
	if(!grandparent)
		grandparent = buildEither(caseIns);
	if(grandparent) {
		return grandparent;
	}
	return caseIns;
}

Child Parser::buildRepeated(Child &child) {
	if(!getIf(TokenType::Repeated) ) return nullptr;
	Child repeat = std::make_unique<NodeRepeated>();
	repeat->addChild(std::move(child) );

	Child grandparent = buildInsensitive(repeat);
	if(!grandparent)
		grandparent = buildEither(repeat);
	if(grandparent) {
		return grandparent;
	}
	return repeat;
}

Child Parser::buildEither(Child &child) {
	if(!getIf(TokenType::Either) ) return nullptr;
	Child either = std::make_unique<NodeEither>();
	either->addChild(std::move(child) );
	either->addChild(std::move(buildExpression() ) );
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
