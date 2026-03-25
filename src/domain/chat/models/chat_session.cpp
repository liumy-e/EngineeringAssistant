#include "chat_session.h"
#include <algorithm>

namespace domain {
namespace chat {

ChatSession::ChatSession(const SessionId& id)
    : m_id(id)
    , m_createdAt(QDateTime::currentDateTime())
{
}

ChatSession::~ChatSession() {
}

void ChatSession::addMessage(const Message& msg) {
    QMutexLocker locker(&m_mutex);
    m_messages.push_back(msg);
}

std::vector<Message> ChatSession::getRecentMessages(int count) const {
    QMutexLocker locker(&m_mutex);

    std::vector<Message> result;
    int startIndex = std::max(0, (int)m_messages.size() - count);

    for (int i = startIndex; i < (int)m_messages.size(); ++i) {
        result.push_back(m_messages[i]);
    }

    return result;
}

void ChatSession::clearHistory() {
    QMutexLocker locker(&m_mutex);
    m_messages.clear();
}

std::vector<Message> ChatSession::getAllMessages() const {
    QMutexLocker locker(&m_mutex);
    return m_messages;
}

} // namespace chat
} // namespace domain
