#pragma once

#include <array>
#include <unordered_set>

#include "EarleyItem.hpp"

namespace app
{
	namespace parser_grammar {
		enum RuleName
		{
			Program,
			GeneralStatement,
			Statement,
			FunctionDeclaration,
			FunctionArguments,
			FunctionArgument,
			CommaFunctionArgument,
            FunctionArgumentIdentifier,
            FunctionBlock,
            Block,
			BlockStatement,
			Condition,
			WhileLoop,
			Branch,
			ElseBranch,
			VariableDeclaration,
			Expression,
			LogicalOrExpression,
			LogicalAndExpression,
			EqualityExpression,
			RelationalExpression,
			AdditiveExpression,
			MultiplicativeExpression,
			UnaryExpression,
			PostfixExpression,
			PrimaryExpression,
			CallArguments,
			CallArgument,
			CommaCallArgument,

			Count,
		};

		constexpr auto RULE_COUNT = Count;

		constexpr auto STARTING_RULE = Program;

		const char* getString(size_t name);
	}

	class ParserGrammar final
	{
	public:
		std::vector<EarleyItem> generateStartingEarleyItems() const;
		std::vector<EarleyItem> generateEarleyItems(size_t name, size_t origin) const;

		bool isNullable(size_t name);

		const Rules& operator[](size_t name) const;

		static const ParserGrammar& create();

	private:
		ParserGrammar();
		void finalize();

		std::array<Rules, parser_grammar::RULE_COUNT> m_rules;
		std::unordered_set<size_t> m_nullableRules;
	};
}
