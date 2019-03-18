#pragma once

#include <stack>
#include <variant>
#include <unordered_map>
#include <unordered_set>

#include "Symbol.hpp"

namespace app
{
	namespace opcode
	{
		enum Code
		{
			DECL,
			ASSIGN,

			PUSH,
			POP,

			NOT,
			UNM,
			INC,
			DEC,

			ADD,
			SUB,
			MUL,
			DIV,

			AND,
			OR,
			EQ,
			NEQ,
			LT,
			LE,
			GT,
			GE,

			JMP,
			CALL,
			RET,

			DEFBLOCK,
			DELBLOCK,

			Count,
		};

		constexpr auto OPCODE_COUNT = Count;
	}

	using Item = std::variant<Null, bool, double, std::string, std::string_view, opcode::Code>;
	using StackItem = std::variant<Null, bool, double, std::string, std::string_view>;

	class Evaluator final
	{
		using ByteCode = std::vector<Item>;

	public:
		void eval(const ByteCode& bytecode);

	private:
		void handleDecl(const ByteCode& bytecode, size_t& position);
		void handleAssign(const ByteCode& bytecode, size_t& position);
		void handlePushPop(const ByteCode& bytecode, size_t& position, opcode::Code op);
		void handleUnary(const ByteCode& bytecode, size_t& position, opcode::Code op);
		void handleBinaryMath(const ByteCode& bytecode, size_t& position, opcode::Code op);
		void handleBinaryLogic(const ByteCode& bytecode, size_t& position, opcode::Code op);
		void handleControl(const ByteCode& bytecode, size_t& position, opcode::Code op);
		void handleBlocks(const ByteCode& bytecode, size_t& position, opcode::Code op);

		std::stack<std::unordered_set<std::string_view>> m_blocks;
		std::unordered_map<std::string_view, Symbol> m_variables;

		std::stack<StackItem> m_stack;
	};
}