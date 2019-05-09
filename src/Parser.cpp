#include "Parser.hpp"

#include <stack>
#include <chrono>
#include <functional>

#include "ByteCode.hpp"

app::Parser::Parser(const bool loggingEnabled) :
    m_loggingEnabled(loggingEnabled), m_grammar(ParserGrammar::create())
{
}

std::vector<app::ByteCodeItem> app::Parser::parse(const std::vector<Token>& tokens)
{
    const auto timeBegin = std::chrono::high_resolution_clock::now();

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
        throw std::runtime_error{ "Unexpected end of stream" };
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
        throw std::runtime_error{ "Input is invalid" };
    }

    // Prepare for generating AST
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
    SyntaxNode root;

    std::stack<SyntaxNode*> stack;
    stack.push(&root);

    for (size_t i = 0; i < tokens.size(); ++i) {
        // Pop all completed items
        while (true) {
            const auto* value = std::get_if<CompletedItem>(&stack.top()->value);
            if (value == nullptr || i < value->second) {
                break;
            }
            stack.pop();
        }

        // Attach other children as terminals
        for (const auto& item : completedItems[i]) {
            const auto* value = std::get_if<CompletedItem>(&stack.top()->value);

            if (value != nullptr && value->second < item.second) {
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
        leaf->value = &tokens[i];

        stack.top()->children.emplace_back(std::move(leaf));
    }

    const auto timeAfter = std::chrono::high_resolution_clock::now();

    if (m_loggingEnabled) {
        using MilliDuration = std::chrono::duration<double, std::milli>;
        printf("AST generated in %f ms\n", std::chrono::duration_cast<MilliDuration>(timeAfter - timeBegin).count());
        SyntaxNode::printTree(root);
    }

    // Translate to bytecode
    CommandBuffer commandBuffer;
    RuleSet::defaultTranslator(commandBuffer, root);

    return commandBuffer.generate();
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
