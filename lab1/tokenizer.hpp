#pragma once
#include <vector>
#include <string>
#include <string_view>
#include <array>
#include <iostream>

namespace TokenType {
	using Type = int_fast32_t;
	constexpr Type String				= 1 << 0;
	constexpr Type LParen		 		= 1 << 1;
	constexpr Type RParen				= 1 << 2;
	constexpr Type Counter				= 1 << 3;
	constexpr Type CaseInsensitive		= 1 << 4;
	constexpr Type Either				= 1 << 5;
	constexpr Type Repeated				= 1 << 6;
	constexpr Type SelectionGroup		= 1 << 7;
	constexpr Type Wildcard				= 1 << 8;
	constexpr Type Error				= 1 << 9;
};


struct Token {
	TokenType::Type type;
	std::string value;
};

std::ostream &operator<<(std::ostream &os, const Token &token);

using Tokens = std::vector<Token>;
using TokenIterator = Tokens::iterator;

class Tokenizer {
public:
	Tokens tokenize(const std::string &str);
	void print() const;
private:
	using CStrIterator = std::string::const_iterator;

	char get();
	char peek();
	bool done();

	Token buildCounterToken();
	Token buildEscapeToken();
	Token buildStringToken();

	CStrIterator iterator;
	CStrIterator end;
	TokenType::Type expectedToken;
	Tokens tokens;
};
