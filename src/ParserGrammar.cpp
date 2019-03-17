#include "ParserGrammar.hpp"

#include <cassert>

const std::string app::ParserGrammar::STARTING_RULE = "program";

app::ParserGrammar::ParserGrammar()
{
	m_rules[STARTING_RULE] =
		NonTerm{ "general_statement" } |
		NonTerm{ "general_statement" } >> NonTerm{ STARTING_RULE };

	//TODO: add function declaration
	m_rules["general_statement"] =
		NonTerm{ "statement" };

	m_rules["statement"] =
		NonTerm{ "while_loop" } |
		NonTerm{ "branch" } |
		NonTerm{ "variable_declaration" } >> Term{ TokenType::Semicolon } |
		NonTerm{ "expression" } >> Term{ TokenType::Semicolon };

	m_rules["block"] =
		NonTerm{"statement"} |
		Term{ TokenType::BraceOpen } >> NonTerm{ "block_statement" } >> Term{ TokenType::BraceClose };

	m_rules["block_statement"] =
		NonTerm{ "statement" } |
		NonTerm{ "statement" } >> NonTerm{"block_statement"};

	m_rules["condition"] =
		Term{ TokenType::ParenthesisOpen } >> NonTerm{ "expression" } >> Term{ TokenType::ParenthesisClose };

	m_rules["while_loop"] =
		Term{TokenType::KeywordWhile} >> NonTerm{"condition"} >> NonTerm{ "block" };

	m_rules["branch"] =
		Term{ TokenType::KeywordIf } >> NonTerm{ "condition" } >> NonTerm{ "block" };

	//TODO: add optional assign operation
	m_rules["variable_declaration"] =
		Term{ TokenType::KeywordLet } >> Term{ TokenType::Identifier };

	m_rules["expression"] =
		NonTerm{ "logical_or_expression" } |
		NonTerm{ "unary_expression" } >> Term{ TokenType::OperatorAssignment } >> NonTerm{ "expression" };

	m_rules["logical_or_expression"] =
		NonTerm{ "logical_and_expression" } |
		NonTerm{ "logical_or_expression" } >> Term{ TokenType::OperatorOr } >> NonTerm{ "logical_and_expression" };

	m_rules["logical_and_expression"] =
		NonTerm{ "equality_expression" } |
		NonTerm{ "logical_and_expression" } >> Term{ TokenType::OperatorAnd } >> NonTerm{ "equality_expression" };

	m_rules["equality_expression"] =
		NonTerm{ "relational_expression" } |
		NonTerm{ "equality_expression" } >> Term{ TokenType::OperatorEq } >> NonTerm{ "relational_expression" } |
		NonTerm{ "equality_expression" } >> Term{ TokenType::OperatorNeq } >> NonTerm{ "relational_expression" };

	m_rules["relational_expression"] =
		NonTerm{ "additive_expression" } |
		NonTerm{ "relational_expression" } >> Term{ TokenType::OperatorLt } >> NonTerm{ "additive_expression" } |
		NonTerm{ "relational_expression" } >> Term{ TokenType::OperatorLeq } >> NonTerm{ "additive_expression" } |
		NonTerm{ "relational_expression" } >> Term{ TokenType::OperatorGt } >> NonTerm{ "additive_expression" } |
		NonTerm{ "relational_expression" } >> Term{ TokenType::OperatorGeq } >> NonTerm{ "additive_expression" };

	m_rules["additive_expression"] =
		NonTerm{ "multiplicative_expression" } |
		NonTerm{ "additive_expression" } >> Term{ TokenType::OperatorPlus } >> NonTerm{ "multiplicative_expression" } |
		NonTerm{ "additive_expression" } >> Term{ TokenType::OperatorMinus } >> NonTerm{ "multiplicative_expression" };

	m_rules["multiplicative_expression"] =
		NonTerm{ "unary_expression" } |
		NonTerm{ "multiplicative_expression" } >> Term{ TokenType::OperatorMul } >> NonTerm{ "unary_expression" } |
		NonTerm{ "multiplicative_expression" } >> Term{ TokenType::OperatorDiv } >> NonTerm{ "unary_expression" };

	m_rules["unary_expression"] =
		NonTerm{ "postfix_expression" } |
		Term{ TokenType::OperatorIncrement } >> NonTerm{ "unary_expression" } |
		Term{ TokenType::OperatorDecrement } >> NonTerm{ "unary_expression" } |
		Term{ TokenType::OperatorPlus } >> NonTerm{ "unary_expression" } |
		Term{ TokenType::OperatorMinus } >> NonTerm{ "unary_expression" } |
		Term{ TokenType::OperatorNegate } >> NonTerm{ "unary_expression" };

	m_rules["postfix_expression"] =
		NonTerm{ "primary_expression" } |
		NonTerm{ "postfix_expression" } >> Term{ TokenType::OperatorIncrement } |
		NonTerm{ "postfix_expression" } >> Term{ TokenType::OperatorDecrement } |
		NonTerm{ "postfix_expression" } >> Term{ TokenType::StructureReference } >> Term{ TokenType::Identifier };

	m_rules["primary_expression"] =
		NonTerm{ "primary_expression" } |
		Term{ TokenType::Identifier } |
		Term{ TokenType::Number } |
		Term{ TokenType::String } |
		Term{ TokenType::ParenthesisOpen } >> NonTerm{ "expression" } >> Term{ TokenType::ParenthesisClose };

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

const app::Rules& app::ParserGrammar::operator[](const std::string& name) const
{
	const auto it = m_rules.find(name);
	assert(it != name, "unable to find rule name");
	return it->second;
}

void app::ParserGrammar::finalize()
{
	for (auto& [name, rules] : m_rules) {
		rules.setName(name);
	}
}
