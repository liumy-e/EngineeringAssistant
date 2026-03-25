#ifndef CHAT_SESSION_H
#define CHAT_SESSION_H

#include <vector>
#include <QMutex>
#include "session_id.h"
#include "message.h"

namespace domain {
namespace chat {

/**
 * @brief 聊天会话聚合根
 */
class ChatSession {
public:
    explicit ChatSession(const SessionId& id);
    ~ChatSession();

    // 业务方法
    void addMessage(const Message& msg);
    std::vector<Message> getRecentMessages(int count) const;
    void clearHistory();

    // Getter
    SessionId getId() const { return m_id; }
    QDateTime getCreatedAt() const { return m_createdAt; }
    size_t getMessageCount() const { return m_messages.size(); }
    std::vector<Message> getAllMessages() const;

private:
    mutable QMutex m_mutex;
    SessionId m_id;
    QDateTime m_createdAt;
    std::vector<Message> m_messages;
};

} // namespace chat
} // namespace domain

#endif // CHAT_SESSION_H
