#pragma once

#include "Symbol.hpp"

namespace app
{
    class CoreObject
    {
    public:
        virtual Symbol getMember(const std::string& name);

    protected:
        template<typename T>
        T& get(const std::string& name)
        {
            return std::get<T>(m_members[name].getData());
        }

        template<typename T>
        bool checkType(const std::string& name)
        {
            return std::holds_alternative<T>(m_members[name].getData());
        }

        template<typename T>
        void registerMember(const std::string& name, T&& data)
        {
            m_members.try_emplace(name, data, Symbol::ValueCategory::Lvalue);
        }

        virtual ~CoreObject() = default;

    private:
        std::unordered_map<std::string, Symbol> m_members;
    };
}
