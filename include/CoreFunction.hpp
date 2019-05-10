#pragma once

#include <functional>

#include "Evaluator.hpp"

namespace app
{
    class CoreFunction
    {
    public:
        virtual void call(Evaluator& evaluator) = 0;

    protected:
        virtual ~CoreFunction() = default;
    };

    class SimpleCoreFunction final : public CoreFunction
    {
    public:
        explicit SimpleCoreFunction(const std::function<void(Evaluator&)>& function) :
            m_function(function)
        {}

        void call(Evaluator& evaluator) override
        {
            m_function(evaluator);
        }

    private:
        std::function<void(Evaluator&)> m_function;
    };
}
