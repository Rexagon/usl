#include "Symbol.hpp"

#include <cassert>
#include <stdexcept>

namespace details
{
    template<typename Tr>
    std::string toString(Tr&& arg)
    {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, std::nullopt_t>) {
            return "Null";
        }
        if constexpr (std::is_same_v<T, bool>) {
            return arg ? "True" : "False";
        }
        else if constexpr (std::is_same_v<T, double>) {
            return std::to_string(arg);
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            return arg;
        }
        else if constexpr (std::is_same_v<T, app::ScriptFunction>) {
            return "[ScriptFunction]";
        }
        else if constexpr (std::is_same_v<T, app::CoreObject*>) {
            return "[CoreObject]";
        }
        else if constexpr (std::is_same_v<T, app::CoreFunction*>) {
            return "[CoreFunction]";
        }
        else {
            return "Unknown";
        }
    }
}

app::Symbol::Symbol(ValueCategory category) :
	m_type(Type::Null), m_data(std::nullopt), m_valueCategory(category)
{
}

app::Symbol::Symbol(std::nullopt_t, app::Symbol::ValueCategory category) :
    m_type(Type::Null), m_data(std::nullopt), m_valueCategory(category)
{
}

app::Symbol::Symbol(const bool value, ValueCategory category) :
	m_type(Type::Bool), m_data(value), m_valueCategory(category)
{
}

app::Symbol::Symbol(const double value, ValueCategory category) :
	m_type(Type::Number), m_data(value), m_valueCategory(category)
{
}

app::Symbol::Symbol(const std::string& value, ValueCategory category) :
	m_type(Type::String), m_data(value), m_valueCategory(category)
{
}

app::Symbol::Symbol(const ScriptFunction& value, ValueCategory category) :
	m_type(Type::ScriptFunction), m_data(value), m_valueCategory(category)
{
}

app::Symbol::Symbol(CoreObject* value, ValueCategory category) :
	m_type(Type::CoreObject), m_data(value), m_valueCategory(category)
{
}

app::Symbol::Symbol(CoreFunction* value, ValueCategory category) :
	m_type(Type::CoreFunction), m_data(value), m_valueCategory(category)
{
}

app::Symbol::Symbol(const Symbol& symbol, ValueCategory valueCategory) :
        m_data(symbol.m_data), m_type(symbol.m_type), m_valueCategory(valueCategory)
{
}

void app::Symbol::assign(std::nullopt_t)
{
    m_data = std::nullopt;
    m_type = Type::Null;
}


void app::Symbol::assign(bool value)
{
    m_data = value;
    m_type = Type ::Bool;
}

void app::Symbol::assign(double value)
{
    m_data = value;
    m_type = Type::Number;
}

void app::Symbol::assign(const std::string &value)
{
    m_data = value;
    m_type = Type::String;
}

void app::Symbol::assign(const Symbol& symbol)
{
	m_data = symbol.m_data;
	m_type = symbol.m_type;
}

app::Symbol app::Symbol::operationUnary(opcode::Code op) const
{
    Symbol result(ValueCategory::Rvalue);
    std::visit([this, &result, op](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, std::nullopt_t>) {
            if (op == opcode::NEQ) {
                result.assign(true);
                return;
            }
        }
        if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, double>) {
            switch (op) {
            case opcode::NOT:
                result.assign(!static_cast<bool>(arg));
                return;
            case opcode::UNM:
                result.assign(-static_cast<double>(arg));
                return;
            default:
                assert(0);
                return;
            }
        }

        throw std::runtime_error("Wrong UNM argument type");
    }, m_data);

    return result;
}

app::Symbol app::Symbol::operationLogic(const app::Symbol &symbol, app::opcode::Code op) const
{
    Symbol result(ValueCategory::Rvalue);
    std::visit([this, &result, op](auto&& argLeft, auto&& argRight) {
        using Tl = std::decay_t<decltype(argLeft)>;
        using Tr = std::decay_t<decltype(argRight)>;

        if constexpr (details::is_any_of_v<Tl, bool, double> && details::is_any_of_v<Tr, bool, double>) {
            switch (op) {
            case opcode::AND:
                result.assign(argLeft && argRight);
                return;
            case opcode::OR:
                result.assign(argLeft || argRight);
                return;
            default:
                assert(0);
                return;
            }
        }

        throw std::runtime_error("Wrong OR argument types");
    }, m_data, symbol.m_data);

    return result;
}

app::Symbol app::Symbol::operationCompare(const Symbol& symbol, opcode::Code op) const
{
    Symbol result(ValueCategory::Rvalue);
    std::visit([this, &result, op](auto&& argLeft, auto&& argRight) {
        using Tl = std::decay_t<decltype(argLeft)>;
        using Tr = std::decay_t<decltype(argRight)>;

        if constexpr (std::is_same_v<Tl, std::nullopt_t> && std::is_same_v<Tr, std::nullopt_t>) {
            result.assign(true);
            return;
        }
        else if constexpr ((details::is_any_of_v<Tl, bool, double> && details::is_any_of_v<Tr, bool, double>) ||
                           (std::is_same_v<Tl, std::string> && details::is_any_of_v<Tr, std::string>))
        {
            if constexpr (std::is_same_v<Tl, bool> && std::is_same_v<Tr, bool>) {
                if (op == opcode::EQ) {
                    result.assign(argLeft == argRight);
                    return;
                }
                else if (op == opcode::NEQ) {
                    result.assign(argLeft != argRight);
                    return;
                }
            }

            using LType = std::conditional_t<details::is_any_of_v<Tl, bool, double>, double, std::string>;
            using RType = std::conditional_t<details::is_any_of_v<Tr, bool, double>, double, std::string>;

            switch (op) {
            case opcode::EQ:
                result.assign(static_cast<LType>(argLeft) == static_cast<RType>(argRight));
                return;
            case opcode::NEQ:
                result.assign(static_cast<LType>(argLeft) != static_cast<RType>(argRight));
                return;
            case opcode::LT:
                result.assign(static_cast<LType>(argLeft) < static_cast<RType>(argRight));
                return;
            case opcode::LE:
                result.assign(static_cast<LType>(argLeft) <= static_cast<RType>(argRight));
                return;
            case opcode::GT:
                result.assign(static_cast<LType>(argLeft) > static_cast<RType>(argRight));
                return;
            case opcode::GE:
                result.assign(static_cast<LType>(argLeft) >= static_cast<RType>(argRight));
                return;
            default:
                assert(0);
                return;
            }
        }

        throw std::runtime_error("Wrong " + std::string(opcode::toString(op)) + " argument types");
    }, m_data, symbol.m_data);

    return result;
}

app::Symbol app::Symbol::operationBinaryMath(const app::Symbol &symbol, opcode::Code op) const
{
    Symbol result(ValueCategory::Rvalue);
    std::visit([this, &result, op](auto&& argLeft, auto&& argRight) {
        using Tl = std::decay_t<decltype(argLeft)>;
        using Tr = std::decay_t<decltype(argRight)>;

        if constexpr (std::is_same_v<Tl, std::string> || std::is_same_v<Tr, std::string>) {
            if (op == opcode::ADD) {
                result.assign(details::toString(argLeft) + details::toString(argRight));
                return;
            }
        }
        if constexpr (details::is_any_of_v<Tl, bool, double> && details::is_any_of_v<Tr, bool, double>) {
            switch (op) {
            case opcode::ADD:
                result.assign(static_cast<double>(argLeft) + static_cast<double>(argRight));
                return;
            case opcode::SUB:
                result.assign(static_cast<double>(argLeft) - static_cast<double>(argRight));
                return;
            case opcode::MUL:
                result.assign(static_cast<double>(argLeft) * static_cast<double>(argRight));
                return;
            case opcode::DIV:
                result.assign(static_cast<double>(argLeft) / static_cast<double>(argRight));
                return;
            default:
                assert(0);
                return;
            }
        }

        throw std::runtime_error("Wrong " + std::string(opcode::toString(op)) + " argument types");
    }, m_data, symbol.m_data);

    return result;
}

void app::Symbol::print() const
{
    visit([](auto&& arg) {
       printf("%s", details::toString(arg).c_str());
    });
}

app::Symbol::Type app::Symbol::getType() const
{
	return m_type;
}

void app::Symbol::setValueCategory(ValueCategory category)
{
    m_valueCategory = category;
}

app::Symbol::ValueCategory app::Symbol::getValueCategory() const {
    return m_valueCategory;
}
