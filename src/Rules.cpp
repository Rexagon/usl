#include "Rules.hpp"

#include "EarleyItem.hpp"

#include <cassert>
#include <unordered_set>

void app::SyntaxNode::translate(CommandBuffer& cb)
{
    const auto* item = std::get_if<CompletedItem>(&value);
    if (item != nullptr) {
        item->first->getRuleSet().translator(cb, *this);
    }
}

void app::SyntaxNode::print() const
{
    const auto* item = std::get_if<CompletedItem>(&value);
    if (item != nullptr) {
        item->first->print();
        return;
    }

    const auto* token = *std::get_if<const Token*>(&value);
    if (token != nullptr){
        printf("Token (%s)\n", std::string{token->second}.c_str());
    }
}

void app::SyntaxNode::printTree(const SyntaxNode& root)
{
    std::unordered_set<size_t> depthMask;

    std::function<void(const SyntaxNode*, size_t)> printHelper;
    printHelper = [&printHelper, &depthMask](const SyntaxNode * node, const size_t depth) {
        for (size_t i = 0; i < depth; ++i) {
            if (i == depth - 1) {
                printf("*--");
            }
            else {
                printf("%s  ", depthMask.find(i) == depthMask.end() ? " " : "|");
            }
        }

        node->print();

        if (!node->children.empty()) {
            depthMask.emplace(depth);

            for (auto it = node->children.begin(); it != node->children.end(); ++it) {
                if (*it == node->children.back()) {
                    depthMask.erase(depth);
                }
                printHelper(it->get(), depth + 1);
            }
        }
    };

    printHelper(&root, 0);
}

app::Rules::Rules(const std::vector<app::RuleSet> &sets) :
        m_sets(sets)
{
}

void app::Rules::setName(const size_t name)
{
	m_name = name;
}

std::vector<app::EarleyItem> app::Rules::generateEarleyItems(const size_t begin) const
{
	std::vector<EarleyItem> result;
	result.reserve(m_sets.size());

	for (const auto& set : m_sets) {
		result.emplace_back(m_name, set, begin, 0);
	}
	return result;
}

const std::vector<app::RuleSet>& app::Rules::getRuleSets() const
{
	return m_sets;
}

bool app::Term::operator==(const Term &other) const {
    return type == other.type;
}

bool app::NonTerm::operator==(const NonTerm &other) const {
    return name == other.name;
}

void app::RuleSet::defaultTranslator(CommandBuffer& cb, SyntaxNode& node)
{
    for (auto& child : node.children) {
        const auto* value = std::get_if<CompletedItem>(&child->value);
        if (value != nullptr) {
            cb.translate([&child](CommandBuffer& cb) {
                child->translate(cb);
            });
        }
    }
}

bool app::RuleSet::operator==(const RuleSet &other) const {
    return rules == other.rules;
}
