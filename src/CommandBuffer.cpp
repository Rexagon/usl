#include "CommandBuffer.hpp"

#include <cassert>
#include <unordered_map>

#include "Rules.hpp"

std::vector<app::ByteCodeItem> app::CommandBuffer::generate()
{
    for (auto it = m_commands.begin(); it != m_commands.end();) {
        std::visit([this, &it](auto && arg) {
            using T = std::decay_t<decltype(arg)>;

            constexpr auto isTask = std::is_same_v<T, Task>;
            constexpr auto isNode = std::is_same_v<T, SyntaxNode*>;

            if constexpr (std::is_same_v<T, ByteCodeItem>) {
                std::visit([this](auto && arg) {
                    using T = std::decay_t<decltype(arg)>;

                    if constexpr (std::is_same_v<T, OpCode>) {
                        if (arg == OpCode::DEFBLOCK) {
                            ++m_currentBlock;
                        }
                        else if (arg == OpCode::DELBLOCK) {
                            if (m_currentBlock == 0) {
                                throw std::runtime_error{ "DELBLOCK error" };
                            }
                            --m_currentBlock;
                        }
                    }
                }, arg);
            }

            if constexpr (isTask || isNode) {
                m_localIterator = it;
                ++m_localIterator;

                m_applyLocalIterator = false;
                if constexpr (isTask) {
                    arg(*this);
                }
                else {
                    arg->translate(*this);
                }

                it = m_commands.erase(it);
                if (m_applyLocalIterator) {
                    it = m_localIterator;
                }
            }
            else {
                ++it;
            }
        }, *it);
    }

    std::unordered_map<size_t, size_t> replies;

    size_t position = 0;
    for (auto it = m_commands.begin(); it != m_commands.end();) {
        std::visit([this, &it, &replies, &position](auto && arg) {
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

    std::vector<ByteCodeItem> result;
    result.reserve(m_commands.size());

    for (const auto& command : m_commands) {
        std::visit([&result, &replies](auto && arg) {
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
        }, command);
    }

    return result;
}

void app::CommandBuffer::translate(Task task)
{
    m_commands.insert(m_localIterator, task);
}

void app::CommandBuffer::translate(SyntaxNode& task)
{
    m_commands.insert(m_localIterator, &task);
}

void app::CommandBuffer::requestPosition(const size_t index)
{
    m_commands.insert(m_localIterator, PointerRequest{ index });
}

void app::CommandBuffer::replyPosition(const size_t index)
{
    m_commands.insert(m_localIterator, PointerReply{ index });
}

void app::CommandBuffer::push(const ByteCodeItem& item)
{
    m_commands.insert(m_localIterator, item);
}

void app::CommandBuffer::pushLoopBounds(size_t startPointerIndex, size_t endPointerIndex)
{
    m_loopBounds.emplace(startPointerIndex, endPointerIndex);
    m_scopeBlocks.emplace(m_currentBlock);
}

size_t app::CommandBuffer::getLoopStartPointerIndex() const
{
    assert(!m_loopBounds.empty());
    return m_loopBounds.top().first;
}

size_t app::CommandBuffer::getLoopEndPointerIndex() const
{
    assert(!m_loopBounds.empty());
    return m_loopBounds.top().second;
}

void app::CommandBuffer::popLoopBounds()
{
    assert(!m_loopBounds.empty());
    m_loopBounds.pop();
    m_scopeBlocks.pop();
}

void app::CommandBuffer::clearBlocks()
{
    assert(!m_scopeBounds.empty());

    auto block = m_currentBlock;
    while (block > m_scopeBlocks.top()) {
        m_commands.insert(m_localIterator, OpCode::DELBLOCK);
        --block;
    }

    m_applyLocalIterator = true;
}

size_t app::CommandBuffer::createPositionIndex() {
    return m_currentPointerIndex++;
}
