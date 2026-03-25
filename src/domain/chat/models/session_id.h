#ifndef SESSION_ID_H
#define SESSION_ID_H

#include <string>

namespace domain {
namespace chat {

/**
 * @brief 会话ID值对象
 */
struct SessionId {
    std::string value;

    SessionId() : value("") {}
    explicit SessionId(const std::string& val) : value(val) {}

    bool operator==(const SessionId& other) const {
        return value == other.value;
    }

    bool operator!=(const SessionId& other) const {
        return !(*this == other);
    }

    bool operator<(const SessionId& other) const {
        return value < other.value;
    }
};

} // namespace chat
} // namespace domain

#endif // SESSION_ID_H
