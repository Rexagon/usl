#include "Parser.hpp"

void app::Parser::parse(const std::vector<Token>& tokens)
{
	auto state = ParserState{ tokens.begin(), tokens };

	const auto primaryExpression = std::make_shared<Term>(
		Term(TokenType::Identifier) || 
		Term(TokenType::Number) ||
		Term(TokenType::String));

	*primaryExpression = *primaryExpression ||
		Term(TokenType::ParenthesisOpen) && *primaryExpression && Term(TokenType::ParenthesisClose);

	printf("Result: %d\n", (*primaryExpression)(state));
}
