#pragma once

#include <unordered_map>

#include "Rules.hpp"

namespace app
{
	using Grammar = std::unordered_map<std::string, Rules>;

	constexpr auto STARTING_RULE = "sum";

	const Grammar& buildGrammar();
}
