#include "Parser.hpp"

#include <stack>
#include <chrono>
#include <functional>

app::Parser::Parser() :
	m_grammar(ParserGrammar::create())
{
}

void app::Parser::parse(const std::vector<Token>& tokens)
{
	const auto BEFORE = std::chrono::high_resolution_clock::now();

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

	// Validate result
	if (m_stateSets.size() != tokens.size() + 1) {
		throw std::runtime_error("Unexpected end of stream");
	}

	const EarleyItem* finalItem = nullptr;
	for (const auto& item : m_stateSets.back()) {
		if (item.getNextType() == EarleyItem::NextType::Null &&
			item.getOrigin() == 0 &&
			item.getName() == parser_grammar::STARTING_RULE)
		{
			finalItem = &item;
		}
	}

	if (finalItem == nullptr) {
		throw std::runtime_error("Input is invalid");
	}

	// Prepare for generating AST
	using CompletedItem = std::pair<const EarleyItem*, size_t>;

	std::vector<std::list<CompletedItem>> completedItems;
	completedItems.resize(m_stateSets.size());

	for (size_t i = 0; i < m_stateSets.size(); ++i) {
		for (const auto& item : m_stateSets[i]) {
			if (item.isComplete() && 
				item.getOrigin() != i &&
				item.getRuleSet().isImportant) 
			{
				completedItems[item.getOrigin()].emplace_front(&item, i);
			}
		}
	}

	/*for (size_t i = 0; i < completedItems.size(); ++i) {
		printf("==%zu==\n", i);

		for (const auto& item : completedItems[i]) {
			printf("(%zu) ", item.second);
			item.first->print();
		}

		printf("\n");
	}*/

	// Generate AST
	struct SyntaxNode
	{
		CompletedItem value {nullptr, 0};
		const Token* token = nullptr;

		std::list<std::unique_ptr<SyntaxNode>> children;
	};

	SyntaxNode root;

	std::stack<SyntaxNode*> stack;
	stack.push(&root);

	for (size_t i = 0; i < tokens.size(); ++i) {
		while (stack.top()->value.first != nullptr &&
			i >= stack.top()->value.second)
		{
			stack.pop();
		}

		for (const auto& item : completedItems[i]) {
			if (stack.top()->value.first != nullptr &&
				stack.top()->value.second < item.second) 
			{
				stack.pop();
				auto last = std::move(stack.top()->children.back());
				stack.top()->children = std::move(last->children);
			}

			auto node = std::make_unique<SyntaxNode>();
			node->value = item;

			auto nodePtr = node.get();
			stack.top()->children.emplace_back(std::move(node));

			stack.push(nodePtr);
		}

		auto leaf = std::make_unique<SyntaxNode>();
		leaf->token = &tokens[i];
		stack.top()->children.emplace_back(std::move(leaf));
	}

	const auto AFTER = std::chrono::high_resolution_clock::now();

	using MilliDuration = std::chrono::duration<double, std::milli>;
	printf("AST generated in %f ms\n", std::chrono::duration_cast<MilliDuration>(AFTER - BEFORE).count());

	std::unordered_set<size_t> depthMask;
	std::function<void(const SyntaxNode*, size_t)> printTree;
	printTree = [&printTree, &depthMask](const SyntaxNode * node, const size_t depth) {
		for (size_t i = 0; i < depth; ++i) {
			if (i == depth - 1) {
				printf("*--");
			}
			else {
				printf("%s  ", depthMask.find(i) == depthMask.end() ? " " : "|");
			}
		}

		if (node->value.first != nullptr) {
			printf("(%zu) ", node->value.second);
			node->value.first->print();
		}
		else if(node->token) {
			printf("Term(%zu)\n", node->token->first);
		}

		if (!node->children.empty()) {
			depthMask.emplace(depth);

			for (auto it = node->children.begin(); it != node->children.end(); ++it) {
				if (*it == node->children.back()) {
					depthMask.erase(depth);
				}
				printTree(it->get(), depth + 1);
			}
		}
	};

	printTree(&root, 0);
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
