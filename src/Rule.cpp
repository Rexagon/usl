#include "Rule.hpp"

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

bool app::RuleSet::operator!=(const RuleSet& other) const
{
	return rules != other.rules;
}

app::EarleyState::EarleyState(const EarleyState& state, size_t n) :
	m_item(state.m_item), m_name(state.m_name), m_origin(state.m_origin), m_next(state.m_next + n)
{
}

app::EarleyState::EarleyState(std::string_view name, const RuleSet& set, const size_t origin, const size_t next) :
	m_item(set), m_name(name), m_origin(origin), m_next(next)
{
}

app::EarleyState::Type app::EarleyState::getNextType() const
{
	auto result = Type::Null;

	if (m_next < m_item.rules.size()) {
		std::visit([&result](auto && arg) {
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, Term>) {
				result = Type::Term;
			}
			else {
				result = Type::NonTerm;
			}
		}, m_item.rules[m_next]);
	}

	return result;
}

const app::Term* app::EarleyState::getNextTerm() const
{
	if (m_next >= m_item.rules.size()) {
		return nullptr;
	}

	return std::get_if<Term>(&m_item.rules[m_next]);
}

const app::NonTerm* app::EarleyState::getNextNonTerm() const
{
	if (m_next >= m_item.rules.size()) {
		return nullptr;
	}

	return std::get_if<NonTerm>(&m_item.rules[m_next]);
}

std::string_view app::EarleyState::getName() const
{
	return m_name;
}

size_t app::EarleyState::getOrigin() const
{
	return m_origin;
}

size_t app::EarleyState::getNextPosition() const
{
	return m_next;
}

size_t app::EarleyState::getEndPosition() const
{
	return m_origin + m_item.rules.size();
}

bool app::EarleyState::operator==(const EarleyState& other) const
{
	return m_item == other.m_item && m_origin == other.m_origin && m_next == other.m_next;
}

bool app::EarleyState::operator!=(const EarleyState& other) const
{
	return !(*this == other);
}

app::RuleCases::RuleCases(const Term& t) :
	m_sets({RuleSet{{t}}})
{
}

app::RuleCases::RuleCases(const NonTerm& t) :
	m_sets({ RuleSet{{t}} })
{
}

app::RuleCases::RuleCases(const RuleSet& t) :
	m_sets({t})
{
}

void app::RuleCases::setName(const std::string& name)
{
	m_name = name;
}

std::vector<app::EarleyState> app::RuleCases::generateStates(const size_t begin, size_t next) const
{
	std::vector<EarleyState> result;
	result.reserve(m_sets.size());

	for (const auto& set : m_sets) {
		result.emplace_back(m_name, set, begin, next);
	}
	return result;
}

app::RuleCases app::RuleCases::operator|(const RuleSet& r) const
{
	auto result = *this;
	result.m_sets.emplace_back(r);
	return result;
}

app::RuleCases app::RuleCases::operator|(const RuleVariant& r) const
{
	auto result = *this;
	result.m_sets.emplace_back(RuleSet{{r}});
	return result;
}

app::RuleCases app::operator|(const RuleSet& l, const RuleSet& r)
{
	RuleCases result;
	result.m_sets = { l, r };
	return result;
}

app::RuleCases app::operator|(const RuleSet& l, const RuleVariant& r)
{
	RuleCases result;
	result.m_sets = { l, RuleSet{{r}} };
	return result;
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
