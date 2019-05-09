#include "CoreObject.hpp"

app::Symbol app::CoreObject::getMember(const std::string_view name)
{
    auto it = m_members.find(name);
    if (it == m_members.end()) {
        throw std::runtime_error{ "Unable to find member " + std::string{name} };
    }

    return Symbol{ &it->second };
}
