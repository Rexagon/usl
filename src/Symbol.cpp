#include "Symbol.hpp"

#include <stack>
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
        else if constexpr (std::is_same_v<T, app::CoreObjectPtr>) {
            return "[CoreObject]";
        }
        else if constexpr (std::is_same_v<T, app::CoreFunctionPtr>) {
            return "[CoreFunction]";
        }
        else if constexpr (std::is_same_v<T, app::Symbol*>) {
            std::string result{ "[ref] " };
            arg->visit([&result](auto && arg) {
                result += details::toString(arg);
            });
            return result;
        }
        else {
            return "Unknown";
        }
    }
}

app::Symbol::Symbol(const ValueCategory category) :
    m_type(Type::Null), m_data(std::nullopt), m_valueCategory(category)
{
}

app::Symbol::Symbol(std::nullopt_t, const ValueCategory category) :
    m_type(Type::Null), m_data(std::nullopt), m_valueCategory(category)
{
}

app::Symbol::Symbol(const bool value, const ValueCategory category) :
    m_type(Type::Bool), m_data(value), m_valueCategory(category)
{
}

app::Symbol::Symbol(const double value, const ValueCategory category) :
    m_type(Type::Number), m_data(value), m_valueCategory(category)
{
}

app::Symbol::Symbol(const std::string& value, const ValueCategory category) :
    m_type(Type::String), m_data(value), m_valueCategory(category)
{
}

app::Symbol::Symbol(const ScriptFunction& value, const ValueCategory category) :
    m_type(Type::ScriptFunction), m_data(value), m_valueCategory(category)
{
}

app::Symbol::Symbol(const CoreObjectPtr& value, const ValueCategory category) :
    m_type(Type::CoreObject), m_data(value), m_valueCategory(category)
{
}

app::Symbol::Symbol(const CoreFunctionPtr& value, const ValueCategory category) :
    m_type(Type::CoreFunction), m_data(value), m_valueCategory(category)
{
}

app::Symbol::Symbol(const Symbol& symbol, const ValueCategory category) :
    m_type(symbol.m_type), m_data(symbol.m_data), m_valueCategory(category)
{
}

app::Symbol::Symbol(Symbol* symbol) :
    m_type(Type::Reference), m_data(symbol), m_valueCategory(ValueCategory::Lvalue)
{
    if (symbol->getType() == Type::Reference) {
        m_data = std::get<Symbol*>(symbol->m_data);
    }
}

app::Symbol app::Symbol::deref() const
{
    if (m_valueCategory == ValueCategory::Rvalue) {
        return *this;
    }

    if (m_type == Type::Reference) {
        auto* symbol = std::get<Symbol*>(m_data);
        return symbol->deref();
    }

    auto result = *this;
    result.setValueCategory(ValueCategory::Rvalue);
    return result;
}

app::Symbol app::Symbol::operationUnary(const OpCode op) const
{
    assert(isUnaryMathOp(op));

    Symbol result{ ValueCategory::Rvalue };
    std::visit([&result, op](auto && arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, std::nullopt_t>) {
            if (op == OpCode::NEQ) {
                result.assign(true);
                return;
            }
        }
        if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, double>) {
            switch (op) {
            case OpCode::NOT:
                result.assign(!static_cast<bool>(arg));
                return;
            case OpCode::UNM:
                result.assign(-static_cast<double>(arg));
                return;
            default:
                return;
            }
        }

        throw std::runtime_error{ "Wrong " + toString(op) + " argument type" };
    }, m_data);

    return result;
}

app::Symbol app::Symbol::operationBinaryMath(const Symbol& symbol, const OpCode op) const
{
    assert(isBinaryMathOp(op));

    Symbol result{ ValueCategory::Rvalue };
    std::visit([&result, op](auto && argLeft, auto && argRight) {
        using Tl = std::decay_t<decltype(argLeft)>;
        using Tr = std::decay_t<decltype(argRight)>;

        if constexpr (std::is_same_v<Tl, std::string> || std::is_same_v<Tr, std::string>) {
            if (op == OpCode::ADD) {
                result.assign(details::toString(argLeft) + details::toString(argRight));
                return;
            }
        }
        if constexpr (details::is_any_of_v<Tl, bool, double> && details::is_any_of_v<Tr, bool, double>) {
            switch (op) {
            case OpCode::ADD:
                result.assign(static_cast<double>(argLeft) + static_cast<double>(argRight));
                return;
            case OpCode::SUB:
                result.assign(static_cast<double>(argLeft) - static_cast<double>(argRight));
                return;
            case OpCode::MUL:
                result.assign(static_cast<double>(argLeft) * static_cast<double>(argRight));
                return;
            case OpCode::DIV:
                result.assign(static_cast<double>(argLeft) / static_cast<double>(argRight));
                return;
            default:
                return;
            }
        }

        throw std::runtime_error{ "Wrong " + toString(op) + " argument types" };
    }, m_data, symbol.m_data);

    return result;
}

app::Symbol app::Symbol::operationLogic(const Symbol& symbol, const OpCode op) const 
{
    assert(isLogicOp(op));

    Symbol result{ ValueCategory::Rvalue };
    std::visit([&result, op](auto && argLeft, auto && argRight) {
        using Tl = std::decay_t<decltype(argLeft)>;
        using Tr = std::decay_t<decltype(argRight)>;

        if constexpr (details::is_any_of_v<Tl, bool, double> && details::is_any_of_v<Tr, bool, double>) {
            switch (op) {
            case OpCode::AND:
                result.assign(argLeft && argRight);
                return;
            case OpCode::OR:
                result.assign(argLeft || argRight);
                return;
            default:
                return;
            }
        }

        throw std::runtime_error{ "Wrong " + toString(op) + " argument types" };
    }, m_data, symbol.m_data);

    return result;
}

app::Symbol app::Symbol::operationCompare(const Symbol& symbol, const OpCode op) const
{
    assert(isComparisonOp(op));

    Symbol result{ ValueCategory::Rvalue };
    std::visit([&result, op](auto && argLeft, auto && argRight) {
        using Tl = std::decay_t<decltype(argLeft)>;
        using Tr = std::decay_t<decltype(argRight)>;

        if constexpr (std::is_same_v<Tl, std::nullopt_t> && std::is_same_v<Tr, std::nullopt_t>) {
            switch (op) {
            case OpCode::EQ:
            case OpCode::LE:
            case OpCode::GE:
                result.assign(true);
                return;
            case OpCode::NEQ:
            case OpCode::LT:
            case OpCode::GT:
                result.assign(false);
                return;
            default:
                return;
            }
        }
        else if constexpr (std::is_same_v<Tl, std::nullopt_t> || std::is_same_v<Tr, std::nullopt_t>) {
            if (op == OpCode::EQ) {
                result.assign(false);
                return;
            }
            if (op == OpCode::NEQ) {
                result.assign(true);
                return;
            }
        }
        else if constexpr ((details::is_any_of_v<Tl, bool, double> && details::is_any_of_v<Tr, bool, double>) ||
            (std::is_same_v<Tl, std::string> && details::is_any_of_v<Tr, std::string>))
        {
            if constexpr (std::is_same_v<Tl, bool> && std::is_same_v<Tr, bool>) {
                if (op == OpCode::EQ) {
                    result.assign(argLeft == argRight);
                    return;
                }
                if (op == OpCode::NEQ) {
                    result.assign(argLeft != argRight);
                    return;
                }
            }

            using LType = std::conditional_t<details::is_any_of_v<Tl, bool, double>, double, std::string>;
            using RType = std::conditional_t<details::is_any_of_v<Tr, bool, double>, double, std::string>;

            switch (op) {
            case OpCode::EQ:
                result.assign(static_cast<LType>(argLeft) == static_cast<RType>(argRight));
                return;
            case OpCode::NEQ:
                result.assign(static_cast<LType>(argLeft) != static_cast<RType>(argRight));
                return;
            case OpCode::LT:
                result.assign(static_cast<LType>(argLeft) < static_cast<RType>(argRight));
                return;
            case OpCode::LE:
                result.assign(static_cast<LType>(argLeft) <= static_cast<RType>(argRight));
                return;
            case OpCode::GT:
                result.assign(static_cast<LType>(argLeft) > static_cast<RType>(argRight));
                return;
            case OpCode::GE:
                result.assign(static_cast<LType>(argLeft) >= static_cast<RType>(argRight));
                return;
            default:
                return;
            }
        }

        throw std::runtime_error{ "Wrong " + toString(op) + " argument types" };
    }, m_data, symbol.m_data);

    return result;
}

void app::Symbol::print() const
{
    visit([](auto && arg) {
        using T = std::decay_t<decltype(arg)>;
        constexpr auto isString = std::is_same_v<T, std::string>;

        printf("%s", details::toString(arg).c_str());
    });
}

app::Symbol::Type app::Symbol::getType() const
{
    return m_type;
}

void app::Symbol::setValueCategory(const ValueCategory category)
{
    m_valueCategory = category;
}

app::Symbol::ValueCategory app::Symbol::getValueCategory() const {
    return m_valueCategory;
}

app::Symbol::DataVariant& app::Symbol::getData()
{
    return m_data;
}
