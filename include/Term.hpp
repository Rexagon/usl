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
		enum class LogicalOp
		{
			Or,
			And,
		};

		struct Data;

		using DataPtr = std::shared_ptr<Data>;

		using BooleanData = std::tuple<DataPtr, DataPtr, LogicalOp>;
		using SequenceData = std::pair<DataPtr, size_t>;

		struct Data final
		{
			explicit Data(const TokenType type) 
				: data(type) 
			{}

			Data(const DataPtr& l, const DataPtr& r, LogicalOp op) 
				: data(std::tuple{ l, r, op}) 
			{}

			Data(const DataPtr& c, size_t n) 
				: data(std::pair{c, n}) 
			{}

			bool operator()(ParserState& state) const;

			std::variant<TokenType, BooleanData, SequenceData> data;
		};

	public:
		Term() = default;
		explicit Term(TokenType type);
		Term(const Term& l, const Term& r, LogicalOp op);
		Term(const Term& c, size_t n);

		bool operator()(ParserState& state) const;

		Term operator|(const Term& other) const;
		Term operator>>(const Term& other) const;
		Term operator++() const;
		Term operator+() const;

	private:
		static bool checkTokenType(ParserState& state, const TokenType& type);
		static bool computeBoolean(ParserState& state, const BooleanData& boolean);
		static bool computeSequence(ParserState& state, const SequenceData& sequence);

		DataPtr m_data{nullptr};
	};
}
