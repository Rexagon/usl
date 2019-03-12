#pragma once

#include "Grammar.hpp"

namespace app
{
	class Lexer final
	{
	public:
		Lexer();

		void run(std::string_view text);

	private:
		const RegexArray& m_regexes;
	};
}
