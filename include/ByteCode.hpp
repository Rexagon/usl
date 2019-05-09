#pragma once

#include <string>
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
    enum class OpCode
    {
        DECLVAR,    // var, DECLVAR ->
        DECLFUN,    // var, ptr, DECLFUN ->
        ASSIGN,     // var, val/var, ASSIGN ->
        ASSIGNREF,  // var, var, ASSIGNREF ->
        DEREF,      // val/var, DEREF -> val

        POP,        // POP ->

        NOT,        // val/var, NOT -> val
        UNM,        // val/var, UNM -> val

        ADD,        // val/var, val/var, ADD -> val
        SUB,        // val/var, val/var, SUB -> val
        MUL,        // val/var, val/var, MUL -> val
        DIV,        // val/var, val/var, DIV -> val

        AND,        // val/var, val/var, AND -> val
        OR,         // val/var, val/var, OR -> val

        EQ,         // val/var, val/var, EQ -> val
        NEQ,        // val/var, val/var, NEQ -> val
        LT,         // val/var, val/var, LT -> val
        LE,         // val/var, val/var, LE -> val
        GT,         // val/var, val/var, GT -> val
        GE,         // val/var, val/var, GE -> val

        IF,         // bool, ptr (if true), ptr (if false), IF ->
        JMP,        // ptr, JMP ->
        CALL,       // var, CALL -> [push current ptr]
        RET,        // RET -> [pop current ptr]

        PUSHARG,    // val/var, PUSHARG ->
        POPARG,     // POPARG -> val/var

        DEFBLOCK,	// DEFBLOCK ->
        DELBLOCK,	// DELBLOCK ->

        Count,
    };

    constexpr auto OPCODE_COUNT = OpCode::Count;

    constexpr bool isUnaryMathOp(const OpCode op)
    {
        return (op >= OpCode::NOT) && (op <= OpCode::UNM);
    }

    constexpr bool isBinaryMathOp(const OpCode op)
    {
        return (op >= OpCode::ADD) && (op <= OpCode::DIV);
    }

    constexpr bool isLogicOp(const OpCode op)
    {
        return (op >= OpCode::AND) && (op <= OpCode::OR);
    }

    constexpr bool isComparisonOp(const OpCode op)
    {
        return (op >= OpCode::EQ) && (op <= OpCode::GE);
    }

    constexpr bool isControlOp(const OpCode op)
    {
        return (op >= OpCode::IF) && (op <= OpCode::RET);
    }

    std::string toString(OpCode code);

    using Pointer = size_t;

    using ByteCodeItem = std::variant<std::nullopt_t, bool, double, std::string, std::string_view, OpCode, Pointer>;

    void print(const ByteCodeItem & item);
}
