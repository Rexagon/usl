#include "Term.hpp"

app::Term::Term(const Term& term) :
	m_data(std::move(term.m_data))
{
}

app::Term::Term(const TokenType type) :
	m_data(type)
{
}

app::Term::Term(TermPtr l, TermPtr r, const BooleanType op) :
	m_data(std::tuple{ l, r, op })
{
}

app::Term::Term(TermPtr c, const size_t n) :
	m_data(std::pair{ c, n })
{
}

bool app::Term::operator()(ParserState& state) const
{
	if (!m_data.has_value()) {
		return true;
	}

	auto result = false;

	const auto& data = m_data.value();
	std::visit([&](auto && arg) {
		using T = std::decay_t<decltype(arg)>;

		if constexpr (std::is_same_v<T, TokenType>) {
			printf("Check token type\n");
			result = checkTokenType(state, arg);
		}
		else if constexpr (std::is_same_v<T, BooleanData>) {
			printf("Check boolean\n");
			result = computeBoolean(state, arg);
		}
		else {
			printf("Check sequence\n");
			result = computeSequence(state, arg);
		}
	}, data);

	return result;
}

app::Term app::Term::operator++() const
{
	return Term(shared_from_this(), 0);
}

app::Term app::Term::operator+() const
{
	return Term(shared_from_this(), 1);
}

app::Term& app::Term::operator=(Term&& other) noexcept
{
	m_data = std::move(other.m_data);
	return *this;
}

bool app::Term::checkTokenType(ParserState& state, const TokenType& type)
{
	const auto nextToken = state.it;

	if (nextToken == state.tokens.end()) {
		throw std::runtime_error("Expected token " +
			std::to_string(static_cast<int>(type)) + "reached end");
	}
	if (nextToken->first != type) {
		return false;
	}

	state.it = nextToken;
	return true;
}

bool app::Term::computeBoolean(ParserState& state, const BooleanData& boolean)
{
	switch (std::get<2>(boolean)) {
	case BooleanType::Or:
		return (*std::get<0>(boolean))(state) || (*std::get<1>(boolean))(state);

	case BooleanType::And:
		return (*std::get<0>(boolean))(state) && (*std::get<1>(boolean))(state);

	default:
		return true;
	}
}

bool app::Term::computeSequence(ParserState& state, const SequenceData& sequence)
{
	size_t i = 0;
	while (state.it != state.tokens.end() && (*sequence.first)(state)) {
		++i;
	}

	return i >= sequence.second;
}

app::Term app::operator||(Term&& left, Term&& right)
{
	printf("|| rr\n");

	return Term(
		std::make_shared<Term>(std::move(left)), 
		std::make_shared<Term>(std::move(right)), 
		Term::BooleanType::Or);
}

app::Term app::operator||(const Term& left, Term&& right)
{
	printf("|| lr\n");

	return Term(
		Term::TermPtr(&left)->shared_from_this(),
		std::make_shared<Term>(std::move(right)), 
		Term::BooleanType::Or);
}

app::Term app::operator||(Term&& left, const Term& right)
{
	printf("|| rl\n");

	return Term(
		std::make_shared<Term>(std::move(left)),
		Term::TermPtr(&right)->shared_from_this(),
		Term::BooleanType::Or);
}

app::Term app::operator||(const Term& left, const Term& right)
{
	printf("|| ll\n");

	return Term(
		Term::TermPtr(&left)->shared_from_this(),
		Term::TermPtr(&right)->shared_from_this(),
		Term::BooleanType::Or);
}

app::Term app::operator&&(Term&& left, Term&& right)
{
	printf("&& rr\n");

	return Term(
		std::make_shared<Term>(std::move(left)),
		std::make_shared<Term>(std::move(right)),
		Term::BooleanType::And);
}

app::Term app::operator&&(const Term& left, Term&& right)
{
	printf("&& lr\n");

	return Term(
		Term::TermPtr(&left)->shared_from_this(),
		std::make_shared<Term>(std::move(right)),
		Term::BooleanType::And);
}

app::Term app::operator&&(Term&& left, const Term& right)
{
	printf("&& rl\n");

	return Term(
		std::make_shared<Term>(std::move(left)),
		Term::TermPtr(&right)->shared_from_this(),
		Term::BooleanType::And);
}

app::Term app::operator&&(const Term& left, const Term& right)
{
	printf("&& ll\n");

	return Term(
		Term::TermPtr(&left)->shared_from_this(),
		Term::TermPtr(&right)->shared_from_this(),
		Term::BooleanType::And);
}
