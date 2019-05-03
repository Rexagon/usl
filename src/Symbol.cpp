#include "Symbol.hpp"

app::Symbol::Symbol() :
	m_type(Type::Null), m_data(std::nullopt)
{
}

app::Symbol::Symbol(const bool value) :
	m_type(Type::Bool), m_data(value)
{
}

app::Symbol::Symbol(const double value) :
	m_type(Type::Number), m_data(value)
{
}

app::Symbol::Symbol(const std::string& value) :
	m_type(Type::String), m_data(value)
{
}

app::Symbol::Symbol(const ScriptFunction& value) :
	m_type(Type::ScriptFunction), m_data(value)
{
}

app::Symbol::Symbol(CoreObject* value) :
	m_type(Type::CoreObject), m_data(value)
{
}

app::Symbol::Symbol(CoreFunction* value) :
	m_type(Type::CoreFunction), m_data(value)
{
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

void app::Symbol::assign(Symbol* symbol)
{
	if (symbol == nullptr || symbol->m_type == Type::Null) {
		m_data = std::nullopt;
		m_type = Type::Null;
		return;
	}

	m_data = symbol->m_data;
	m_type = symbol->m_type;
}

void* app::Symbol::data()
{
	if (m_type == Type::Null) {
		return nullptr;
	}

	void* result = nullptr;
	std::visit([&result](auto && arg) {
		using T = std::decay_t<decltype(arg)>;

		if constexpr (std::is_pointer_v<T>) {
			result = reinterpret_cast<void*>(arg);
		}
		else {
			result = reinterpret_cast<void*>(&arg);
		}
	}, m_data);

	return result;
}

const void* app::Symbol::data() const {
    if (m_type == Type::Null) {
        return nullptr;
    }

    const void* result = nullptr;
    std::visit([&result](auto && arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_pointer_v<T>) {
            result = reinterpret_cast<const void*>(arg);
        }
        else {
            result = reinterpret_cast<const void*>(&arg);
        }
    }, m_data);

    return result;
}

app::Symbol::Type app::Symbol::getType() const
{
	return m_type;
}

bool app::Symbol::canBeDereferenced() const {
    switch (m_type) {
    case Symbol::Type::Null:
    case Symbol::Type::Bool:
    case Symbol::Type::Number:
    case Symbol::Type::String:
        return true;
    default:
        return false;
    }
}
