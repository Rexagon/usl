#include "Parser.hpp"

#include <cassert>
#include <unordered_map>

app::Parser::Parser()
{
	m_grammar["sum"] =
		NonTerm{ "sum" } >> Term{ TokenType::OperatorPlus } >> NonTerm{ "product" } |
		NonTerm{ "sum" } >> Term{ TokenType::OperatorMinus } >> NonTerm{ "product" } |
		NonTerm{ "product" };

	m_grammar["product"] =
		NonTerm{ "product" } >> Term{ TokenType::OperatorMul } >> NonTerm{ "factor" } |
		NonTerm{ "product" } >> Term{ TokenType::OperatorDiv } >> NonTerm{ "factor" } |
		NonTerm{ "factor" };

	m_grammar["factor"] =
		Term{ TokenType::ParenthesisOpen } >> NonTerm{ "sum" } >> Term{ TokenType::ParenthesisClose } |
		NonTerm{ "number" };

	m_grammar["number"] =
		Term{ TokenType::Number };

	for (auto&[name, ruleCases] : m_grammar) {
		ruleCases.setName(name);
	}
}

void app::Parser::parse(const std::vector<Token>& tokens)
{
	if (tokens.empty()) {
		return;
	}

	// Fill parser states
	StateGroups stateGroups;
	stateGroups.reserve(tokens.size());

	stateGroups.emplace_back(m_grammar[m_startRule].generateStates(0));

	for (size_t i = 0; i < stateGroups.size(); ++i) {
		printf("==%zu==\n", i);

		for (size_t j = 0; j < stateGroups[i].size(); ++j) {
			const auto& item = stateGroups[i][j];

			switch (item.getNextType()) {
			case EarleyState::Type::Term:
				if (i < tokens.size()) {
					scan(stateGroups, i, j, tokens[i]);
				}
				break;

			case EarleyState::Type::NonTerm:
				predict(stateGroups, i, j, m_grammar);
				break;

			case EarleyState::Type::Null:
				complete(stateGroups, i, j);
				break;

			default: 
				break;
			}

			item.print();
		}
	}

	for (size_t i = 0; i < stateGroups.size(); ++i) {
		printf("==%zu==\n", i);

		for (const auto& item : stateGroups[i]) {
			item.print();
		}

		printf("\n");
	}

	// Validate result
	if (stateGroups.size() != tokens.size() + 1) {
		throw std::runtime_error("Unexpected end of stream");
	}

	const EarleyState* finalState = nullptr;
	for (const auto& item : stateGroups.back()) {
		if (item.getNextType() == EarleyState::Type::Null &&
			item.getOrigin() == 0 &&
			item.getName() == m_startRule) 
		{
			finalState = &item;
		}
	}

	if (finalState != nullptr) {
		printf("Input is valid\n");
		finalState->print();
	}
}

void app::Parser::scan(StateGroups& s, size_t i, size_t j, const Token& token)
{
	printf("scan %d\t ", token.first);

	const auto& item = s[i][j];

	const auto* nextSymbol = item.getNextTerm();

	if (nextSymbol == nullptr || nextSymbol->type != token.first) {
		return;
	}

	if (i + 1 >= s.size()) {
		s.emplace_back();
	}

	tryEmplace(s[i + 1], EarleyState(item, 1));
}

void app::Parser::predict(StateGroups& s, size_t i, size_t j, const Grammar& g)
{
	printf("predict\t ");

	const auto& item = s[i][j];

	const auto* nextSymbol = item.getNextNonTerm();

	const auto it = g.find(nextSymbol->name);
	assert(it != m_grammar.end());

	const auto states = it->second.generateStates(i);
	for (const auto& state : states) {
		tryEmplace(s[i], state);
	}
}

void app::Parser::complete(StateGroups& s, const size_t i, const size_t j)
{
	printf("complete ");

	const auto& item = s[i][j];

	for (const auto& state : s[item.getOrigin()]) {
		const auto* nextSymbol = state.getNextNonTerm();

		if (nextSymbol && nextSymbol->name == item.getName()) {
			tryEmplace(s[i], EarleyState(state, 1));
		}
	}
}

void app::Parser::tryEmplace(EarleySet& earleySet, const EarleyState& state)
{
	printf("try ");

	for (const auto& item : earleySet) {
		if (item == state) {
			return;
		}
	}

	printf("emplace ");

	earleySet.emplace_back(state);
}
