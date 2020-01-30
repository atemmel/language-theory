#include "tokenizer.hpp"

std::ostream &operator<<(std::ostream &os, const Token &t) {
	switch(t.type) {
		case TokenType::String:
			os << "String";
			break;
		case TokenType::LParen:
			os << "LParen";
			break;
		case TokenType::RParen:
			os << "RParen";
			break;
		case TokenType::Counter:
			os << "Counter";
			break;
		case TokenType::CaseInsensitive:
			os << "CaseInsensitive";
			break;
		case TokenType::Either:
			os << "Either";
			break;
		case TokenType::Repeated:
			os << "Repeated";
			break;
		case TokenType::SelectionGroup:
			os << "SelectionGroup";
			break;
		case TokenType::Wildcard:
			os << "Wildcard";
			break;
		case TokenType::Error:
			os << "Error";
			break;
	}
	os << ' ' << t.value;
	return os;
};

Tokens Tokenizer::tokenize(const std::string &str) {
	iterator = str.begin();
	end = str.end();

	while(!done() ) {
		char lexeme = get();
		Token token;

		switch(lexeme) {
			case '(':
				token.type = TokenType::LParen;
				break;
			case ')':
				token.type = TokenType::RParen;
				break;
			case '{':
				token = buildCounterToken();
				break;
			case '\\':
				token = buildEscapeToken();
				break;
			case '+':
				token.type = TokenType::Either;
				break;
			case '*':
				token.type = TokenType::Repeated;
				break;
			case '.':
				token.type = TokenType::Wildcard;
				break;
			default:
				token = buildStringToken();
				break;
		}

		tokens.push_back(token);
	}

	return tokens;
}

void Tokenizer::print() const {
	for(const auto &t : tokens) {
		std::cout << t << '\n';
	}
}

char Tokenizer::get() {
	return *iterator++;
}

char Tokenizer::peek() {
	return *iterator;
}

bool Tokenizer::done() {
	return iterator == end;
}

Token Tokenizer::buildCounterToken() {
	auto start = iterator;
	Token token;
	while(!done() && std::isdigit(get() ) );
	if(peek() == '}') {
		token.type = TokenType::Counter;
		token.value.assign(start, iterator);
	} else token.type = TokenType::Error;
	return token;
}

Token Tokenizer::buildEscapeToken() {
	Token token;
	char c = get();
	switch(c) {
		case 'I':
			token.type = TokenType::CaseInsensitive;
			break;
		case 'O':
			token.type = TokenType::SelectionGroup;
			break;
		default:
			token.type = TokenType::Error;
			break;
	}
	return token;
}

Token Tokenizer::buildStringToken() {
	Token token;
	auto start = std::prev(iterator);

	while(!done() ) {
		char c = peek();
		switch(c) {
			case '(':
			case ')':
			case '{':
			case '\\':
			case '+':
			case '*':
			case '.':
				token.value.assign(start, iterator);
				token.type = TokenType::String;
				return token;
		}
		get();
	}

	token.value.assign(start, iterator);
	token.type = TokenType::String;
	return token;
}
