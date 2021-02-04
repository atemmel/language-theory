#include "node.hpp"

unsigned Scope::depth = 0;

State state;

Scope::Scope() {
	++depth;
}

Scope::~Scope() {
	--depth;
}

void Node::addChild(Child child) {
	children.push_back(std::move(child) );
}

bool NodeSequence::eval() {
	return evalPlane(children);
}

bool evalPlane(std::vector<Child> &plane) {
ALGO_START:
	auto old = state.resEnd;
	bool res = plane.front()->eval();
	while(!res && state.strEnd != state.resEnd) {
		state.resEnd = ++state.resBegin;
		res = plane.front()->eval();
	}
	auto multiplePlaneCheck = state.resBegin;

	for(int i = 1; i < plane.size() && res; i++) {
		if(state.wasGreedy) {
			state.wasGreedy = false;
			auto back = state.strEnd;
			state.resEnd = back;
			res = plane[i]->eval();
			while(!res && old != state.resEnd) {
				state.resEnd = --back;
				res = plane[i]->eval();
			}
			if(!res) {
				state.resEnd = state.resBegin = state.strEnd;
				return false;
			} 
			if(!state.groupings.empty()) {
				state.groupings[state.lastGrouping].last = back;
			}
			old = state.resEnd;
		} else {
			old = state.resEnd;
			res = plane[i]->eval();
		}
		
	}

	if(!res && state.resEnd != state.strEnd) {
		goto ALGO_START;
	}

	if(multiplePlaneCheck != state.resBegin) {
		return false;
	}

	// Cutoff error fix
	if(!res) {
		state.resBegin = state.strEnd;
	}
	return res;
}

bool NodeSelectionGroup::eval() {
	bool res = children.front()->eval();
	if(res && value > 0) {
		state.resBegin = state.groupings[value - 1].first;
		state.resEnd = state.groupings[value - 1].last;
	} else if(!res) {
		state.resBegin = state.resEnd = state.strEnd;
	}
	return res;
}

bool NodeGrouping::eval() {
	auto start = state.resEnd;
	bool res = children.front()->eval();
	state.groupings[index] = res ? Span{start, state.resEnd} 
		: Span{state.strEnd, state.strEnd};
	state.lastGrouping = index;
	return res;
}

bool NodeCaseInsensitive::eval() {
	state.caseInsDepth++;
	bool result = children.front()->eval();
	state.caseInsDepth--;
	return result;
}

bool NodeRepeated::eval() {
	state.cameFromWildcard = false;
	bool result = children.front()->eval();
	if(!result) {
		return false;
	}
	auto prev = std::prev(state.resEnd);
	if(*state.resEnd == *prev) {
		while(state.resEnd < state.strEnd && *state.resEnd == *prev) {
			state.resEnd++;
		}
		return true;
	} else if(state.cameFromWildcard) {
		state.cameFromWildcard = false;
		state.wasGreedy = true;
		state.resEnd = state.strEnd;
	}
	return true;
}

bool NodeEither::eval() {
	State save = state;
	bool lhsSuccess = children.front()->eval();
	State lhsState = state;
	state = save;
	bool rhsSuccess = children.back()->eval();
	State rhsState = state;

	if(!lhsSuccess && !rhsSuccess) {
		state = rhsState.resEnd < lhsState.resEnd ? rhsState : lhsState;
		return false;
	} else if(lhsSuccess && !rhsSuccess) {
		state = lhsState;
		return true;
	} else if(!lhsSuccess && rhsSuccess) {
		state = rhsState;
		return true;
	}

	// Both suceeded, find out which one progressed the furthest
	state = rhsState.resBegin <= lhsState.resBegin ? rhsState : lhsState;
	return true;
}

bool NodeCounter::eval() {
	if(std::distance(state.resEnd, state.strEnd) < value) {
		return false;
	}
	for(int i = 0; i < value; i++) {
		if(!children.front()->eval() ) {
			return false;
		}
	}
	return true;
}

bool NodeString::eval() {
	for(auto c : value) {
		const bool isUpper = state.caseInsDepth == 0;
		if(isUpper) {
			if(state.resEnd == state.strEnd || *state.resEnd != c) {
				return false;
			}
		} else if(std::toupper(*state.resEnd) != std::toupper(c)) {
			return false;
		}
		state.resEnd++;
	}
	return true;
}

bool NodeWildcard::eval() {
	if(state.resEnd == state.strEnd) {
		return false;
	}
	state.resEnd++;
	state.cameFromWildcard = true;
	return true;
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


		Child seq = std::make_unique<NodeSequence>();
		Child binexpr = buildBinExpression(seq);
		if(binexpr)  {
			binexpr->children.front()->addChild(std::move(child) );
			child = std::move(binexpr);

		} 
		sequence->addChild(std::move(child) );
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
	parent->index = state.groupings.size();
	state.groupings.emplace_back();

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
