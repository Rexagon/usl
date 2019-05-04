#pragma once

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
	public:
		void eval(const ByteCode& bytecode);

	private:
		void handleDecl(const ByteCode& bytecode, size_t& position);
		void handleAssign(const ByteCode& bytecode, size_t& position);
		void handleDeref(const ByteCode& bytecode, size_t& position);
		void handlePop(const ByteCode& bytecode, size_t& position);
		void handleUnary(const ByteCode& bytecode, size_t& position, opcode::Code op);
		void handleBinaryMath(const ByteCode& bytecode, size_t& position, opcode::Code op);
		void handleBinaryLogic(const ByteCode& bytecode, size_t& position, opcode::Code op);
		void handleControl(const ByteCode& bytecode, size_t& position, opcode::Code op);
		void handleBlocks(const ByteCode& bytecode, size_t& position, opcode::Code op);

		Symbol& findVariable(std::string_view name);

        template<typename F>
        void deref(F&& f, StackItem& item) {
            std::visit([this, &f](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (dereferencable<T>) {
                    f(arg);
                }
                else if constexpr (std::is_same_v<T, std::string_view>) {
                    findVariable(arg).deref(f);
                }
            }, item);
        }

        template<typename F>
        void deref(F&& f, StackItem& argLeft, StackItem& argRight)
        {
            deref([this, &f, &argRight](auto&& argL) {
                deref([&f, &argL](auto&& argR) {
                    f(argL, argR);
                }, argRight);
            }, argLeft);
        }

        void printStack();

		std::vector<std::unordered_set<std::string_view>> m_blocks;
		std::unordered_map<std::string_view, Symbol> m_variables;

		std::vector<StackItem> m_stack;
	};
}
