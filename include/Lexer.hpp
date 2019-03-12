#pragma once

#include "Grammar.hpp"

namespace app
{
	struct Token final
	{
		std::string_view text;
		TokenType type;
	};

	class Lexer final
	{
	public:
		Lexer();

		std::vector<Token> run(std::string_view text) const;

	private:
		const RegexArray& m_regexes;
	};
}
