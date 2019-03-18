#include "ParserGrammar.hpp"

#include <cassert>

using namespace app::lexer_grammar;
using namespace app::parser_grammar;

app::ParserGrammar::ParserGrammar()
{
	m_rules[STARTING_RULE] =
		RuleSet{} << NotImportant{} |
		NonTerm{ GeneralStatement } >> NonTerm{ STARTING_RULE } << NotImportant{};

	m_rules[GeneralStatement] =
		NonTerm{ Statement } << NotImportant{} |
		NonTerm{ FunctionDeclaration } << NotImportant{};

	m_rules[Statement] =
		NonTerm{ WhileLoop } << NotImportant{} |
		NonTerm{ Branch } << NotImportant{} |
		NonTerm{ VariableDeclaration } >> Term{ Semicolon } |
		NonTerm{ Expression } >> Term{ Semicolon } << createSimpleTranslator(opcode::POP) |
		Term{ KeywordReturn } >> NonTerm{ Expression } >> Term{ Semicolon } |
		Term{ KeywordBreak } >> Term{ Semicolon } |
		Term{ KeywordContinue } >> Term{ Semicolon };

	m_rules[FunctionDeclaration] =
		Term{ KeywordFunction } >> Term{ Identifier } >> Term{ ParenthesisOpen } >>
			NonTerm{ FunctionArguments } >> Term{ ParenthesisClose } >> NonTerm{ Block };

	m_rules[FunctionArguments] =
		RuleSet{} |
		NonTerm{ FunctionArgument } << NotImportant{};

	m_rules[FunctionArgument] =
		Term{ Identifier } >> NonTerm{ CommaFunctionArgument };

	m_rules[CommaFunctionArgument] =
		RuleSet{} |
		Term{ Comma } >> Term{ Identifier } >> NonTerm{ CommaFunctionArgument };

	m_rules[Block] =
		NonTerm{ Statement } |
		Term{ BraceOpen } >> NonTerm{ BlockStatement } >> Term{ BraceClose };

	m_rules[BlockStatement] =
		RuleSet{} |
		NonTerm{ Statement } >> NonTerm{ BlockStatement } << NotImportant{};

	m_rules[Condition] =
		Term{ ParenthesisOpen } >> NonTerm{ Expression } >> Term{ ParenthesisClose };

	m_rules[WhileLoop] =
		Term{ KeywordWhile } >> NonTerm{ Condition } >> NonTerm{ Block };

	m_rules[Branch] =
		Term{ KeywordIf } >> NonTerm{ Condition} >> NonTerm{ Block } |
		Term{ KeywordIf } >> NonTerm{ Condition } >> NonTerm{ Block } >> NonTerm{ ElseBranch };

	m_rules[ElseBranch] =
		Term{ KeywordElse } >> NonTerm{ Block };

	m_rules[VariableDeclaration] =
		Term{ KeywordLet } >> Term{ Identifier } << createSimpleTranslator(opcode::DECL) |
		Term{ KeywordLet } >> Term{ Identifier } >> Term{ OperatorAssignment } >> NonTerm{ Expression };

	m_rules[Expression] =
		NonTerm{ LogicalOrExpression } << NotImportant{} |
		NonTerm{ UnaryExpression } >> Term{ OperatorAssignment } >> NonTerm{ Expression } << createSimpleTranslator(opcode::ASSIGN);

	m_rules[LogicalOrExpression] =
		NonTerm{ LogicalAndExpression } << NotImportant{} |
		NonTerm{ LogicalOrExpression } >> Term{ OperatorOr } >> NonTerm{ LogicalAndExpression } << createSimpleTranslator(opcode::OR);

	m_rules[LogicalAndExpression] =
		NonTerm{ EqualityExpression } << NotImportant{} |
		NonTerm{ LogicalAndExpression } >> Term{ OperatorAnd } >> NonTerm{ EqualityExpression } << createSimpleTranslator(opcode::AND);

	m_rules[EqualityExpression] =
		NonTerm{ RelationalExpression } << NotImportant{} |
		NonTerm{ EqualityExpression } >> Term{ OperatorEq } >> NonTerm{ RelationalExpression } << createSimpleTranslator(opcode::EQ) |
		NonTerm{ EqualityExpression } >> Term{ OperatorNeq } >> NonTerm{ RelationalExpression } << createSimpleTranslator(opcode::NEQ);

	m_rules[RelationalExpression] =
		NonTerm{ AdditiveExpression } << NotImportant{} |
		NonTerm{ RelationalExpression } >> Term{ OperatorLt } >> NonTerm{ AdditiveExpression } << createSimpleTranslator(opcode::LT) |
		NonTerm{ RelationalExpression } >> Term{ OperatorLeq } >> NonTerm{ AdditiveExpression } << createSimpleTranslator(opcode::LE) |
		NonTerm{ RelationalExpression } >> Term{ OperatorGt } >> NonTerm{ AdditiveExpression } << createSimpleTranslator(opcode::GT) |
		NonTerm{ RelationalExpression } >> Term{ OperatorGeq } >> NonTerm{ AdditiveExpression } << createSimpleTranslator(opcode::GE);

	m_rules[AdditiveExpression] =
		NonTerm{ MultiplicativeExpression } << NotImportant{} |
		NonTerm{ AdditiveExpression } >> Term{ OperatorPlus } >> NonTerm{ MultiplicativeExpression } << createSimpleTranslator(opcode::ADD) |
		NonTerm{ AdditiveExpression } >> Term{ OperatorMinus } >> NonTerm{ MultiplicativeExpression } << createSimpleTranslator(opcode::SUB);

	m_rules[MultiplicativeExpression] =
		NonTerm{ UnaryExpression } << NotImportant{} |
		NonTerm{ MultiplicativeExpression } >> Term{ OperatorMul } >> NonTerm{ UnaryExpression } << createSimpleTranslator(opcode::MUL) |
		NonTerm{ MultiplicativeExpression } >> Term{ OperatorDiv } >> NonTerm{ UnaryExpression } << createSimpleTranslator(opcode::DIV);

	m_rules[UnaryExpression] =
		NonTerm{ PostfixExpression } << NotImportant{} |
		Term{ OperatorIncrement } >> NonTerm{ UnaryExpression } << createSimpleTranslator(opcode::INC) |
		Term{ OperatorDecrement } >> NonTerm{ UnaryExpression } << createSimpleTranslator(opcode::DEC) |
		Term{ OperatorPlus } >> NonTerm{ UnaryExpression } << NotImportant{} |
		Term{ OperatorMinus } >> NonTerm{ UnaryExpression } |
		Term{ OperatorNegate } >> NonTerm{ UnaryExpression };

	m_rules[PostfixExpression] =
		NonTerm{ PrimaryExpression } << NotImportant{} |
		NonTerm{ PostfixExpression } >> Term{ OperatorIncrement } |
		NonTerm{ PostfixExpression } >> Term{ OperatorDecrement } |
		NonTerm{ PostfixExpression } >> Term{ StructureReference } >> Term{ Identifier } |
		NonTerm{ PostfixExpression } >> Term{ ParenthesisOpen } >> NonTerm{ CallArguments } >> Term{ ParenthesisClose };

	m_rules[PrimaryExpression] =
		Term{ Identifier } |
		Term{ Number } |
		Term{ String } |
		Term{ ParenthesisOpen } >> NonTerm{ Expression } >> Term{ ParenthesisClose };

	m_rules[CallArguments] =
		RuleSet{} |
		NonTerm{ CallArgument } << NotImportant{};

	m_rules[CallArgument] =
		NonTerm{ Expression } >> NonTerm{ CommaCallArgument };

	m_rules[CommaCallArgument] =
		RuleSet{} |
		Term{ Comma } >> NonTerm{ Expression } >> NonTerm{ CommaCallArgument };

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

std::vector<app::EarleyItem> app::ParserGrammar::generateEarleyItems(const size_t name, const size_t origin) const
{
	assert(name < RuleName::Count);
	return (*this)[name].generateEarleyItems(origin);
}

bool app::ParserGrammar::isNullable(const size_t name)
{
	return m_nullableRules.find(name) != m_nullableRules.end();
}

const app::Rules& app::ParserGrammar::operator[](const size_t name) const
{
	assert(name < RuleName::Count);
	return m_rules[name];
}

void app::ParserGrammar::finalize()
{
	for (size_t i = 0; i < m_rules.size(); ++i) {
		m_rules[i].setName(i);
	}

	while (true) {
		const auto oldSize = m_nullableRules.size();

		for (size_t i = 0; i < m_rules.size(); ++i) {
			for (const auto& set : m_rules[i].getRuleSets()) {
				auto nullable = true;
				for (const auto& item : set.rules) {
					const auto* nonterm = std::get_if<NonTerm>(&item);
					if (nonterm == nullptr || !isNullable(nonterm->name)) {
						nullable = false;
					}
				}

				if (nullable) {
					m_nullableRules.emplace(i);
				}
			}
		}

		if (m_nullableRules.size() == oldSize) {
			break;
		}
	}
}

const char* app::parser_grammar::getString(const size_t name)
{
	switch (name) {
	case Program: 
		return "program";
	case GeneralStatement: 
		return "general_statement";
	case Statement: 
		return "statement";
	case FunctionDeclaration: 
		return "function_declaration";
	case FunctionArguments: 
		return "function_arguments";
	case FunctionArgument: 
		return "function_argument";
	case CommaFunctionArgument: 
		return "comma_function_argument";
	case Block: 
		return "block";
	case BlockStatement: 
		return "statement";
	case Condition: 
		return "condition";
	case WhileLoop: 
		return "while_loop";
	case Branch: 
		return "branch";
	case ElseBranch: 
		return "else_branch";
	case VariableDeclaration: 
		return "variable_declaration";
	case Expression: 
		return "expression";
	case LogicalOrExpression: 
		return "logical_or_expression";
	case LogicalAndExpression: 
		return "logical_and_expression";
	case EqualityExpression: 
		return "equality_expression";
	case RelationalExpression: 
		return "relational_expression";
	case AdditiveExpression: 
		return "additive_expression";
	case MultiplicativeExpression: 
		return "multiplicative_expression";
	case UnaryExpression: 
		return "unary_expression";
	case PostfixExpression: 
		return "postfix_expression";
	case PrimaryExpression: 
		return "primary_expression";
	case CallArguments: 
		return "call_arguments";
	case CallArgument: 
		return "call_argument";
	case CommaCallArgument: 
		return "comma_call_argument";

	case Count:
	default: 
		return "invalid";
	}
}