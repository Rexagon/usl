#pragma once

#include <array>
#include <regex>
#include <bitset>

namespace app
{
	enum class TokenType
	{
		KeywordLet,
		KeywordIf,
		KeywordElse,
		KeywordWhile,
		KeywordFor,
		KeywordBreak,
		KeywordContinue,
		KeywordFunction,
		KeywordReturn,

		Identifier,
		String,
		Number,

		OperatorAssignment,
		OperatorOr,
		OperatorAnd,
		OperatorEq,
		OperatorNeq,
		OperatorLt,
		OperatorLeq,
		OperatorGt,
		OperatorGeq,
		OperatorPlus,
		OperatorMinus,
		OperatorMul,
		OperatorDiv,
		OperatorIncrement,
		OperatorDecrement,
		OperatorNegate,

		StructureReference,

		ParenthesisOpen,
		ParenthesisClose,
		BraceOpen,
		BraceClose,
		BracketOpen,
		BracketClose,

		Comma,
		Semicolon,

		CommentSingleLine,
		CommentMultiLine,

		Invalid, // also used as count
	};

	constexpr auto TOKEN_COUNT = static_cast<size_t>(TokenType::Invalid);

	using RegexArray = std::array<std::regex, TOKEN_COUNT>;
	using RegexMask = std::bitset<TOKEN_COUNT>;

	using Token = std::pair<TokenType, std::string_view>;

	const RegexArray& buildRegexes();
}
