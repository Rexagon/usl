#pragma once

#include <array>
#include <regex>
#include <bitset>

#include "ByteCode.hpp"

namespace app
{
    namespace lexer_grammar {
        enum TokenType
        {
            KeywordLet,
            KeywordIf,
            KeywordElse,
            KeywordWhile,
            KeywordFor,
            KeywordBreak,
            KeywordContinue,
            KeywordFunction,
            KeywordReturn,
            KeywordRef,

            Null,
            Boolean,
            Identifier,
            String,
            Number,

            OperatorAssignment,
            OperatorOr,
            OperatorAnd,
            OperatorEq,
            OperatorNeq,
            OperatorLt,
            OperatorLeq,
            OperatorGt,
            OperatorGeq,
            OperatorPlus,
            OperatorMinus,
            OperatorMul,
            OperatorDiv,
            OperatorIncrement,
            OperatorDecrement,
            OperatorNegate,

            StructureReference,

            ParenthesisOpen,
            ParenthesisClose,
            BraceOpen,
            BraceClose,
            BracketOpen,
            BracketClose,

            Comma,
            Semicolon,

            CommentSingleLine,
            CommentMultiLine,

            Invalid, // also used as count
        };

        constexpr auto TOKEN_COUNT = Invalid;

        constexpr bool isValue(const size_t type)
        {
            return
                type == Null ||
                type == Boolean ||
                type == Identifier ||
                type == String ||
                type == Number;
        }

        constexpr bool isUseless(const size_t type)
        {
            return
                type == CommentSingleLine ||
                type == CommentMultiLine ||
                type == Invalid;
        }
    }

    using RegexArray = std::array<std::regex, lexer_grammar::TOKEN_COUNT>;
    using RegexMask = std::bitset<lexer_grammar::TOKEN_COUNT>;

    using Token = std::pair<size_t, std::string_view>;

    const RegexArray& buildRegexes();

    ByteCodeItem convert(const Token& token);
}
