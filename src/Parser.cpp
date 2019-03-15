#include "Parser.hpp"

void app::Parser::parse(const std::vector<Token>& tokens)
{
	auto state = ParserState{ tokens.begin(), tokens };

	auto primaryExpression =
		Term(TokenType::Identifier) >> Term(TokenType::Number);

	primaryExpression = +primaryExpression;

	printf("Result: %d\n", primaryExpression(state));
}
