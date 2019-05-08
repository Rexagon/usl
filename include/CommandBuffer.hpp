#pragma once

#include <list>
#include <stack>
#include <vector>
#include <variant>
#include <functional>

#include "ByteCode.hpp"

namespace app
{
    struct SyntaxNode;

    class CommandBuffer
    {
        using Task = std::function<void(CommandBuffer&)>;

        struct PointerRequest { size_t index; };
        struct PointerReply { size_t index; };

        using Command = std::variant<Task, SyntaxNode*, PointerRequest, PointerReply, ByteCodeItem>;

    public:
        std::vector<ByteCodeItem> generate();

        void translate(Task task);
        void translate(SyntaxNode& task);
        void requestPosition(size_t index);
        void replyPosition(size_t index);
        void push(const ByteCodeItem& item);

        void pushLoopBounds(size_t startPointerIndex, size_t endPointerIndex);
        size_t getLoopStartPointerIndex() const;
        size_t getLoopEndPointerIndex() const;
        void popLoopBounds();

        size_t createPositionIndex();

    private:
        std::list<Command> m_commands;
        size_t m_currentPointerIndex = 0;
        std::stack<std::pair<size_t, size_t>> m_loopBounds;

        std::list<Command>::iterator m_currentPosition = m_commands.end();
    };
}
