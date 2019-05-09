#pragma once

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
}
