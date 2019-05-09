#include "Position.hpp"

app::Position::Position(const char* position, const char* end) :
    m_position(position), m_end(end), m_line(1), m_column(1), m_lastColumn(1)
{
}

bool app::Position::hasMore() const
{
    return m_position != m_end;
}

size_t app::Position::remaining() const
{
    return static_cast<size_t>(m_end - m_position);
}

const char& app::Position::operator*() const
{
    if (m_position == m_end) {
        return ""[0];
    }

    return *m_position;
}

app::Position& app::Position::operator++()
{
    if (m_position != m_end) {
        if (*m_position == '\n') {
            ++m_line;
            m_lastColumn = m_column;
            m_column = 1;
        }
        else {
            ++m_column;
        }

        m_position++;
    }
    return *this;
}

app::Position& app::Position::operator--()
{
    --m_position;
    if (*m_position == '\n') {
        --m_line;
        m_column = m_lastColumn;
    }
    else {
        --m_column;
    }
    return *this;
}

app::Position app::Position::operator+(const size_t distance) const
{
    auto result(*this);
    for (size_t i = 0; i < distance; ++i) {
        ++result;
    }
    return result;
}

app::Position app::Position::operator-(const size_t distance) const
{
    auto result(*this);
    for (size_t i = 0; i < distance; ++i) {
        --result;
    }
    return result;
}

app::Position& app::Position::operator+=(const size_t distance)
{
    *this = *this + distance;
    return *this;
}

app::Position& app::Position::operator-=(const size_t distance)
{
    *this = *this - distance;
    return *this;
}

bool app::Position::operator<(const Position & other) const
{
    return m_position < other.m_position;
}

bool app::Position::operator==(const Position & other) const
{
    return m_position == other.m_position;
}

bool app::Position::operator!=(const Position & other) const
{
    return m_position != other.m_position;
}

std::string_view app::Position::toString(const Position & begin, const Position & end)
{
    if (begin.m_position == nullptr || end.m_position == nullptr) {
        return {};
    }

    return std::string_view(begin.m_position, std::distance(begin.m_position, end.m_position));
}
