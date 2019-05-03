#include "Lexer.hpp"

#include <bitset>

#include "Position.hpp"
#include <iostream>

app::Lexer::Lexer() :
	m_regexes(buildRegexes())
{
}

std::vector<app::Token> app::Lexer::run(std::string_view text) const
{
	std::vector<Token> result;

	RegexMask invalidExpressions;

	Position begin{ &*text.begin(), &*text.end() };
	Position end = begin;

	while (true) {
		const auto lastCharacter = !end.hasMore();

		RegexMask nextInvalidExpressions;
		if (lastCharacter) {
			nextInvalidExpressions.flip();
		}
		else {
			const std::string currentToken{ Position::toString(begin, end + 1) };

			for (size_t i = 0; i < lexer_grammar::TOKEN_COUNT; ++i) {
				nextInvalidExpressions.set(i,
					!std::regex_match(currentToken, m_regexes[i]));
			}
		}

		if (nextInvalidExpressions.all() && (begin != end))
		{
			size_t tokenType = lexer_grammar::Invalid;

			for (size_t i = 0; i < lexer_grammar::TOKEN_COUNT; ++i) {
				if (!invalidExpressions.test(i)) {
					tokenType = i;
					break;
				}
			}

			const auto token = Token{ tokenType, Position::toString(begin, end) };
			std::cout << "{ token: \"" << token.second <<
				"\",\n  type: " << static_cast<size_t>(token.first) <<
				" }\n\n";

			if (!lexer_grammar::isUseless(tokenType)) {
				result.emplace_back(tokenType, Position::toString(begin, end));
			}

			invalidExpressions.reset();
			begin = end;

			continue;
		}

		if (lastCharacter) {
			break;
		}

		invalidExpressions = nextInvalidExpressions;
		++end;
	}

	return result;
}
