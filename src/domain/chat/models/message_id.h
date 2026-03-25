#ifndef MESSAGE_ID_H
#define MESSAGE_ID_H

#include <string>

namespace domain {
namespace chat {

/**
 * @brief 消息ID值对象
 */
struct MessageId {
    std::string value;

    MessageId() : value("") {}
    explicit MessageId(const std::string& val) : value(val) {}

    bool operator==(const MessageId& other) const {
        return value == other.value;
    }

    bool operator!=(const MessageId& other) const {
        return !(*this == other);
    }

    bool operator<(const MessageId& other) const {
        return value < other.value;
    }
};

} // namespace chat
} // namespace domain

#endif // MESSAGE_ID_H
