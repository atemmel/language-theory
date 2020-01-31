#include "node.hpp"

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
	Child program = std::make_unique<NodeProgram>();
	for(;;) {
		Child expression = buildExpression();
		if(!expression) return nullptr;
		program->addChild(std::move(expression) );
		if(end() ) break;
	}
	return program;
}

Child Parser::buildExpression() {
	Token *token = nullptr;
	token = getIf(TokenType::String);
	if(token) {
		if(getIf(TokenType::CaseInsensitive) ) {
			Child caseIns = std::make_unique<NodeCaseInsensitive>();
			caseIns->addChild(std::move(buildString(token->value) ) );
			return caseIns;
		} else if(getIf(TokenType::Repeated) ) {
			Child repeated = std::make_unique<NodeRepeated>();
			repeated->addChild(std::move(buildString(token->value) ) );
			return repeated;
		} else if(getIf(TokenType::Either) ) {
			Child either = std::make_unique<NodeEither>();
			either->addChild(std::move(buildString(token->value) ) );
			either->addChild(std::move(buildExpression() ) );
			return either;
		}else if(end() ){
			return buildString(token->value);
		}
	}
	return nullptr;
}

Child Parser::buildString(const std::string &string) {
	return std::make_unique<NodeString>(string);
}
