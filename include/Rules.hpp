#pragma once

#include <variant>

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
		Rules(const Term& t);
		Rules(const NonTerm& t);
		Rules(const RuleSet& t);
		Rules(const std::vector<RuleSet>& t);

		void setName(size_t name);

		std::vector<EarleyItem> generateEarleyItems(size_t begin) const;

		const std::vector<RuleSet>& getRuleSets() const;

		Rules operator|(const RuleSet& r) const;
		Rules operator|(const RuleVariant& r) const;
		friend Rules operator|(const RuleSet& l, const RuleSet& r);
		friend Rules operator|(const RuleSet& l, const RuleVariant& r);

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

	Rules operator|(const RuleVariant& l, const RuleSet& r);
	Rules operator|(const RuleVariant& l, const RuleVariant& r);

	RuleSet operator>>(const RuleSet& l, const RuleVariant& r);
	RuleSet operator>>(const RuleVariant& l, const RuleSet& r);
	RuleSet operator>>(const RuleVariant& l, const RuleVariant& r);

	RuleSet operator<<(const RuleSet& l, bool important);
	RuleSet operator<<(const RuleVariant& l, bool important);
}
