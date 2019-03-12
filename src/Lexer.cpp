#include "Lexer.hpp"

#include <bitset>

#include "Position.hpp"

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

	while (end.hasMore()) {
		const std::string currentToken{ Position::toString(begin, end + 1) };

		RegexMask nextInvalidExpressions;
		for (size_t i = 0; i < TOKEN_COUNT; ++i) {
			nextInvalidExpressions.set(i, 
				!std::regex_match(currentToken, m_regexes[i]));
		}

		if (nextInvalidExpressions.all()) {
			if (begin != end) {
				auto tokenType = TokenType::Invalid;

				for (size_t i = 0; i < TOKEN_COUNT; ++i) {
					if (!invalidExpressions.test(i)) {
						tokenType = static_cast<TokenType>(i);
						break;
					}
				}

				if (tokenType != TokenType::Invalid) {
					result.emplace_back(Token{ Position::toString(begin, end), tokenType});
				}

				invalidExpressions.reset();
				begin = end;
				continue;
			}
		}

		invalidExpressions = nextInvalidExpressions;
		++end;
	}

	return result;
}
