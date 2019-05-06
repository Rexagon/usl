#pragma once

#include <variant>
#include <functional>

#include "LexerGrammar.hpp"

namespace app
{
	struct Term;
	struct NonTerm;
	struct RuleSet;

	class EarleyItem;

	using RuleVariant = std::variant<Term, NonTerm>;

	class Rules final
	{
	public:
	    Rules() = default;
	    explicit Rules(const std::vector<RuleSet>& ruleSets);

		void setName(size_t name);

		std::vector<EarleyItem> generateEarleyItems(size_t begin) const;

		const std::vector<RuleSet>& getRuleSets() const;

	private:
		size_t m_name = -1;
		std::vector<RuleSet> m_sets;
	};

	struct Term final
	{
		bool operator==(const Term& other) const;

		size_t type;
	};

	struct NonTerm final
	{
		bool operator==(const NonTerm& other) const;

		size_t name;
	};

	struct RuleSet final
	{
		RuleSet() = default;

		bool operator==(const RuleSet& other) const;

		std::vector<RuleVariant> rules;
		bool isImportant = true;
	};
}
