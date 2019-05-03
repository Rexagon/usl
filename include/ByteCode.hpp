#pragma once

#include <string>
#include <vector>
#include <variant>
#include <optional>

namespace app
{
	namespace opcode
	{
		enum Code
		{
			DECL,
			ASSIGN,
			DEREF,

			POP,

			NOT,
			UNM,

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

		const char* getString(size_t code);
	}

	using ByteCodeItem = std::variant<std::nullopt_t, bool, double, std::string, std::string_view, opcode::Code>;
	using StackItem = std::variant<std::nullopt_t, bool, double, std::string, std::string_view>;

	using ByteCode = std::vector<ByteCodeItem>;
}
