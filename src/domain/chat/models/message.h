#ifndef MESSAGE_H
#define MESSAGE_H

#include <QDateTime>
#include "message_type.h"
#include "message_id.h"

namespace domain {
namespace chat {

/**
 * @brief 消息实体
 */
class Message {
public:
    Message();
    ~Message();

    // Getter
    MessageId getId() const { return m_id; }
    std::string getContent() const { return m_content; }
    MessageType getType() const { return m_type; }
    QDateTime getTimestamp() const { return m_timestamp; }

    // Setter
    void setContent(const std::string& content);
    void setType(MessageType type);
    void setTimestamp(const QDateTime& timestamp);

private:
    MessageId m_id;
    std::string m_content;
    MessageType m_type;
    QDateTime m_timestamp;
};

} // namespace chat
} // namespace domain

#endif // MESSAGE_H
