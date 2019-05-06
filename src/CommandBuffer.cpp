#include "CommandBuffer.hpp"

#include <cassert>
#include <numeric>

std::vector<app::ByteCodeItem> app::CommandBuffer::generate()
{
    for (const auto& command : m_commands) {
        std::visit([](auto&& arg) {
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
            }
            else {
                printf("task\n");
            }
        }, command);
    }

    return {};
}

void app::CommandBuffer::translate(Task task)
{
    m_commands.emplace_back(task);
}

void app::CommandBuffer::requestPosition(size_t index)
{
    m_commands.emplace_back(PointerRequest{ index });
}

void app::CommandBuffer::replyPosition(size_t index)
{
    m_commands.emplace_back(PointerReply{ index });
}

void app::CommandBuffer::push(const ByteCodeItem& item)
{
    m_commands.emplace_back(item);
}

size_t app::CommandBuffer::createPositionIndex() {
    return m_currentPointerIndex++;
}
