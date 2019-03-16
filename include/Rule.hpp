#pragma once

#include <variant>

#include "Grammar.hpp"

namespace app
{
	struct Term final
	{
		bool operator==(const Term& other) const;

		TokenType type;
	};

	struct NonTerm final
	{
		bool operator==(const NonTerm& other) const;

		std::string name;
	};

	using RuleVariant = std::variant<Term, NonTerm>;

	struct RuleSet final
	{
		RuleSet() = default;

		bool operator==(const RuleSet& other) const;
		bool operator!=(const RuleSet& other) const;

		std::vector<RuleVariant> rules;
	};

	class EarleyState final
	{
	public:
		enum class Type
		{
			Term,
			NonTerm,
			Null,
		};

		EarleyState(const EarleyState& state, size_t n = 1);
		EarleyState(std::string_view name, const RuleSet& set, size_t origin, size_t next = 0);

		Type getNextType() const;
		const Term* getNextTerm() const;
		const NonTerm* getNextNonTerm() const;

		std::string_view getName() const;

		size_t getOrigin() const;
		size_t getNextPosition() const;

		size_t getEndPosition() const;

		bool operator==(const EarleyState& other) const;
		bool operator!=(const EarleyState& other) const;

	private:
		const RuleSet& m_item;

		std::string_view m_name;
		size_t m_origin = 0;
		size_t m_next = 0;
	};

	class RuleCases final
	{
	public:
		RuleCases() = default;
		RuleCases(const Term& t);
		RuleCases(const NonTerm& t);
		RuleCases(const RuleSet& t);

		void setName(const std::string& name);

		std::vector<EarleyState> generateStates(size_t begin, size_t next = 0) const;

		RuleCases operator|(const RuleSet& r) const;
		RuleCases operator|(const RuleVariant& r) const;
		friend RuleCases operator|(const RuleSet& l, const RuleSet& r);
		friend RuleCases operator|(const RuleSet& l, const RuleVariant& r);

	private:
		std::string m_name;
		std::vector<RuleSet> m_sets;
	};

	RuleSet operator>>(const RuleSet& l, const RuleVariant& r);
	RuleSet operator>>(const RuleVariant& l, const RuleSet& r);
	RuleSet operator>>(const RuleVariant& l, const RuleVariant& r);
}
