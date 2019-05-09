#pragma once

#include "Symbol.hpp"

namespace app
{
    class CoreObject
    {
    public:
        virtual Symbol getMember(std::string_view name);

    protected:
        template<typename T>
        T& get(const std::string_view name)
        {
            return std::get<T>(m_members[name].getData());
        }

        template<typename T>
        bool checkType(const std::string_view name)
        {
            return std::holds_alternative<T>(m_members[name].getData());
        }

        template<typename T>
        void registerMember(std::string_view name, T&& data)
        {
            m_members.try_emplace(name, data, Symbol::ValueCategory::Lvalue);
        }

        virtual ~CoreObject() = default;

    private:
        std::unordered_map<std::string_view, Symbol> m_members;
    };
}
