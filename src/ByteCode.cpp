#include "ByteCode.hpp"

const char* app::opcode::getString(size_t code)
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
