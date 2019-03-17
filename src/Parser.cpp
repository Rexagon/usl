#include "Parser.hpp"

app::Parser::Parser() :
	m_grammar(ParserGrammar::create())
{
}

void app::Parser::parse(const std::vector<Token>& tokens)
{
	if (tokens.empty()) {
		return;
	}

	// Fill parser states
	m_stateSets.clear();
	m_stateSets.reserve(tokens.size());

	m_stateSets.emplace_back(m_grammar.generateStartingEarleyItems());

	for (size_t i = 0; i < m_stateSets.size(); ++i) {
		for (size_t j = 0; j < m_stateSets[i].size(); ++j) {
			const auto& item = m_stateSets[i][j];

			switch (item.getNextType()) {
			case EarleyItem::NextType::Term:
				if (i < tokens.size()) {
					scan(i, j, tokens[i]);
				}
				break;

			case EarleyItem::NextType::NonTerm:
				predict(i, j, m_grammar);
				break;

			case EarleyItem::NextType::Null:
				complete(i, j);
				break;

			default: 
				break;
			}
		}
	}

	for (size_t i = 0; i < m_stateSets.size(); ++i) {
		printf("==%zu==\n", i);

		for (const auto& item : m_stateSets[i]) {
			item.print();
		}

		printf("\n");
	}

	// Validate result
	if (m_stateSets.size() != tokens.size() + 1) {
		throw std::runtime_error("Unexpected end of stream");
	}

	const EarleyItem* finalItem = nullptr;
	for (const auto& item : m_stateSets.back()) {
		if (item.getNextType() == EarleyItem::NextType::Null &&
			item.getOrigin() == 0 &&
			item.getName() == ParserGrammar::STARTING_RULE)
		{
			finalItem = &item;
		}
	}

	if (finalItem != nullptr) {
		printf("Input is valid\n");
		finalItem->print();
	}
}

void app::Parser::scan(const size_t i, const size_t j, const Token& token)
{
	const auto& currentItem = m_stateSets[i][j];

	const auto* nextSymbol = currentItem.getNextTerm();

	if (nextSymbol == nullptr || nextSymbol->type != token.first) {
		return;
	}

	if (i + 1 >= m_stateSets.size()) {
		m_stateSets.emplace_back();
	}

	tryEmplace(m_stateSets[i + 1], currentItem.createAdvanced(1));
}

void app::Parser::predict(const size_t i, const size_t j, const ParserGrammar& g)
{
	const auto& currentItem = m_stateSets[i][j];

	const auto* nextSymbol = currentItem.getNextNonTerm();

	const auto items = m_grammar.generateEarleyItems(nextSymbol->name, i);
	for (const auto& item : items) {
		tryEmplace(m_stateSets[i], item);
	}
}

void app::Parser::complete(const size_t i, const size_t j)
{
	const auto& currentItem = m_stateSets[i][j];

	for (const auto& item : m_stateSets[currentItem.getOrigin()]) {
		const auto* nextSymbol = item.getNextNonTerm();

		if (nextSymbol && nextSymbol->name == currentItem.getName()) {
			tryEmplace(m_stateSets[i], item.createAdvanced(1));
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
