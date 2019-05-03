#pragma once

#include <stack>
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

		std::vector<std::unordered_set<std::string_view>> m_blocks;
		std::unordered_map<std::string_view, Symbol> m_variables;

		std::stack<StackItem> m_stack;
	};
}
