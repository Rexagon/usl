#pragma once

#include "Rules.hpp"

namespace app
{
	class EarleyItem final
	{
	public:
		enum class NextType
		{
			Term,
			NonTerm,
			Null,
		};

		EarleyItem(std::string_view name, const RuleSet& set, size_t origin, size_t next = 0);

		EarleyItem createAdvanced(size_t n) const;

		void print() const;

		bool isEmpty() const;

		NextType getNextType() const;
		const Term* getNextTerm() const;
		const NonTerm* getNextNonTerm() const;

		std::string_view getName() const;

		size_t getOrigin() const;
		size_t getNextPosition() const;

		size_t getEndPosition() const;

		bool operator==(const EarleyItem& other) const;

	private:
		const RuleSet& m_set;

		std::string_view m_name;
		size_t m_origin = 0;
		size_t m_next = 0;
	};
}
