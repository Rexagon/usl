#pragma once

#include <memory>
#include <stdexcept>
#include <unordered_map>

#include "ByteCode.hpp"

namespace app
{
    class CoreObject;
    class CoreFunction;

    using CoreObjectPtr = std::shared_ptr<CoreObject>;
    using CoreFunctionPtr = std::shared_ptr<CoreFunction>;

    struct ScriptFunction final
    {
        size_t address;
    };

    class Symbol final
    {
    public:
        using DataVariant = std::variant<
            std::nullopt_t,
            bool,
            double,
            std::string,
            ScriptFunction,
            CoreObjectPtr,
            CoreFunctionPtr,
            Symbol*>;

        enum class Type
        {
            Null,
            Bool,
            Number,
            String,
            ScriptFunction,

            CoreObject,
            CoreFunction,

            Reference
        };

        enum class ValueCategory {
            Lvalue,
            Rvalue
        };

        explicit Symbol(ValueCategory category);
        Symbol(std::nullopt_t, ValueCategory category);
        Symbol(bool value, ValueCategory category);
        Symbol(double value, ValueCategory category);
        Symbol(const std::string& value, ValueCategory category);
        Symbol(const ScriptFunction& value, ValueCategory category);
        Symbol(CoreObject* value, ValueCategory category);
        Symbol(CoreFunction* value, ValueCategory category);
        Symbol(const Symbol& symbol, ValueCategory category);

        explicit Symbol(Symbol* symbol);

        Symbol deref() const;

        template<typename T>
        void assign(const T& value)
        {
            std::visit([this, &value](auto && arg) {
                using D = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<D, Symbol*>) {
                    arg->assign(value);
                }
                else if constexpr (details::is_any_of_v<T, std::nullopt_t, bool, double, std::string>) {
                    m_data = value;
                    if constexpr (std::is_same_v<T, std::nullopt_t>) {
                        m_type = Type::Null;
                    }
                    else if constexpr (std::is_same_v<T, bool>) {
                        m_type = Type::Bool;
                    }
                    else if constexpr (std::is_same_v<T, double>) {
                        m_type = Type::Number;
                    }
                    else if constexpr (std::is_same_v<T, std::string>) {
                        m_type = Type::String;
                    }
                }
                else if constexpr (std::is_same_v<T, Symbol>) {
                    m_data = value.m_data;
                    m_type = value.m_type;
                }
                else {
                    throw std::runtime_error("Bad assign");
                }
            }, m_data);
        }

        Symbol operationUnary(OpCode op) const;
        Symbol operationBinaryMath(const Symbol& symbol, OpCode op) const;
        Symbol operationLogic(const Symbol& symbol, OpCode op) const;
        Symbol operationCompare(const Symbol& symbol, OpCode op) const;

        template<typename Callable>
        void visit(Callable&& f) const
        {
            std::visit(f, m_data);
        }

        void print() const;

        Type getType() const;

        void setValueCategory(ValueCategory category);
        ValueCategory getValueCategory() const;

    protected:
        Type m_type;
        DataVariant m_data;

        ValueCategory m_valueCategory;
    };

    class CoreObject
    {
    public:
        virtual ~CoreObject() = default;

        Symbol getMember(std::string_view name) const;

    private:
        std::unordered_map<std::string_view, Symbol> m_members;
    };

    class CoreFunction
    {
    public:
        virtual ~CoreFunction() = default;

    protected:

    };
}
