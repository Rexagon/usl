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

	struct Translator final
	{
		std::function<void(ByteCode&, size_t)> func = [](ByteCode&, size_t) {};
		size_t span = 0;
	};

	struct RuleSet final
	{
		RuleSet() = default;

		bool operator==(const RuleSet& other) const;

		std::vector<RuleVariant> rules;
		std::optional<Translator> translator = std::nullopt;
		bool isImportant = true;
	};

	Rules operator|(const RuleVariant& l, const RuleSet& r);
	Rules operator|(const RuleVariant& l, const RuleVariant& r);

	RuleSet operator>>(const RuleSet& l, const RuleVariant& r);
	RuleSet operator>>(const RuleVariant& l, const RuleSet& r);
	RuleSet operator>>(const RuleVariant& l, const RuleVariant& r);

	template<bool V>
	struct Importance : std::bool_constant<V> {};
	struct IsImportant final : Importance<true> {};
	struct NotImportant final : Importance<false> {};

	template<bool V>
	RuleSet operator<<(const RuleSet& l, const Importance<V>& importance)
	{
		auto result{ l };
		result.isImportant = importance.value;
		return result;
	}

	template<bool V>
	RuleSet operator<<(const RuleVariant& l, const Importance<V>& importance)
	{
		RuleSet result;
		result.rules = { l };
		result.isImportant = importance.value;
		return result;
	}

	RuleSet operator<<(const RuleSet& l, const Translator& translator);
	RuleSet operator<<(const RuleVariant& l, const Translator& translator);
}
