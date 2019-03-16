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
		Term{ TokenType::ParenthesisOpen } >> NonTerm{ "number" } >> Term{ TokenType::ParenthesisClose } |
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

	// Start Earley parsing
	StateGroups stateGroups;
	stateGroups.reserve(tokens.size());

	stateGroups.emplace_back(m_grammar["sum"].generateStates(0));

	for (size_t i = 0; i < tokens.size(); ++i) {
		printf("==%zu==\n", i);

		for (size_t j = 0; j < stateGroups[i].size(); ++j) {
			const auto& item = stateGroups[i][j];

			printf("%zu: %s ->", j, std::string(item.getName()).c_str());

			switch (item.getNextType()) {
			case EarleyState::Type::Term:
				printf("term %d\n", item.getNextTerm()->type);
				scan(stateGroups, i, j, tokens[i]);
				break;

			case EarleyState::Type::NonTerm:
				printf("nonterm %s\n", item.getNextNonTerm()->name.c_str());
				predict(stateGroups, i, j, m_grammar);
				break;

			case EarleyState::Type::Null:
				printf("null\n");
				complete(stateGroups, i, j);
				break;

			default: 
				break;
			}
		}
	}
}

void app::Parser::scan(StateGroups& s, size_t i, size_t j, const Token& token)
{
	const auto& item = s[i][j];

	const auto* nextSymbol = item.getNextTerm();
	if (nextSymbol == nullptr || nextSymbol->type != token.first) {
		return;
	}

	if (i >= s.size()) {
		s.emplace_back();
	}

	tryEmplace(s[i + 1], EarleyState(item, 1));
}

void app::Parser::predict(StateGroups& s, size_t i, size_t j, const Grammar& g)
{
	const auto& item = s[i][j];

	const auto* nextSymbol = item.getNextNonTerm();

	const auto it = g.find(nextSymbol->name);
	assert(it != m_grammar.end());

	const auto states = it->second.generateStates(i, 1);
	for (const auto& state : states) {
		tryEmplace(s[i], state);
	}
}

void app::Parser::complete(StateGroups& s, const size_t i, const size_t j)
{
	const auto& item = s[i][j];

	for (const auto& state : s[i]) {
		const auto* nextSymbol = state.getNextNonTerm();
		if (nextSymbol && nextSymbol->name == item.getName()) {
			tryEmplace(s[i], EarleyState(item, 1));
		}
	}
}

void app::Parser::tryEmplace(EarleySet& earleySet, const EarleyState& state)
{
	for (const auto& item : earleySet) {
		if (item == state) {
			return;
		}
	}

	earleySet.emplace_back(state);
}
