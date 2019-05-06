#pragma once

#include <string>
#include <vector>
#include <variant>
#include <optional>

namespace details {
    template<typename T, typename... Ts>
    struct is_any_of : std::bool_constant<(std::is_same_v<T, Ts> || ...)> {};

    template<typename T, typename T1, typename... Ts>
    inline constexpr bool is_any_of_v = is_any_of<T, T1, Ts...>::value;
}

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

		constexpr bool isMathOp(app::opcode::Code op)
		{
			return (op >= ADD) && (op <= DIV);
		}

		constexpr bool isBoolOp(app::opcode::Code op)
		{
			return (op >= AND) && (op <= GE);
		}

		const char* getString(size_t code);
	}

	using ByteCodeItem = std::variant<std::nullopt_t, bool, double, std::string, std::string_view, opcode::Code>;
	using StackItem = std::variant<std::nullopt_t, bool, double, std::string, std::string_view>;

	void print(const StackItem& item);

	template<typename T>
	inline constexpr bool dereferencable = details::is_any_of_v<T, std::nullopt_t, bool, double, std::string>;

	using ByteCode = std::vector<ByteCodeItem>;
}
