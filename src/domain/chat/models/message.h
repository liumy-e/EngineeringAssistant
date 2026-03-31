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
    const std::string& getContent() const { return m_content; }  // 引用返回，零拷贝
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
