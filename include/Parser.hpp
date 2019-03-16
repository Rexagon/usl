#pragma once

#include "EarleyItem.hpp"
#include "ParserGrammar.hpp"

namespace app
{
	class Parser final
	{
		using StateSet = std::vector<EarleyItem>;
		using StateSets = std::vector<StateSet>;

	public:
		Parser();

		void parse(const std::vector<Token>& tokens);

	private:
		static void scan(StateSets& s, size_t i, size_t j, const Token& token);
		static void predict(StateSets& s, size_t i, size_t j, const Grammar& g);
		static void complete(StateSets& s, size_t i, size_t j);

		static void tryEmplace(StateSet& stateSet, const EarleyItem& item);

		const Grammar& m_grammar;
	};
}
