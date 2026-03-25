#ifndef CHAT_SESSION_REPOSITORY_H
#define CHAT_SESSION_REPOSITORY_H

#include <vector>
#include <memory>
#include "session_id.h"
#include "chat_session.h"

namespace domain {
namespace chat {

/**
 * @brief 聊天会话仓储接口
 */
class ChatSessionRepository {
public:
    virtual ~ChatSessionRepository() = default;

    /**
     * @brief 保存会话
     * @param session 会话对象
     */
    virtual void save(const ChatSession& session) = 0;

    /**
     * @brief 加载会话
     * @param id 会话ID
     * @return 会话对象指针，如果不存在返回nullptr
     */
    virtual std::shared_ptr<ChatSession> load(const SessionId& id) = 0;

    /**
     * @brief 列出所有会话ID
     * @return 会话ID列表
     */
    virtual std::vector<SessionId> listAllSessions() = 0;

    /**
     * @brief 删除会话
     * @param id 会话ID
     */
    virtual void remove(const SessionId& id) = 0;
};

} // namespace chat
} // namespace domain

#endif // CHAT_SESSION_REPOSITORY_H
