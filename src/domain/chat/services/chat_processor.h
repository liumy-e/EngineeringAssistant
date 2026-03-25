#ifndef CHAT_PROCESSOR_H
#define CHAT_PROCESSOR_H

#include <string>
#include "message.h"

namespace domain {
namespace chat {

/**
 * @brief 对话处理器领域服务
 */
class ChatProcessor {
public:
    ChatProcessor();
    ~ChatProcessor();

    /**
     * @brief 处理用户消息
     * @param userMessage 用户消息
     * @return 助手回复消息
     */
    Message processUserMessage(const Message& userMessage);

private:
    std::string generateResponse(const std::string& query);
};

} // namespace chat
} // namespace domain

#endif // CHAT_PROCESSOR_H
