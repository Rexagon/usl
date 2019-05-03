#pragma once

#include "ByteCode.hpp"

namespace app
{
	class CoreObject;
	class CoreFunction;

	struct ScriptFunction final
	{
		size_t address;
		uint8_t argc;
	};

	class Symbol final
	{
		using DataVariant = std::variant<
			std::nullopt_t,
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

		void assign(bool value);
		void assign(double value);
		void assign(const std::string& value);
		void assign(Symbol* symbol);

		void* data();
		const void* data() const;

		Type getType() const;

		bool canBeDereferenced() const;

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
