#include "StandardLibrary.hpp"

#include "CoreFunction.hpp"

namespace app::standard_functions
{
    class FunctionPrint final : public CoreFunction
    {
    public:
        explicit FunctionPrint(const bool insertNewLine) : m_insertNewLine(insertNewLine) {}

        void call(Evaluator& evaluator) override
        {
            const auto arg = evaluator.popFunctionArgument();

            if (arg.getType() == Symbol::Type::String) {
                printf("\"");
            }
            arg.deref().print();
            if (arg.getType() == Symbol::Type::String) {
                printf("\"");
            }

            if (m_insertNewLine) {
                printf("\n");
            }
        }

    private:
        bool m_insertNewLine = false;
    };
}

app::StandardLibrary::StandardLibrary()
{
    registerMember("print", std::make_shared<standard_functions::FunctionPrint>(false));
    registerMember("println", std::make_shared<standard_functions::FunctionPrint>(true));
}
