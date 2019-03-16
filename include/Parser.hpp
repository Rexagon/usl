#pragma once

#include <unordered_map>

#include "Rule.hpp"

namespace app
{
	class Parser final
	{
		using EarleySet = std::vector<EarleyState>;
		using StateGroups = std::vector<EarleySet>;
		using Grammar = std::unordered_map<std::string, RuleCases>;
	public:
		Parser();

		void parse(const std::vector<Token>& tokens);

	private:
		static void scan(StateGroups& s, size_t i, size_t j, const Token& token);
		static void predict(StateGroups& s, size_t i, size_t j, const Grammar& g);
		static void complete(StateGroups& s, size_t i, size_t j);

		static void tryEmplace(EarleySet& earleySet, const EarleyState& state);

		Grammar m_grammar;
	};
}
