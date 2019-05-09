#pragma once

#include <deque>
#include <stack>
#include <functional>
#include <unordered_map>
#include <unordered_set>

#include "Symbol.hpp"

namespace app
{
    class Evaluator final
    {
        using StackItem = std::variant<Symbol, std::string_view>;

    public:
        explicit Evaluator(bool loggingEnabled);

        void eval(const std::vector<ByteCodeItem>& byteCode);

        void push(const Symbol& symbol);

        template<typename T>
        void registerVariable(std::string_view name, T&& value)
        {
            m_blocks.back().try_emplace(name, value, Symbol::ValueCategory::Lvalue);
        }

        Symbol& findVariable(std::string_view name);
        bool hasVariable(std::string_view name) const;

        void pushFunctionArgument(const Symbol& symbol);
        Symbol popFunctionArgument();

    private:
        void handleDecl(OpCode op);
        void handleAssign(OpCode op);
        void handleDeref();
        void handleStructRef();
        void handlePop();
        void handleUnaryOperator(OpCode op);
        void handleBinaryOperator(OpCode op);
        void handleControl(OpCode op);
        void handleArguments(OpCode op);
        void handleBlocks(OpCode op);

        template<typename F>
        void visitSymbol(F&& visitor, StackItem& item)
        {
            std::visit([this, &visitor](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;

                const auto unref = [&visitor](auto&& symbol) {
                    symbol.visit([&visitor, &symbol](auto&& data) {
                        using D = std::decay_t<decltype(data)>;

                        if constexpr (std::is_same_v<D, Symbol*>) {
                            visitor(*data);
                        }
                        else {
                            visitor(symbol);
                        }
                    });
                };

                if constexpr (std::is_same_v<T, std::string_view>) {
                    unref(findVariable(arg));
                }
                else {
                    unref(arg);
                }
            }, item);
        }

        template<typename F>
        void visitSymbolsPair(F&& visitor, StackItem& itemLeft, StackItem& itemRight)
        {
            visitSymbol([this, &visitor, &itemRight](auto&& symbolLeft) {
                visitSymbol([&visitor, &symbolLeft](auto&& symbolRight) {
                    visitor(symbolLeft, symbolRight);
                }, itemRight);
            }, itemLeft);
        }

        void printState(bool showVariables);

        bool m_loggingEnabled;

        size_t m_position = 0;

        std::vector<std::unordered_map<std::string_view, Symbol>> m_blocks;

        std::deque<StackItem> m_stack;
        std::deque<Symbol> m_argumentsStack;
        std::stack<Pointer> m_pointerStack;
    };
}
