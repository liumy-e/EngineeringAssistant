#ifndef MESSAGE_TYPE_H
#define MESSAGE_TYPE_H

namespace domain {
namespace chat {

/**
 * @brief 消息类型枚举
 */
enum class MessageType {
    User,       ///< 用户消息
    Assistant,  ///< 助手消息
    System      ///< 系统消息
};

} // namespace chat
} // namespace domain

#endif // MESSAGE_TYPE_H
