#pragma once

#include "ByteCode.hpp"

namespace app
{
	class CoreObject;
	class CoreFunction;

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
                CoreObject*,
                CoreFunction*>;

		enum class Type
		{
			Null,
			Bool,
			Number,
			String,
			ScriptFunction,

			CoreObject,
			CoreFunction,
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

		void assign(std::nullopt_t);
		void assign(bool value);
		void assign(double value);
		void assign(const std::string& value);
		void assign(const Symbol& symbol);

		Symbol operationUnary(opcode::Code op) const;
        Symbol operationBinaryMath(const Symbol& symbol, opcode::Code op) const;
        Symbol operationLogic(const Symbol& symbol, opcode::Code op) const;
        Symbol operationCompare(const Symbol& symbol, opcode::Code op) const;

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
	};

	class CoreFunction
	{
	public:
		virtual ~CoreFunction() = default;

    protected:

	};
}
