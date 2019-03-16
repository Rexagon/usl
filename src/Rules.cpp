#include "Rules.hpp"

#include "EarleyItem.hpp"

app::Rules::Rules(const Term& t) :
	m_sets({RuleSet{{t}}})
{
}

app::Rules::Rules(const NonTerm& t) :
	m_sets({ RuleSet{{t}} })
{
}

app::Rules::Rules(const RuleSet& t) :
	m_sets({t})
{
}

void app::Rules::setName(const std::string& name)
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

app::Rules app::Rules::operator|(const RuleSet& r) const
{
	auto result = *this;
	result.m_sets.emplace_back(r);
	return result;
}

app::Rules app::Rules::operator|(const RuleVariant& r) const
{
	auto result = *this;
	result.m_sets.emplace_back(RuleSet{{r}});
	return result;
}

app::Rules app::operator|(const RuleSet& l, const RuleSet& r)
{
	Rules result;
	result.m_sets = { l, r };
	return result;
}

app::Rules app::operator|(const RuleSet& l, const RuleVariant& r)
{
	Rules result;
	result.m_sets = { l, RuleSet{{r}} };
	return result;
}

bool app::Term::operator==(const Term& other) const
{
	return type == other.type;
}

bool app::NonTerm::operator==(const NonTerm& other) const
{
	return name == other.name;
}

bool app::RuleSet::operator==(const RuleSet& other) const
{
	return rules == other.rules;
}

app::RuleSet app::operator>>(const RuleSet& l, const RuleVariant& r)
{
	RuleSet result;
	result.rules.insert(result.rules.end(), l.rules.begin(), l.rules.end());
	result.rules.insert(result.rules.end(), r);
	return result;
}

app::RuleSet app::operator>>(const RuleVariant& l, const RuleSet& r)
{
	RuleSet result;
	result.rules.insert(result.rules.end(), l);
	result.rules.insert(result.rules.end(), r.rules.begin(), r.rules.end());
	return result;
}

app::RuleSet app::operator>>(const RuleVariant& l, const RuleVariant& r)
{
	RuleSet result;
	result.rules = { l, r };
	return result;
}
