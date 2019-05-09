#include "StandardLibrary.hpp"

#include "CoreFunction.hpp"

#include <iostream>

namespace app::standard_functions
{
    class FunctionPrint final : public CoreFunction
    {
    public:
        explicit FunctionPrint(const bool insertNewLine) : m_insertNewLine(insertNewLine) {}

        void call(Evaluator& evaluator) override
        {
            evaluator.popFunctionArgument().deref().print();
            if (m_insertNewLine) {
                printf("\n");
            }
        }

    private:
        bool m_insertNewLine = false;
    };

    class FunctionRead final : public CoreFunction
    {
    public:
        void call(Evaluator& evaluator) override
        {
            std::string input;
            std::getline(std::cin, input);

            evaluator.push(Symbol{ input, Symbol::ValueCategory::Rvalue });
        }
    };
}

namespace app::standard_objects
{
    
}

app::StandardLibrary::StandardLibrary()
{
    registerMember("print", std::make_shared<standard_functions::FunctionPrint>(false));
    registerMember("println", std::make_shared<standard_functions::FunctionPrint>(true));
    registerMember("readln", std::make_shared<standard_functions::FunctionRead>());
}
