#include "LexerGrammar.hpp"

#include <cassert>
#include <charconv>

using namespace app::lexer_grammar;

const app::RegexArray& app::buildRegexes()
{
    static RegexArray regexes{
        (KeywordLet,            std::regex{"^let"}),
        (KeywordIf,             std::regex{"^if"}),
        (KeywordElse,           std::regex{"^else"}),
        (KeywordWhile,          std::regex{"^while"}),
        (KeywordFor,            std::regex{"^for"}),
        (KeywordBreak,          std::regex{"^break"}),
        (KeywordContinue,       std::regex{"^continue"}),
        (KeywordFunction,       std::regex{"^function"}),
        (KeywordReturn,         std::regex{"^return"}),
        (KeywordRef,            std::regex{"^ref"}),

        (Null,                  std::regex{"^null"}),
        (Boolean,               std::regex{"^(?:true)|(?:false)"}),
        (Identifier,            std::regex{"^[a-zA-Z_]+"}),
        (String,                std::regex{"^\"(?:\\\\.|[^\"])*\"?"}),
        (Number,                std::regex{"^[0-9]+\\.?[0-9]*"}),

        (OperatorAssignment,    std::regex{"^="}),
        (OperatorOr,            std::regex{"^\\|\\|"}),
        (OperatorAnd,           std::regex{"^&&"}),
        (OperatorEq,            std::regex{"^=="}),
        (OperatorNeq,           std::regex{"^!="}),
        (OperatorLt,            std::regex{"^<"}),
        (OperatorLeq,           std::regex{"^<="}),
        (OperatorGt,            std::regex{"^>"}),
        (OperatorGeq,           std::regex{"^<="}),
        (OperatorPlus,          std::regex{"^\\+"}),
        (OperatorMinus,         std::regex{"^-"}),
        (OperatorMul,           std::regex{"^\\*"}),
        (OperatorDiv,           std::regex{"^/"}),
        (OperatorIncrement,     std::regex{"^\\+\\+"}),
        (OperatorDecrement,     std::regex{"^--"}),
        (OperatorNegate,        std::regex{"^!"}),

        (StructureReference,    std::regex{"^\\."}),

        (ParenthesisOpen,       std::regex{"^\\("}),
        (ParenthesisClose,      std::regex{"^\\)"}),
        (BraceOpen,             std::regex{"^\\{"}),
        (BraceClose,            std::regex{"^\\}"}),
        (BracketOpen,           std::regex{"^\\["}),
        (BracketClose,          std::regex{"^\\]"}),

        (Comma,                 std::regex{"^,"}),
        (Semicolon,             std::regex{"^;"}),

        (CommentSingleLine,     std::regex{"^//[^\\n]*(?:\\n)?"}),
        (CommentMultiLine,      std::regex{"^/\\*(?:[^\\*]/|\\*[^/]|[^\\*/])*.?.?"})
    };

    return regexes;
}

app::ByteCodeItem app::convert(const Token& token)
{
    assert(isValue(token.first));

    switch (token.first) {
    case Boolean:
        return token.second == "true";

    case Identifier:
        return token.second;

    case String:
    {
        const auto* begin = &*token.second.begin();
        auto size = token.second.size();

        if (*begin == '\"') {
            ++begin;
            --size;
        }

        if (size > 0 && *(begin + size - 1) == '\"') {
            --size;
        }

        return std::string(std::string_view{ begin, size });
    }

    case Number:
    {
        auto result = 0.0;
        const auto ret = std::from_chars(&*token.second.begin(), &*token.second.end(), result);
        return result;
    }

    case Null:
    default:
        return std::nullopt;
    }
}
