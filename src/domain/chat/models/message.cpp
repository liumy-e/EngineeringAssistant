#include "message.h"
#include <QUuid>

namespace domain {
namespace chat {

Message::Message()
    : m_type(MessageType::User)
    , m_timestamp(QDateTime::currentDateTime())
{
    m_id = MessageId(QUuid::createUuid().toString().toStdString());
}

Message::~Message() {
}

void Message::setContent(const std::string& content) {
    m_content = content;
}

void Message::setType(MessageType type) {
    m_type = type;
}

void Message::setTimestamp(const QDateTime& timestamp) {
    m_timestamp = timestamp;
}

} // namespace chat
} // namespace domain
