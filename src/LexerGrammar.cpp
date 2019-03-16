#include "LexerGrammar.hpp"

const app::RegexArray& app::buildRegexes()
{
	static RegexArray regexes{
		(TokenType::KeywordLet,			std::regex{"^let"}),
		(TokenType::KeywordIf,			std::regex{"^if"}),
		(TokenType::KeywordElse,		std::regex{"^else"}),
		(TokenType::KeywordWhile,		std::regex{"^while"}),
		(TokenType::KeywordFor,			std::regex{"^for"}),
		(TokenType::KeywordBreak,		std::regex{"^break"}),
		(TokenType::KeywordContinue,	std::regex{"^continue"}),
		(TokenType::KeywordFunction,	std::regex{"^function"}),
		(TokenType::KeywordReturn,		std::regex{"^return"}),

		(TokenType::Identifier,			std::regex{"^[a-zA-Z_]+"}),
		(TokenType::String,				std::regex{"^\"(?:\\\\.|[^\"])*\"?"}),
		(TokenType::Number,				std::regex{"^[0-9]+\\.?[0-9]*"}),

		(TokenType::OperatorAssignment, std::regex{"^="}),
		(TokenType::OperatorOr,			std::regex{"^\\|\\|"}),
		(TokenType::OperatorAnd,		std::regex{"^&&"}),
		(TokenType::OperatorEq,			std::regex{"^=="}),
		(TokenType::OperatorNeq,		std::regex{"^!="}),
		(TokenType::OperatorLt,			std::regex{"^<"}),
		(TokenType::OperatorLeq,		std::regex{"^<="}),
		(TokenType::OperatorGt,			std::regex{"^>"}),
		(TokenType::OperatorGeq,		std::regex{"^<="}),
		(TokenType::OperatorPlus,		std::regex{"^\\+"}),
		(TokenType::OperatorMinus,		std::regex{"^-"}),
		(TokenType::OperatorMul,		std::regex{"^\\*"}),
		(TokenType::OperatorDiv,		std::regex{"^/"}),
		(TokenType::OperatorIncrement,	std::regex{"^\\+\\+"}),
		(TokenType::OperatorDecrement,	std::regex{"^--"}),
		(TokenType::OperatorNegate,		std::regex{"^!"}),

		(TokenType::StructureReference, std::regex{"^\\."}),

		(TokenType::ParenthesisOpen,	std::regex{"^\\("}),
		(TokenType::ParenthesisClose,	std::regex{"^\\)"}),
		(TokenType::BraceOpen,			std::regex{"^\\{"}),
		(TokenType::BraceClose,			std::regex{"^\\}"}),
		(TokenType::BracketOpen,		std::regex{"^\\["}),
		(TokenType::BracketClose,		std::regex{"^\\]"}),

		(TokenType::Comma,				std::regex{"^,"}),
		(TokenType::Semicolon,			std::regex{"^;"}),

		(TokenType::CommentSingleLine,	std::regex{"^//[^\\n]*(?:\\n)?"}),
		(TokenType::CommentMultiLine,	std::regex{"^/\\*(?:[^\\*]/|\\*[^/]|[^\\*/])*.?.?"})
	};

	return regexes;
}
