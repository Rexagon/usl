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
        Symbol(const CoreObjectPtr& value, ValueCategory category);
        Symbol(const CoreFunctionPtr& value, ValueCategory category);
        Symbol(const Symbol& symbol, ValueCategory category);

        explicit Symbol(Symbol* symbol);

        Symbol& unref();
        const Symbol& unref() const;

        template<typename T>
        void assign(T&& value)
        {
            if (m_valueCategory != ValueCategory::Lvalue) {
                throw std::runtime_error{ "Unable to assign value to rvalue 123" };
            }

            auto& symbol = unref();

            using D = std::decay_t<T>;

            if constexpr (std::is_same_v<D, Symbol*>) {
                symbol.assign(value->unref());
            }
            else if constexpr (details::is_any_of_v<D,
                std::nullopt_t,
                bool,
                double,
                std::string,
                ScriptFunction,
                CoreObjectPtr,
                CoreFunctionPtr>)
            {
                symbol.m_data = value;
                if constexpr (std::is_same_v<D, std::nullopt_t>) {
                    symbol.m_type = Type::Null;
                }
                else if constexpr (std::is_same_v<D, bool>) {
                    symbol.m_type = Type::Bool;
                }
                else if constexpr (std::is_same_v<D, double>) {
                    symbol.m_type = Type::Number;
                }
                else if constexpr (std::is_same_v<D, std::string>) {
                    symbol.m_type = Type::String;
                }
                else if constexpr (std::is_same_v<D, ScriptFunction>) {
                    symbol.m_type = Type::ScriptFunction;
                }
                else if constexpr (std::is_same_v<D, CoreObjectPtr>) {
                    symbol.m_type = Type::CoreObject;
                }
                else if constexpr (std::is_same_v<D, CoreFunctionPtr>) {
                    symbol.m_type = Type::CoreFunction;
                }
            }
            else if constexpr (std::is_same_v<D, Symbol>) {
                auto& unreferencedValue = value.unref();
                symbol.m_data = unreferencedValue.m_data;
                symbol.m_type = unreferencedValue.m_type;
            }
            else {
                throw std::runtime_error{ "Bad assign argument" };
            }
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

        DataVariant& getData();

    protected:
        Type m_type;
        DataVariant m_data;

        ValueCategory m_valueCategory;
    };
}
