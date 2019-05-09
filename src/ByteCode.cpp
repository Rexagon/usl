#include "ByteCode.hpp"

std::string app::toString(const OpCode code)
{
    switch (code) {
    case OpCode::DECLVAR:
        return "DECLVAR";
    case OpCode::DECLFUN:
        return "DECLFUN";
    case OpCode::ASSIGN:
        return "ASSIGN";
    case OpCode::ASSIGNREF:
        return "ASSIGNREF";
    case OpCode::DEREF:
        return "DEREF";
    case OpCode::POP:
        return "POP";
    case OpCode::NOT:
        return "NOT";
    case OpCode::UNM:
        return "UNM";
    case OpCode::ADD:
        return "ADD";
    case OpCode::SUB:
        return "SUB";
    case OpCode::MUL:
        return "MUL";
    case OpCode::DIV:
        return "DIV";
    case OpCode::AND:
        return "AND";
    case OpCode::OR:
        return "OR";
    case OpCode::EQ:
        return "EQ";
    case OpCode::NEQ:
        return "NEQ";
    case OpCode::LT:
        return "LT";
    case OpCode::LE:
        return "LE";
    case OpCode::GT:
        return "GT";
    case OpCode::GE:
        return "GE";
	case OpCode::IF:
		return "IF";
    case OpCode::JMP:
        return "JMP";
    case OpCode::CALL:
        return "CALL";
    case OpCode::RET:
        return "RET";
    case OpCode::PUSHARG:
        return "PUSHARG";
    case OpCode::POPARG:
        return "POPARG";
    case OpCode::DEFBLOCK:
        return "DEFBLOCK";
    case OpCode::DELBLOCK:
        return "DELBLOCK";
    default:
        return "Unknown";
    }
}

void app::print(const ByteCodeItem & item)
{
    std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, std::nullopt_t>) {
            printf("null");
        }
        else if constexpr (std::is_same_v<T, bool>) {
            printf("bool: %s", arg ? "true" : "false");
        }
        else if constexpr (std::is_same_v<T, double>) {
            printf("number: %f", arg);
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            printf("string: '%s'", arg.c_str());
        }
        else if constexpr (std::is_same_v<T, std::string_view>) {
            printf("var: %s", std::string(arg).c_str());
        }
        else if constexpr (std::is_same_v<T, OpCode>) {
            printf("op: %s", toString(arg).c_str());
        }
        else if constexpr (std::is_same_v<T, Pointer >) {
            printf("ptr: %zu", arg);
        }
    }, item);
}
