#include "Rules.hpp"

#include "EarleyItem.hpp"

#include <cassert>

void app::SyntaxNode::translate(CommandBuffer& cb)
{
    const auto* item = std::get_if<CompletedItem>(&value);
    if (item != nullptr) {
        item->first->getRuleSet().translator(cb, *this);
    }
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
        child->translate(cb);
    }
}

bool app::RuleSet::operator==(const RuleSet &other) const {
    return rules == other.rules;
}
