#pragma once

#include <string>
#include <variant>
#include <optional>

namespace app
{
	class CoreObject;
	class CoreFunction;

	struct Null final {};

	struct ScriptFunction final
	{
		size_t address;
		uint8_t argc;
	};

	class Symbol final
	{
		using DataVariant = std::variant<
			Null,
			bool, 
			double, 
			std::string, 
			ScriptFunction,
			CoreObject*,
			CoreFunction*>;

	public:
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

		Symbol();
		explicit Symbol(bool value);
		explicit Symbol(double value);
		explicit Symbol(const std::string& value);
		explicit Symbol(const ScriptFunction& value);
		explicit Symbol(CoreObject* value);
		explicit Symbol(CoreFunction* value);

		void assign(Symbol* symbol);

		void* data();

		Type getType() const;

	protected:
		Type m_type;
		DataVariant m_data;
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
	};
}
