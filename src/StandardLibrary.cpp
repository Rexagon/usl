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

    class Math final : public CoreObject
    {
    public:
        Math()
        {
            const auto createUnaryMathOperation = [](auto && op) {
                return [&op](Evaluator & evaluator) {
                    const auto arg = evaluator.popFunctionArgument().unref();

                    auto result = 0.0;
                    arg.visit([&op, &result](auto && arg) {
                        using T = std::decay_t<decltype(arg)>;

                        if constexpr (details::is_any_of_v<T, bool, double>) {
                            result = op(arg);
                        }
                        else {
                            throw std::runtime_error{ "Wrong argument type" };
                        }
                    });

                    evaluator.push(Symbol{ result, Symbol::ValueCategory::Rvalue });
                };
            };

            const auto createBinaryMathOperation = [](auto&& op) {
                return [&op](Evaluator & evaluator) {
                    const auto argLeft = evaluator.popFunctionArgument().unref();
                    const auto argRight = evaluator.popFunctionArgument().unref();

                    auto result = 0.0;
                    argLeft.visit([&op, &result, &argRight](auto && argLeft) {
                        argRight.visit([&op, &result, &argLeft](auto && argRight) {
                            using Tl = std::decay_t<decltype(argLeft)>;
                            using Tr = std::decay_t<decltype(argRight)>;

                            if constexpr (
                                details::is_any_of_v<Tl, bool, double> &&
                                details::is_any_of_v<Tr, bool, double>)
                            {
                                result = op(static_cast<double>(argLeft), static_cast<double>(argRight));
                            }
                            else {
                                throw std::runtime_error{ "Wrong argument type" };
                            }
                        });
                    });

                    evaluator.push(Symbol{ result, Symbol::ValueCategory::Rvalue });
                };
            };

            const std::unordered_map<std::string_view, double(*)(double)> unaryFunctions = {
                {"abs", std::fabs},
                {"sqrt", std::sqrt},
                {"sin", std::sin},
                {"asin", std::asin},
                {"cos", std::cos},
                {"acos", std::acos},
                {"tan", std::tan},
                {"atan", std::atan},
                {"ceil", std::ceil},        // nearest integer not less than the given v
                {"floor", std::floor},      // nearest integer not greater than the given value
                {"trunc", std::trunc},      // nearest integer not greater in magnitude than the given value 
                {"round", std::round},      // nearest integer, rounding away from zero in halfway cases 
            };

            for (const auto&[name, function] : unaryFunctions) {
                registerMember(name, std::make_shared<SimpleCoreFunction>(createUnaryMathOperation(function)));
            }

            const std::unordered_map<std::string_view, double(*)(double, double)> binaryFunctions = {
                {"mod", std::fmod},
                {"min", std::fmin},
                {"max", std::fmax},
            };

            for (const auto& [name, function] : binaryFunctions) {
                registerMember(name, std::make_shared<SimpleCoreFunction>(createBinaryMathOperation(function)));
            }
        }
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

    registerMember("Math", std::make_shared<standard_objects::Math>());

    registerMember("LinkedListNode", std::make_shared<standard_objects::LinkedListNode>());
    registerMember("Pair", std::make_shared<standard_objects::Pair>());
}
