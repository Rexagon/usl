#pragma once

#include <string_view>

namespace app
{
	class Position final
	{
	public:
		Position() = default;
		Position(const char* position, const char* end);

		bool hasMore() const;
		size_t remaining() const;

		const char& operator*() const;

		Position& operator++();
		Position& operator--();

		Position operator+(size_t distance) const;
		Position operator-(size_t distance) const;

		Position& operator+=(size_t distance);
		Position& operator-=(size_t distance);

		bool operator<(const Position& other);

		bool operator==(const Position& other) const;
		bool operator!=(const Position& other) const;

		static std::string_view toString(const Position& begin, const Position& end);

	private:
		const char* m_position = nullptr;
		const char* m_end = nullptr;

		int m_line = -1;
		int m_column = -1;
		int m_lastColumn = -1;
	};
}
