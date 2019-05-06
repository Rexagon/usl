#pragma once

#include <vector>
#include <variant>
#include <functional>

#include "ByteCode.hpp"

namespace app
{
    class CommandBuffer
    {
        using Task = std::function<void(CommandBuffer&)>;

        struct PointerRequest { size_t index; };
        struct PointerReply { size_t index; };

        using Command = std::variant<Task, PointerRequest, PointerReply, ByteCodeItem>;

    public:
        std::vector<ByteCodeItem> generate();

        void translate(Task task);
        void requestPosition(size_t index);
        void replyPosition(size_t index);
        void push(const ByteCodeItem& item);

        size_t createPositionIndex();

    private:
        std::vector<Command> m_commands;
        size_t m_currentPointerIndex = 0;
    };
}