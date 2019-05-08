#include "CommandBuffer.hpp"

#include <cassert>
#include <numeric>
#include <unordered_map>

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

    std::unordered_map<size_t, size_t> replies;

    size_t position = 0;
    for (auto it = m_commands.begin(); it != m_commands.end();) {
        std::visit([this, &it, &replies, &position](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, PointerReply>) {
                replies.try_emplace(arg.index, position);

                it = m_commands.erase(it);
            }
            else if constexpr (!std::is_same_v<T, Task> && !std::is_same_v<T, SyntaxNode*>) {
                ++it;
                ++position;
            }
            else {
                throw std::runtime_error("Bad grammar");
            }
        }, *it);
    }

    std::vector<app::ByteCodeItem> result;
    result.reserve(m_commands.size());

    for (const auto& command : m_commands) {
        std::visit([&result, &replies](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, PointerRequest>) {
                const auto it = replies.find(arg.index);
                if (it == replies.end()) {
                    throw std::runtime_error("Bad pointers grammar");
                }

                result.emplace_back(Pointer{ it->second });
            }
            else if constexpr (std::is_same_v<T, ByteCodeItem>) {
                result.emplace_back(arg);
            }
            else {
                throw std::runtime_error("Bad grammar");
            }

            printf("[%3zu] ", result.size() - 1);
            print(result.back());
            printf("\n");
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

void app::CommandBuffer::pushLoopBounds(size_t startPointerIndex, size_t endPointerIndex)
{
    m_loopBounds.emplace(startPointerIndex, endPointerIndex);
}

size_t app::CommandBuffer::getLoopStartPointerIndex() const {
    assert(!m_loopBounds.empty());
    return m_loopBounds.top().first;
}

size_t app::CommandBuffer::getLoopEndPointerIndex() const {
    assert(!m_loopBounds.empty());
    return m_loopBounds.top().second;
}

void app::CommandBuffer::popLoopBounds()
{
    assert(!m_loopBounds.empty());
    m_loopBounds.pop();
}

size_t app::CommandBuffer::createPositionIndex() {
    return m_currentPointerIndex++;
}
