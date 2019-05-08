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
		void handleAssign(const ByteCode& bytecode, size_t& position);
		void handleDeref(const ByteCode& bytecode, size_t& position);
		void handlePop(const ByteCode& bytecode, size_t& position);
		void handleUnaryOperator(const ByteCode& bytecode, size_t& position, opcode::Code op);
		void handleBinaryOperator(const ByteCode& bytecode, size_t& position, opcode::Code op);
		void handleControl(const ByteCode& bytecode, size_t& position, opcode::Code op);
		void handleBlocks(const ByteCode& bytecode, size_t& position, opcode::Code op);


        void visitSymbol(std::function<void(const Symbol&)> visitor, const StackItem& item)
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
        void visitSymbolsPair(F visitor, const StackItem& itemLeft, const StackItem& itemRight)
        {
            visitSymbol([this, &visitor, &itemRight](const auto& symbolLeft) {
                visitSymbol([&visitor, &symbolLeft](const auto& symbolRight) {
                    visitor(symbolLeft, symbolRight);
                }, itemRight);
            }, itemLeft);
        }

		Symbol& findVariable(std::string_view name);

        void printStack();

		std::vector<std::unordered_set<std::string_view>> m_blocks;
		std::unordered_map<std::string_view, Symbol> m_variables;

		std::deque<StackItem> m_stack;
		std::stack<Pointer> m_pointerStack;
	};
}
