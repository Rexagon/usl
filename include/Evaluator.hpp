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

                if constexpr (std::is_same_v<T, std::string_view>) {
                    visitor(findVariable(arg));
                }
                else {
                    visitor(arg);
                }
            }, item);
        }

        template<typename F>
        void visitSymbolsPair(F&& visitor, StackItem& itemLeft, StackItem& itemRight)
        {
            visitSymbol([this, &visitor, &itemRight](Symbol& symbolLeft) {
                visitSymbol([&visitor, &symbolLeft](Symbol& symbolRight) {
                    visitor(symbolLeft, symbolRight);
                }, itemRight);
            }, itemLeft);
        }

        void printState(bool showVariables);

        bool m_loggingEnabled;

        size_t m_position = 0;

        std::deque<std::unordered_map<std::string_view, Symbol>> m_blocks;

        std::deque<StackItem> m_stack;
        std::deque<Symbol> m_argumentsStack;
        std::stack<Pointer> m_pointerStack;
    };
}
