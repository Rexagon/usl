#pragma once

#include <unordered_map>

#include "EarleyItem.hpp"

namespace app
{
	class ParserGrammar final
	{
	public:
		std::vector<EarleyItem> generateStartingEarleyItems() const;
		std::vector<EarleyItem> generateEarleyItems(const std::string& name, size_t origin) const;

		const Rules& operator[](const std::string& name) const;

		static const ParserGrammar& create();

		static const std::string STARTING_RULE;

	private:
		ParserGrammar();
		void finalize();

		std::unordered_map<std::string, Rules> m_rules;
	};
}
