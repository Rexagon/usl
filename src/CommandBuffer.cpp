#include "CommandBuffer.hpp"

#include <cassert>
#include <numeric>

#include "Rules.hpp"

std::vector<app::ByteCodeItem> app::CommandBuffer::generate()
{
    for (auto it = m_commands.begin(); it != m_commands.end();) {
        std::visit([this, &it](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            constexpr auto isTask = std::is_same_v<T, Task>;
            constexpr auto isNode = std::is_same_v<T, SyntaxNode*>;

            if constexpr (isTask || isNode) {
                m_currentPosition = it;
                m_currentPosition++;

                if constexpr (isTask) {
                    arg(*this);
                }
                else {
                    arg->translate(*this);
                }

                it = m_commands.erase(it);
            }
            else {
                ++it;
            }
        }, *it);
    }

    std::vector<app::ByteCodeItem> result;
    result.reserve(m_commands.size());

    for (const auto& command : m_commands) {
        std::visit([&result](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, PointerRequest>) {
                printf("ptr request\n");
            }
            else if constexpr (std::is_same_v<T, PointerReply>) {
                printf("ptr reply\n");
            }
            else if constexpr (std::is_same_v<T, ByteCodeItem>) {
                print(arg);
                printf("\n");

                result.emplace_back(arg);
            }
            else if constexpr (std::is_same_v<T, SyntaxNode*>) {
                printf("Node\n");
            }
            else {
                printf("task\n");
            }
        }, command);
    }

    return result;
}

void app::CommandBuffer::translate(Task task)
{
    m_commands.insert(m_currentPosition, task);
}

void app::CommandBuffer::translate(app::SyntaxNode& task)
{
    m_commands.insert(m_currentPosition, &task);
}

void app::CommandBuffer::requestPosition(size_t index)
{
    m_commands.insert(m_currentPosition, PointerRequest{ index });
}

void app::CommandBuffer::replyPosition(size_t index)
{
    m_commands.insert(m_currentPosition, PointerReply{ index });
}

void app::CommandBuffer::push(const ByteCodeItem& item)
{
    m_commands.insert(m_currentPosition, item);
}

size_t app::CommandBuffer::createPositionIndex() {
    return m_currentPointerIndex++;
}
