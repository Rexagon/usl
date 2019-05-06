#include "ByteCode.hpp"

const char* app::opcode::toString(size_t code)
{
    switch (code) {
    case DECL:
        return "DECL";
    case ASSIGN:
        return "ASSIGN";
    case DEREF:
        return "DEREF";
    case POP:
        return "POP";
    case NOT:
        return "NOT";
    case UNM:
        return "UNM";
    case ADD:
        return "ADD";
    case SUB:
        return "SUB";
    case MUL:
        return "MUL";
    case DIV:
        return "DIV";
    case AND:
        return "AND";
    case OR:
        return "OR";
    case EQ:
        return "EQ";
    case NEQ:
        return "NEQ";
    case LT:
        return "LT";
    case LE:
        return "LE";
    case GT:
        return "GT";
    case GE:
        return "GE";
	case IF:
		return "IF";
    case JMP:
        return "JMP";
    case CALL:
        return "CALL";
    case RET:
        return "RET";
    case DEFBLOCK:
        return "DEFBLOCK";
    case DELBLOCK:
        return "DELBLOCK";
    default:
        return "Unknown";
    }
}

void app::print(const app::StackItem& item)
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
    }, item);
}
