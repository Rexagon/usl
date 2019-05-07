#pragma once

#include "ParserGrammar.hpp"

namespace app
{
	class Parser final
	{
		using StateSet = std::vector<EarleyItem>;
		using StateSets = std::vector<StateSet>;

	public:
		Parser();

		std::vector<ByteCodeItem> parse(const std::vector<Token>& tokens);

	private:
		void scan(size_t i, size_t j, const Token& token);
		void predict(size_t i, size_t j, const ParserGrammar& g);
		void complete(size_t i, size_t j);

		static void tryEmplace(StateSet& stateSet, const EarleyItem& item);

		StateSets m_stateSets;
		const ParserGrammar& m_grammar;
	};
}
