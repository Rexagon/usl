#include "ParserGrammar.hpp"

const app::Grammar& app::buildGrammar()
{
	static auto initialized = false;
	static Grammar grammar;

	if (initialized) {
		return grammar;
	}

	grammar["sum"] =
		NonTerm{ "sum" } >> Term{ TokenType::OperatorPlus } >> NonTerm{ "product" } |
		NonTerm{ "sum" } >> Term{ TokenType::OperatorMinus } >> NonTerm{ "product" } |
		NonTerm{ "product" };

	grammar["product"] =
		NonTerm{ "product" } >> Term{ TokenType::OperatorMul } >> NonTerm{ "factor" } |
		NonTerm{ "product" } >> Term{ TokenType::OperatorDiv } >> NonTerm{ "factor" } |
		NonTerm{ "factor" };

	grammar["factor"] =
		Term{ TokenType::ParenthesisOpen } >> NonTerm{ "sum" } >> Term{ TokenType::ParenthesisClose } |
		NonTerm{ "number" };

	grammar["number"] =
		Term{ TokenType::Number };

	for (auto& [name, rules] : grammar) {
		rules.setName(name);
	}

	initialized = true;
	return grammar;
}
