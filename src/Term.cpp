#include "Term.hpp"

bool app::Term::Data::operator()(ParserState& state) const
{
	auto result = false;

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

app::Term::Term(const TokenType type) :
	m_data(std::make_shared<Data>(type))
{
}

app::Term::Term(const Term& l, const Term& r, const LogicalOp op) :
	m_data(std::make_shared<Data>(l.m_data, r.m_data, op))
{
}

app::Term::Term(const Term& c, const size_t n) :
	m_data(std::make_shared<Data>(c.m_data, n))
{
}

bool app::Term::operator()(ParserState& state) const
{
	if (m_data == nullptr) {
		return true;
	}

	return (*m_data)(state);
}

app::Term app::Term::operator|(const Term& other) const
{
	return Term(*this, other, LogicalOp::Or);
}

app::Term app::Term::operator>>(const Term& other) const
{
	return Term(*this, other, LogicalOp::And);
}

app::Term app::Term::operator++() const
{
	return Term(*this, 0);
}

app::Term app::Term::operator+() const
{
	return Term(*this, 1);
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

	state.it = nextToken + 1;
	return true;
}

bool app::Term::computeBoolean(ParserState& state, const BooleanData& boolean)
{
	switch (std::get<2>(boolean)) {
	case LogicalOp::Or:
		return (*std::get<0>(boolean))(state) || (*std::get<1>(boolean))(state);

	case LogicalOp::And:
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
