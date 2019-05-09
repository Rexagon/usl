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

        EarleyItem(size_t name, const RuleSet& set, size_t origin, size_t next = 0);

        EarleyItem createAdvanced(size_t n) const;

        bool isEmpty() const;
        bool isComplete() const;

        const RuleSet& getRuleSet() const;

        NextType getNextType() const;
        const Term* getNextTerm() const;
        const NonTerm* getNextNonTerm() const;

        size_t getName() const;

        size_t getOrigin() const;
        size_t getNextPosition() const;

        size_t getEndPosition() const;

        void print() const;

        bool operator==(const EarleyItem& other) const;

    private:
        const RuleSet& m_set;

        size_t m_name;
        size_t m_origin = 0;
        size_t m_next = 0;
    };
}
