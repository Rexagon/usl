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
            DECLVAR,	// var, DECLVAR ->
            DECLFUN,    // var, ptr, DECLFUN ->
			ASSIGN,		// var, val/var, ASSIGN ->
			ASSIGNREF,  // var, var, ASSIGNREF ->
			DEREF,		// val/var, DEREF -> val

			POP,		// POP ->

			NOT,		// val/var, NOT -> val
			UNM,		// val/var, UNM -> val

			ADD,		// val/var, val/var, ADD -> val
			SUB,		// val/var, val/var, SUB -> val
			MUL,		// val/var, val/var, MUL -> val
			DIV,		// val/var, val/var, DIV -> val

			AND,		// val/var, val/var, AND -> val
			OR,			// val/var, val/var, OR -> val

			EQ,			// val/var, val/var, EQ -> val
			NEQ,		// val/var, val/var, NEQ -> val
			LT,			// val/var, val/var, LT -> val
			LE,			// val/var, val/var, LE -> val
			GT,			// val/var, val/var, GT -> val
			GE,			// val/var, val/var, GE -> val

			IF,			// bool, ptr (if true), ptr (if false), IF ->
			JMP,		// ptr, JMP ->
			CALL,		// var, CALL -> [push current ptr]
			RET,		// RET -> [pop current ptr]

			PUSHARG,    // val/var, PUSHARG ->
			POPARG,     // POPARG -> val/var

			DEFBLOCK,	// DEFBLOCK ->
			DELBLOCK,	// DELBLOCK ->

			Count,
		};

		constexpr auto OPCODE_COUNT = Count;

		constexpr bool isMathOp(app::opcode::Code op)
		{
			return (op >= ADD) && (op <= DIV);
		}

		constexpr bool isLogicOp(app::opcode::Code op)
		{
			return (op >= AND) && (op <= OR);
		}

        constexpr bool isComparationOp(app::opcode::Code op)
        {
            return (op >= EQ) && (op <= GE);
        }

		constexpr bool isControlOp(Code op)
		{
			return (op >= IF) && (op <= RET);
		}

		const char* toString(size_t code);
	}

	using Pointer = size_t;

	using ByteCodeItem = std::variant<
	        std::nullopt_t,
	        bool,
	        double,
	        std::string,
	        std::string_view,
	        opcode::Code,
	        Pointer>;

	void print(const ByteCodeItem& item);

	using ByteCode = std::vector<ByteCodeItem>;
}
