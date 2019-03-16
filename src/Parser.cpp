#include "Parser.hpp"

#include <cassert>
#include <unordered_map>

app::Parser::Parser() :
	m_grammar(buildGrammar())
{
}

void app::Parser::parse(const std::vector<Token>& tokens)
{
	if (tokens.empty()) {
		return;
	}

	// Fill parser states
	StateSets stateSets;
	stateSets.reserve(tokens.size());

	const auto it = m_grammar.find(STARTING_RULE);
	assert(it != m_grammar.end(), "Unable to find starting rule");

	stateSets.emplace_back(it->second.generateEarleyItems(0));

	for (size_t i = 0; i < stateSets.size(); ++i) {
		for (size_t j = 0; j < stateSets[i].size(); ++j) {
			const auto& item = stateSets[i][j];

			switch (item.getNextType()) {
			case EarleyItem::NextType::Term:
				if (i < tokens.size()) {
					scan(stateSets, i, j, tokens[i]);
				}
				break;

			case EarleyItem::NextType::NonTerm:
				predict(stateSets, i, j, m_grammar);
				break;

			case EarleyItem::NextType::Null:
				complete(stateSets, i, j);
				break;

			default: 
				break;
			}
		}
	}

	for (size_t i = 0; i < stateSets.size(); ++i) {
		printf("==%zu==\n", i);

		for (const auto& item : stateSets[i]) {
			item.print();
		}

		printf("\n");
	}

	// Validate result
	if (stateSets.size() != tokens.size() + 1) {
		throw std::runtime_error("Unexpected end of stream");
	}

	const EarleyItem* finalItem = nullptr;
	for (const auto& item : stateSets.back()) {
		if (item.getNextType() == EarleyItem::NextType::Null &&
			item.getOrigin() == 0 &&
			item.getName() == STARTING_RULE)
		{
			finalItem = &item;
		}
	}

	if (finalItem != nullptr) {
		printf("Input is valid\n");
		finalItem->print();
	}
}

void app::Parser::scan(StateSets& s, size_t i, size_t j, const Token& token)
{
	const auto& currentItem = s[i][j];

	const auto* nextSymbol = currentItem.getNextTerm();

	if (nextSymbol == nullptr || nextSymbol->type != token.first) {
		return;
	}

	if (i + 1 >= s.size()) {
		s.emplace_back();
	}

	tryEmplace(s[i + 1], currentItem.createAdvanced(1));
}

void app::Parser::predict(StateSets& s, size_t i, size_t j, const Grammar& g)
{
	const auto& currentItem = s[i][j];

	const auto* nextSymbol = currentItem.getNextNonTerm();

	const auto it = g.find(nextSymbol->name);
	assert(it != m_grammar.end());

	const auto items = it->second.generateEarleyItems(i);
	for (const auto& item : items) {
		tryEmplace(s[i], item);
	}
}

void app::Parser::complete(StateSets& s, const size_t i, const size_t j)
{
	const auto& currentItem = s[i][j];

	for (const auto& item : s[currentItem.getOrigin()]) {
		const auto* nextSymbol = item.getNextNonTerm();

		if (nextSymbol && nextSymbol->name == currentItem.getName()) {
			tryEmplace(s[i], item.createAdvanced(1));
		}
	}
}

void app::Parser::tryEmplace(StateSet& stateSet, const EarleyItem& item)
{
	for (const auto& existingItem : stateSet) {
		if (existingItem == item) {
			return;
		}
	}

	stateSet.emplace_back(item);
}
