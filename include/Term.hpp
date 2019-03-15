#pragma once

#include <tuple>
#include <variant>
#include <optional>

#include "Grammar.hpp"

namespace app
{
	using TokenIterator = std::vector<Token>::const_iterator;

	struct ParserState final
	{
		TokenIterator it;
		const std::vector<Token>& tokens;
	};

	class Term final : std::enable_shared_from_this<Term>
	{
		enum class BooleanType
		{
			Or,
			And,
		};

		using TermPtr = std::shared_ptr<Term const>;

		using BooleanData = std::tuple<TermPtr, TermPtr, BooleanType>;
		using SequenceData = std::pair<TermPtr, size_t>;

	public:
		Term() = default;
		Term(const Term& term);

		explicit Term(TokenType type);
		Term(TermPtr, TermPtr, BooleanType op);
		Term(TermPtr, size_t n);

		bool operator()(ParserState& state) const;

		Term operator++() const;
		Term operator+() const;

		Term& operator=(Term&& other) noexcept;

		friend Term operator||(Term&& left, Term&& right);
		friend Term operator||(const Term& left, Term&& right);
		friend Term operator||(Term&& left, const Term& right);
		friend Term operator||(const Term& left, const Term& right);

		friend Term operator&&(Term&& left, Term&& right);
		friend Term operator&&(const Term& left, Term&& right);
		friend Term operator&&(Term&& left, const Term& right);
		friend Term operator&&(const Term& left, const Term& right);

	private:

		static bool checkTokenType(ParserState& state, const TokenType& type);
		static bool computeBoolean(ParserState& state, const BooleanData& boolean);
		static bool computeSequence(ParserState& state, const SequenceData& sequence);

		std::optional<std::variant<TokenType, BooleanData, SequenceData>> m_data;
	};
}
