#include "StandardLibrary.hpp"

#include "CoreFunction.hpp"

#include <iostream>

namespace app::standard_objects
{
    class LinkedListNode final : public CoreObject
    {
    public:
        LinkedListNode()
        {
            registerMember("new", std::make_shared<SimpleCoreFunction>([](Evaluator& evaluator) {
                evaluator.push(Symbol{ std::make_shared<LinkedListNode>(), Symbol::ValueCategory::Rvalue });
            }));

            registerMember("value", std::nullopt);

            const auto createSetter = [](Symbol& symbol) {
                return [&symbol](Evaluator & evaluator) {
                    const auto node = evaluator.popFunctionArgument().unref();

                    if (node.getType() == Symbol::Type::Null) {
                        symbol = Symbol{ Symbol::ValueCategory::Lvalue };
                        return;
                    }

                    if (node.getType() != Symbol::Type::CoreObject) {
                        throw std::runtime_error{ "Wrong argument type" };
                    }

                    symbol = Symbol{ node.unref(), Symbol::ValueCategory::Lvalue };
                };
            };

            registerMember("set_next", std::make_shared<SimpleCoreFunction>(createSetter(m_next)));

            registerMember("get_next", std::make_shared<SimpleCoreFunction>([this](Evaluator & evaluator) {
                evaluator.push(Symbol{ m_next, Symbol::ValueCategory::Rvalue });
            }));

            registerMember("set_prev", std::make_shared<SimpleCoreFunction>(createSetter(m_prev)));

            registerMember("get_prev", std::make_shared<SimpleCoreFunction>([this](Evaluator & evaluator) {
                evaluator.push(Symbol{ m_prev, Symbol::ValueCategory::Rvalue });
            }));
        }

    private:
        Symbol m_next{ Symbol::ValueCategory::Lvalue };
        Symbol m_prev{ Symbol::ValueCategory::Lvalue };
    };

    class Pair final : public CoreObject
    {
    public:
        Pair()
        {
            registerMember("new", std::make_shared<SimpleCoreFunction>([](Evaluator & evaluator) {
                evaluator.push(Symbol{ std::make_shared<Pair>(), Symbol::ValueCategory::Rvalue });
            }));

            registerMember("first", std::nullopt);
            registerMember("second", std::nullopt);
        }
    };
}

app::StandardLibrary::StandardLibrary()
{
    registerMember("print", std::make_shared<SimpleCoreFunction>([](Evaluator& evaluator) {
        evaluator.popFunctionArgument().unref().print();
    }));

    registerMember("println", std::make_shared<SimpleCoreFunction>([](Evaluator & evaluator) {
        evaluator.popFunctionArgument().unref().print();
        printf("\n");
    }));

    registerMember("readln", std::make_shared<SimpleCoreFunction>([](Evaluator & evaluator) {
        std::string input;
        std::getline(std::cin, input);

        evaluator.push(Symbol{ input, Symbol::ValueCategory::Rvalue });
    }));

    registerMember("LinkedListNode", std::make_shared<standard_objects::LinkedListNode>());
    registerMember("Pair", std::make_shared<standard_objects::Pair>());
}
