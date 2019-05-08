#pragma once

#include <deque>
#include <stack>
#include <stdexcept>
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
		void eval(const ByteCode& bytecode);

	private:
		void handleDecl(const ByteCode& bytecode, size_t& position, opcode::Code op);
		void handleAssign(const ByteCode& bytecode, size_t& position, opcode::Code op);
		void handleDeref(const ByteCode& bytecode, size_t& position);
		void handlePop(const ByteCode& bytecode, size_t& position);
		void handleUnaryOperator(const ByteCode& bytecode, size_t& position, opcode::Code op);
		void handleBinaryOperator(const ByteCode& bytecode, size_t& position, opcode::Code op);
		void handleControl(const ByteCode& bytecode, size_t& position, opcode::Code op);
		void handleArguments(const ByteCode& byteCode, size_t& position, opcode::Code op);
		void handleBlocks(const ByteCode& bytecode, size_t& position, opcode::Code op);

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

		Symbol& findVariable(std::string_view name);

        void printStack();

		std::vector<std::unordered_set<std::string_view>> m_blocks;
		std::unordered_map<std::string_view, Symbol> m_variables;

		std::deque<StackItem> m_stack;
        std::deque<StackItem> m_argumentsStack;
		std::stack<Pointer> m_pointerStack;
	};
}
