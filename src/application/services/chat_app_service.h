#ifndef CHAT_APP_SERVICE_H
#define CHAT_APP_SERVICE_H

#include <string>
#include <memory>
#include <functional>
#include "../../domain/chat/models/message.h"
#include "../../domain/chat/models/chat_session.h"
#include "../../domain/chat/services/chat_processor.h"
#include "../../domain/knowledge/models/knowledge_base.h"
#include "../../domain/skill/services/skill_executor.h"
#include "../../domain/rendering/services/markdown_renderer.h"

namespace application {

/**
 * @brief 聊天应用服务
 *
 * 协调 ChatContext、KnowledgeContext、SkillContext，
 * 处理一次完整的"用户输入 -> 助手回复"流程。
 */
class ChatAppService {
public:
    /**
     * @brief 回复回调函数类型
     *        参数: 渲染后的 HTML 字符串
     */
    using ReplyCallback = std::function<void(const std::string& html)>;

    ChatAppService(
        std::shared_ptr<domain::chat::ChatSession>         session,
        std::shared_ptr<domain::chat::ChatProcessor>       processor,
        std::shared_ptr<domain::skill::SkillExecutor>      skillExecutor,
        std::shared_ptr<domain::rendering::IMarkdownRenderer> renderer
    );
    ~ChatAppService() = default;

    /**
     * @brief 处理用户输入
     * @param userText  用户输入的原始文本
     * @param callback  回复就绪时的回调（传入渲染后的 HTML）
     */
    void handleUserInput(const std::string& userText,
                         const ReplyCallback& callback);

    /**
     * @brief 获取当前会话
     */
    std::shared_ptr<domain::chat::ChatSession> session() const;

    /**
     * @brief 绑定知识库，用于检索增强
     */
    void attachKnowledgeBase(std::shared_ptr<domain::knowledge::KnowledgeBase> kb);

    /**
     * @brief 清空当前会话
     */
    void clearSession();

private:
    std::string buildContextPrompt(const std::string& query) const;

    std::shared_ptr<domain::chat::ChatSession>            m_session;
    std::shared_ptr<domain::chat::ChatProcessor>          m_processor;
    std::shared_ptr<domain::skill::SkillExecutor>         m_skillExecutor;
    std::shared_ptr<domain::rendering::IMarkdownRenderer> m_renderer;
    std::shared_ptr<domain::knowledge::KnowledgeBase>     m_knowledgeBase;
};

} // namespace application

#endif // CHAT_APP_SERVICE_H
