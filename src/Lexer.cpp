#include "Lexer.hpp"

#include <bitset>

#include "Position.hpp"

app::Lexer::Lexer() :
	m_regexes(buildRegexes())
{
}

void app::Lexer::run(std::string_view text)
{
	//TODO: return parsed tokens
	//TODO: fix last token parsing

	RegexMask invalidExpressions;

	Position begin{ &*text.begin(), &*text.end() };
	Position end = begin;

	while ((end + 1).hasMore()) {
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
					const std::string token{ Position::toString(begin, end) };
					printf("{ token: \"%s\",\n  type: %d }\n\n", token.c_str(), tokenType);
				}

				invalidExpressions.reset();
				begin = end;
				continue;
			}
		}

		invalidExpressions = nextInvalidExpressions;
		++end;
	}
}
