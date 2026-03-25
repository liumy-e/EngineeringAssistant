#include "chat_processor.h"
#include <algorithm>
#include <cctype>

namespace domain {
namespace chat {

ChatProcessor::ChatProcessor() {
}

ChatProcessor::~ChatProcessor() {
}

Message ChatProcessor::processUserMessage(const Message& userMessage) {
    Message assistantMessage;

    assistantMessage.setType(MessageType::Assistant);
    assistantMessage.setContent(generateResponse(userMessage.getContent()));
    assistantMessage.setTimestamp(QDateTime::currentDateTime());

    return assistantMessage;
}

std::string ChatProcessor::generateResponse(const std::string& query) {
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(),
                  lowerQuery.begin(), ::tolower);

    // 简单的关键词匹配
    if (lowerQuery.find("你好") != std::string::npos ||
        lowerQuery.find("hello") != std::string::npos) {
        return "你好！我是工程图纸设计助手，有什么我可以帮你的吗？";
    }

    if (lowerQuery.find("图纸") != std::string::npos ||
        lowerQuery.find("标准") != std::string::npos) {
        return "关于图纸设计，我可以帮你查询相关标准、规范和最佳实践。例如：\n"
               "\n"
               "• GB/T 14689 - 技术制图 图纸幅面和格式\n"
               "• GB/T 4457 - 机械制图标准\n"
               "• 图纸审查流程\n"
               "\n"
               "你想了解哪个方面？";
    }

    if (lowerQuery.find("尺寸") != std::string::npos) {
        return "关于尺寸标注，有以下要点：\n"
               "\n"
               "1. 尺寸标注应完整、清晰、准确\n"
               "2. 尺寸基准选择要合理\n"
               "3. 避免标注重叠\n"
               "4. 公差标注要正确\n"
               "5. 标注位置要便于读图\n"
               "\n"
               "如果你有具体的图纸尺寸标注问题，可以详细描述。";
    }

    if (lowerQuery.find("审查") != std::string::npos ||
        lowerQuery.find("检查") != std::string::npos) {
        return "图纸审查流程包括以下步骤：\n"
               "\n"
               "**1. 审查前准备**\n"
               "- 收集完整资料\n"
               "- 确定审查范围\n"
               "\n"
               "**2. 视图审查**\n"
               "- 检查视图完整性\n"
               "- 评估视图表达清晰度\n"
               "\n"
               "**3. 尺寸标注审查**\n"
               "- 检查标注完整性\n"
               "- 验证标注准确性\n"
               "\n"
               "**4. 技术要求审查**\n"
               "- 检查技术要求明确性\n"
               "- 验证标注正确性\n"
               "\n"
               "**5. 工艺性审查**\n"
               "- 评估结构工艺性\n"
               "- 检查加工可行性\n"
               "\n"
               "你可以询问具体的审查问题。";
    }

    if (lowerQuery.find("help") != std::string::npos ||
        lowerQuery.find("帮助") != std::string::npos) {
        return "我可以帮你：\n"
               "\n"
               "• **查询设计标准**: 图纸幅面、格式、标注规范等\n"
               "• **了解审查流程**: 图纸审查的步骤和方法\n"
               "• **获取设计建议**: 常见设计问题的解决方案\n"
               "• **工艺性分析**: 结构工艺性评估\n"
               "\n"
               "请告诉我你需要什么帮助！";
    }

    // 默认回复
    return "我理解你的问题：" + query + "\n\n"
           "作为一个工程图纸设计助手，我可以帮你查询设计标准、规范和最佳实践。"
           "请提供更多细节，我会尽力帮助你。\n\n"
           "你也可以尝试问：\n"
           "• 图纸标准有哪些？\n"
           "• 如何进行图纸审查？\n"
           "• 尺寸标注要注意什么？";
}

} // namespace chat
} // namespace domain
