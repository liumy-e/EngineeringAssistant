#include "chat_app_service.h"
#include "../../domain/chat/models/message_type.h"
#include "../../domain/skill/models/skill_id.h"
#include "../../domain/skill/models/skill_param.h"

namespace application {

ChatAppService::ChatAppService(
    std::shared_ptr<domain::chat::ChatSession>            session,
    std::shared_ptr<domain::chat::ChatProcessor>          processor,
    std::shared_ptr<domain::skill::SkillExecutor>         skillExecutor,
    std::shared_ptr<domain::rendering::IMarkdownRenderer> renderer
)
    : m_session(std::move(session))
    , m_processor(std::move(processor))
    , m_skillExecutor(std::move(skillExecutor))
    , m_renderer(std::move(renderer))
{}

void ChatAppService::handleUserInput(const std::string& userText,
                                     const ReplyCallback& callback)
{
    if (!callback) return;

    // 1. 构建用户消息并加入会话
    domain::chat::Message userMsg;
    userMsg.setContent(userText);
    userMsg.setType(domain::chat::MessageType::User);
    userMsg.setTimestamp(QDateTime::currentDateTime());
    m_session->addMessage(userMsg);

    // 2. 检查是否是 @skill 指令
    domain::skill::SkillId  skillId;
    domain::skill::SkillParams skillParams;
    if (domain::skill::SkillExecutor::parseCommand(userText, skillId, skillParams)) {
        // 执行 Skill
        auto execResult = m_skillExecutor->execute(skillId, skillParams);
        std::string markdown = execResult.success
                               ? execResult.output
                               : "**执行错误**: " + execResult.error;
        auto renderResult = m_renderer->render(markdown);
        callback(renderResult.success ? renderResult.html : markdown);
        return;
    }

    // 3. 知识库检索增强（RAG）
    std::string query = userText;
    if (m_knowledgeBase) {
        auto docs = m_knowledgeBase->searchDocuments(userText);
        if (!docs.empty()) {
            // 把最相关的前 3 篇文档摘要拼入 prompt（直接复用已有搜索结果，不再二次搜索）
            query = buildContextPrompt(userText, docs);
        }
    }

    // 4. 用 ChatProcessor 生成回复
    domain::chat::Message queryMsg;
    queryMsg.setContent(query);
    queryMsg.setType(domain::chat::MessageType::User);

    auto replyMsg = m_processor->processUserMessage(queryMsg);
    m_session->addMessage(replyMsg);

    // 5. 渲染 Markdown -> HTML
    auto renderResult = m_renderer->render(replyMsg.getContent());
    callback(renderResult.success ? renderResult.html : replyMsg.getContent());
}

std::shared_ptr<domain::chat::ChatSession> ChatAppService::session() const
{
    return m_session;
}

void ChatAppService::attachKnowledgeBase(
        std::shared_ptr<domain::knowledge::KnowledgeBase> kb)
{
    m_knowledgeBase = std::move(kb);
}

void ChatAppService::clearSession()
{
    m_session->clearHistory();
}

// ----------------------------------------------------------------
// 检索增强：把已搜索到的文档片段拼到查询前面（无重复搜索）
// ----------------------------------------------------------------
std::string ChatAppService::buildContextPrompt(
        const std::string& query,
        const std::vector<domain::knowledge::Document>& docs) const
{
    if (docs.empty()) return query;

    std::string context = "【知识库参考】\n\n";
    int count = 0;
    for (const auto& doc : docs) {
        if (count++ >= 3) break;
        context += "**" + doc.getTitle() + "**\n";
        // 取前 300 字（引用引用返回后 getContent() 是零拷贝）
        const auto& content = doc.getContent();
        if (content.size() > 300)
            context += content.substr(0, 300) + "...\n\n";
        else
            context += content + "\n\n";
    }
    context += "---\n\n**用户问题：** " + query;
    return context;
}

} // namespace application
