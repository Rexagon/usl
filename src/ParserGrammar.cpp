#include "ParserGrammar.hpp"

#include <cassert>

const std::string app::ParserGrammar::STARTING_RULE = "program";

app::ParserGrammar::ParserGrammar()
{
	m_rules[STARTING_RULE] =
		RuleSet{} << false |
		NonTerm{ "general_statement" } >> NonTerm{ STARTING_RULE } << false;

	m_rules["general_statement"] =
		NonTerm{ "statement" } << false |
		NonTerm{ "function_declaration" } << false;

	m_rules["statement"] =
		NonTerm{ "while_loop" } |
		NonTerm{ "branch" } |
		NonTerm{ "variable_declaration" } >> Term{ TokenType::Semicolon } |
		NonTerm{ "expression" } >> Term{ TokenType::Semicolon } |
		Term{ TokenType::KeywordReturn } >> NonTerm{ "expression" } >> Term{ TokenType::Semicolon } |
		Term{ TokenType::KeywordBreak } |
		Term{ TokenType::KeywordContinue };

	m_rules["function_declaration"] =
		Term{ TokenType::KeywordFunction } >> Term{TokenType::Identifier} >> Term{ TokenType::ParenthesisOpen } >> 
			NonTerm{ "function_arguments" } >> Term{ TokenType::ParenthesisClose } >> NonTerm{ "block" };

	m_rules["function_arguments"] =
		RuleSet{} |
		Term{ TokenType::Identifier } >> NonTerm{ "comma_function_argument" };

	m_rules["comma_function_argument"] =
		RuleSet{} |
		Term{ TokenType::Comma } >> Term{ TokenType::Identifier } >> NonTerm{ "comma_function_argument" };

	m_rules["block"] =
		NonTerm{"statement"} |
		Term{ TokenType::BraceOpen } >> NonTerm{ "block_statement" } >> Term{ TokenType::BraceClose };

	m_rules["block_statement"] =
		RuleSet{} |
		NonTerm{ "statement" } >> NonTerm{"block_statement"} << false;

	m_rules["condition"] =
		Term{ TokenType::ParenthesisOpen } >> NonTerm{ "expression" } >> Term{ TokenType::ParenthesisClose };

	m_rules["while_loop"] =
		Term{TokenType::KeywordWhile} >> NonTerm{"condition"} >> NonTerm{ "block" };

	m_rules["branch"] =
		Term{ TokenType::KeywordIf } >> NonTerm{ "condition" } >> NonTerm{ "block" } |
		Term{ TokenType::KeywordIf } >> NonTerm{ "condition" } >> NonTerm{ "block" } >> NonTerm{ "else_branch" };

	m_rules["else_branch"] =
		Term{ TokenType::KeywordElse } >> NonTerm{ "block" };

	m_rules["variable_declaration"] =
		Term{ TokenType::KeywordLet } >> Term{ TokenType::Identifier } |
		Term{ TokenType::KeywordLet } >> Term{ TokenType::Identifier } >> Term{ TokenType::OperatorAssignment } >> NonTerm{ "expression" };

	m_rules["expression"] =
		NonTerm{ "logical_or_expression" } << false |
		NonTerm{ "unary_expression" } >> Term{ TokenType::OperatorAssignment } >> NonTerm{ "expression" };

	m_rules["logical_or_expression"] =
		NonTerm{ "logical_and_expression" } << false |
		NonTerm{ "logical_or_expression" } >> Term{ TokenType::OperatorOr } >> NonTerm{ "logical_and_expression" };

	m_rules["logical_and_expression"] =
		NonTerm{ "equality_expression" } << false |
		NonTerm{ "logical_and_expression" } >> Term{ TokenType::OperatorAnd } >> NonTerm{ "equality_expression" };

	m_rules["equality_expression"] =
		NonTerm{ "relational_expression" } << false |
		NonTerm{ "equality_expression" } >> Term{ TokenType::OperatorEq } >> NonTerm{ "relational_expression" } |
		NonTerm{ "equality_expression" } >> Term{ TokenType::OperatorNeq } >> NonTerm{ "relational_expression" };

	m_rules["relational_expression"] =
		NonTerm{ "additive_expression" } << false |
		NonTerm{ "relational_expression" } >> Term{ TokenType::OperatorLt } >> NonTerm{ "additive_expression" } |
		NonTerm{ "relational_expression" } >> Term{ TokenType::OperatorLeq } >> NonTerm{ "additive_expression" } |
		NonTerm{ "relational_expression" } >> Term{ TokenType::OperatorGt } >> NonTerm{ "additive_expression" } |
		NonTerm{ "relational_expression" } >> Term{ TokenType::OperatorGeq } >> NonTerm{ "additive_expression" };

	m_rules["additive_expression"] =
		NonTerm{ "multiplicative_expression" } << false |
		NonTerm{ "additive_expression" } >> Term{ TokenType::OperatorPlus } >> NonTerm{ "multiplicative_expression" } |
		NonTerm{ "additive_expression" } >> Term{ TokenType::OperatorMinus } >> NonTerm{ "multiplicative_expression" };

	m_rules["multiplicative_expression"] =
		NonTerm{ "unary_expression" } << false |
		NonTerm{ "multiplicative_expression" } >> Term{ TokenType::OperatorMul } >> NonTerm{ "unary_expression" } |
		NonTerm{ "multiplicative_expression" } >> Term{ TokenType::OperatorDiv } >> NonTerm{ "unary_expression" };

	m_rules["unary_expression"] =
		NonTerm{ "postfix_expression" } << false |
		Term{ TokenType::OperatorIncrement } >> NonTerm{ "unary_expression" } |
		Term{ TokenType::OperatorDecrement } >> NonTerm{ "unary_expression" } |
		Term{ TokenType::OperatorPlus } >> NonTerm{ "unary_expression" } |
		Term{ TokenType::OperatorMinus } >> NonTerm{ "unary_expression" } |
		Term{ TokenType::OperatorNegate } >> NonTerm{ "unary_expression" };

	m_rules["postfix_expression"] =
		NonTerm{ "primary_expression" } << false |
		NonTerm{ "postfix_expression" } >> Term{ TokenType::OperatorIncrement } |
		NonTerm{ "postfix_expression" } >> Term{ TokenType::OperatorDecrement } |
		NonTerm{ "postfix_expression" } >> Term{ TokenType::StructureReference } >> Term{ TokenType::Identifier } |
		NonTerm{ "postfix_expression" } >> Term{ TokenType::ParenthesisOpen } >> NonTerm{ "call_arguments" } >> Term{ TokenType::ParenthesisClose };

	m_rules["primary_expression"] =
		NonTerm{ "primary_expression" } << false |
		Term{ TokenType::Identifier } |
		Term{ TokenType::Number } |
		Term{ TokenType::String } |
		Term{ TokenType::ParenthesisOpen } >> NonTerm{ "expression" } >> Term{ TokenType::ParenthesisClose };

	m_rules["call_arguments"] =
		RuleSet{} |
		NonTerm{ "expression" } >> NonTerm{ "comma_call_argument" };

	m_rules["comma_call_argument"] =
		RuleSet{} |
		Term{ TokenType::Comma } >> NonTerm{ "expression" } >> NonTerm{ "comma_call_argument" };

	finalize();
}

const app::ParserGrammar& app::ParserGrammar::create()
{
	static ParserGrammar grammar;
	return grammar;
}

std::vector<app::EarleyItem> app::ParserGrammar::generateStartingEarleyItems() const
{
	return (*this)[STARTING_RULE].generateEarleyItems(0);
}

std::vector<app::EarleyItem> app::ParserGrammar::generateEarleyItems(const std::string& name, const size_t origin) const
{
	return (*this)[name].generateEarleyItems(origin);
}

bool app::ParserGrammar::isNullable(const std::string& name)
{
	return m_nullableRules.find(name) != m_nullableRules.end();
}

const app::Rules& app::ParserGrammar::operator[](const std::string& name) const
{
	const auto it = m_rules.find(name);
	assert(it != name);
	return it->second;
}

void app::ParserGrammar::finalize()
{
	for (auto& [name, rules] : m_rules) {
		rules.setName(name);
	}

	while (true) {
		const auto oldSize = m_nullableRules.size();

		for (const auto& [name, rules] : m_rules) {
			for (const auto& set : rules.getRuleSets()) {
				auto nullable = true;
				for (const auto& item : set.rules) {
					const auto* nonterm = std::get_if<NonTerm>(&item);
					if (nonterm == nullptr || !isNullable(nonterm->name)) {
						nullable = false;
					}
				}

				if (nullable) {
					m_nullableRules.emplace(name);
				}
			}
		}

		if (m_nullableRules.size() == oldSize) {
			break;
		}
	}
}
