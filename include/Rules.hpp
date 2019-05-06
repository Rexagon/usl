#pragma once

#include <variant>
#include <functional>

#include "LexerGrammar.hpp"
#include "CommandBuffer.hpp"

namespace app
{
	struct Term;
	struct NonTerm;
	struct RuleSet;

	class EarleyItem;

	using RuleVariant = std::variant<Term, NonTerm>;
    using CompletedItem = std::pair<const EarleyItem*, size_t>; // Item and its end

    struct SyntaxNode
    {
        std::variant<CompletedItem, const Token*> value = nullptr;

        std::list<std::unique_ptr<SyntaxNode>> children;

        void translate(CommandBuffer& cb);
    };

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
	    using Translator = std::function<void(CommandBuffer&, SyntaxNode& node)>;
        static void defaultTranslator(CommandBuffer& cb, SyntaxNode& node);

		RuleSet() = default;

		bool operator==(const RuleSet& other) const;

		std::vector<RuleVariant> rules;
		Translator translator = &RuleSet::defaultTranslator;
		bool isImportant = true;

	};
}
