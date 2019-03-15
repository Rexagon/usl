#pragma once

#include "Term.hpp"

namespace app
{
	class Parser final
	{
	public:
		void parse(const std::vector<Token>& tokens);
	};
}
