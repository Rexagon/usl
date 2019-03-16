#include "EarleyItem.hpp"

app::EarleyItem::EarleyItem(const std::string_view name, const RuleSet& set, 
	const size_t origin, const size_t next) :
	m_item(set), m_name(name), m_origin(origin), m_next(next)
{
}

app::EarleyItem app::EarleyItem::createAdvanced(size_t n) const
{
	auto result{ *this };
	result.m_next = std::min(result.m_item.rules.size(), result.m_next + n);
	return result;
}

void app::EarleyItem::print() const
{
	printf("(%zu) %s -> ", m_origin, std::string{ m_name }.c_str());

	for (size_t i = 0; i < m_item.rules.size(); ++i) {
		if (i == m_next) {
			printf(". ");
		}

		std::visit([](auto && arg) {
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, Term>) {
				printf("Term(%d) ", arg.type);
			}
			else {
				printf("%s ", arg.name.c_str());
			}
		}, m_item.rules[i]);
	}

	if (m_item.rules.empty() || m_next >= m_item.rules.size()) {
		printf(". ");
	}

	printf("\n");
}

app::EarleyItem::NextType app::EarleyItem::getNextType() const
{
	auto result = NextType::Null;

	if (m_next < m_item.rules.size()) {
		std::visit([&result](auto && arg) {
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, Term>) {
				result = NextType::Term;
			}
			else {
				result = NextType::NonTerm;
			}
			}, m_item.rules[m_next]);
	}

	return result;
}

const app::Term* app::EarleyItem::getNextTerm() const
{
	if (m_next >= m_item.rules.size()) {
		return nullptr;
	}

	return std::get_if<Term>(&m_item.rules[m_next]);
}

const app::NonTerm* app::EarleyItem::getNextNonTerm() const
{
	if (m_next >= m_item.rules.size()) {
		return nullptr;
	}

	return std::get_if<NonTerm>(&m_item.rules[m_next]);
}

std::string_view app::EarleyItem::getName() const
{
	return m_name;
}

size_t app::EarleyItem::getOrigin() const
{
	return m_origin;
}

size_t app::EarleyItem::getNextPosition() const
{
	return m_next;
}

size_t app::EarleyItem::getEndPosition() const
{
	return m_origin + m_item.rules.size();
}

bool app::EarleyItem::operator==(const EarleyItem& other) const
{
	return m_item == other.m_item && m_origin == other.m_origin && m_next == other.m_next;
}
