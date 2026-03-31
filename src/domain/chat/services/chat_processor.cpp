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

    // ─── 若 prompt 包含 RAG 知识库上下文，直接组织成回答 ───────────────────
    const std::string kbTag = "【知识库参考】";
    const std::string userTag = "**用户问题：** ";
    if (query.find(kbTag) != std::string::npos) {
        // 提取用户实际问题
        std::string userQ = query;
        auto upos = query.rfind(userTag);
        if (upos != std::string::npos)
            userQ = query.substr(upos + userTag.size());

        // 提取知识库内容（kbTag 到 "---" 分隔线之间）
        std::string kbContent;
        auto kbStart = query.find(kbTag);
        auto kbEnd   = query.rfind("---\n\n");
        if (kbStart != std::string::npos && kbEnd != std::string::npos && kbEnd > kbStart)
            kbContent = query.substr(kbStart + kbTag.size(), kbEnd - kbStart - kbTag.size());

        std::string resp;
        resp += "根据知识库检索结果，为你整理如下：\n\n";
        if (!kbContent.empty()) {
            resp += kbContent;
            resp += "\n";
        }
        resp += "---\n\n";
        resp += "**你的问题**：" + userQ + "\n\n";
        resp += "以上是从知识库中找到的相关内容。";
        resp += "如需了解更多细节，可以进一步追问，或在「知识库」页签中搜索关键词。";
        return resp;
    }

    // ─── 本地关键词匹配兜底（无知识库时仍能给基础回答）─────────────────────
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(),
                   lowerQuery.begin(), ::tolower);

    if (lowerQuery.find("你好") != std::string::npos ||
        lowerQuery.find("hello") != std::string::npos ||
        lowerQuery.find("hi") != std::string::npos) {
        return "你好！我是工程图纸设计助手 👋\n\n"
               "我可以帮你：\n"
               "- 查询 GB/T 图纸设计标准\n"
               "- 解答尺寸标注、视图表达问题\n"
               "- 生成图纸审查报告\n"
               "- 通过「知识库」功能检索你导入的工程文档\n\n"
               "> 💡 **小提示**：先在左侧「📚 知识库」页签导入文档并加载，"
               "回来问我时我会自动结合知识库内容作答。";
    }

    if (lowerQuery.find("图纸") != std::string::npos ||
        lowerQuery.find("标准") != std::string::npos ||
        lowerQuery.find("gb") != std::string::npos) {
        return "关于图纸设计标准，常用规范如下：\n\n"
               "| 标准编号 | 名称 |\n"
               "|----------|------|\n"
               "| GB/T 14689 | 技术制图 · 图纸幅面和格式 |\n"
               "| GB/T 4457  | 机械制图 |\n"
               "| GB/T 17451 | 技术制图 · 图样画法 · 视图 |\n"
               "| GB/T 16675 | 技术制图 · 简化表示法 |\n\n"
               "你想了解哪个方面？可以继续追问。\n\n"
               "> 也可以在「知识库」页签导入相关标准文档，我会结合内容给出更准确的答案。";
    }

    if (lowerQuery.find("尺寸") != std::string::npos ||
        lowerQuery.find("标注") != std::string::npos) {
        return "尺寸标注要点：\n\n"
               "1. **完整性** — 所有加工面均需标注，不遗漏\n"
               "2. **基准选取** — 选择加工基准面，避免封闭尺寸链\n"
               "3. **清晰性** — 尺寸线不交叉，数字方向一致\n"
               "4. **公差标注** — 配合面注明尺寸公差和形位公差\n"
               "5. **表面粗糙度** — 按 GB/T 1031 正确标注 Ra 值\n\n"
               "有具体标注问题请继续描述，我来帮你分析。";
    }

    if (lowerQuery.find("审查") != std::string::npos ||
        lowerQuery.find("检查") != std::string::npos ||
        lowerQuery.find("审核") != std::string::npos) {
        return "## 图纸审查流程\n\n"
               "**第一步：资料核查**\n"
               "- 标题栏完整（图号、比例、材料、版次）\n"
               "- 明细栏与图纸一致\n\n"
               "**第二步：视图审查**\n"
               "- 视图数量满足表达需要\n"
               "- 主视图选择合理\n"
               "- 剖视、断面标注规范\n\n"
               "**第三步：尺寸审查**\n"
               "- 尺寸标注完整，无多余/缺漏\n"
               "- 公差与技术要求匹配\n\n"
               "**第四步：技术要求**\n"
               "- 材料、热处理、表面处理说明清晰\n"
               "- 关键尺寸公差等级合理\n\n"
               "**第五步：工艺性**\n"
               "- 结构可加工、可装配\n"
               "- 检验方便可行\n\n"
               "> 可使用 `@drawing-review part=零件名` 调用自动审查功能";
    }

    if (lowerQuery.find("帮助") != std::string::npos ||
        lowerQuery.find("help") != std::string::npos ||
        lowerQuery.find("功能") != std::string::npos) {
        return "## 工程图纸设计助手 · 功能说明\n\n"
               "### 💬 对话功能\n"
               "直接输入问题，我会回答图纸设计、标准查询等问题。\n\n"
               "### 📚 知识库\n"
               "- **导入文档**：导入 md/txt 等文本文件\n"
               "- **导入网址**：抓取网页内容加入知识库\n"
               "- **加载后问答**：加载知识库后，回答会自动结合内容\n"
               "- **切片查看**：查看文档分块详情\n"
               "- **知识图谱**：可视化关键词共现关系\n\n"
               "### 🔧 技能（Skill）\n"
               "在对话中使用 `@skill_id` 调用：\n"
               "- `@drawing-dimension-check drawing=图纸名`\n"
               "- `@standard-query standard=GB/T14689`\n"
               "- `@drawing-review part=零件名`";
    }

    // 默认回复：给出有用引导而不是废话模板
    return "我收到了你的问题：**" + query + "**\n\n"
           "目前我的本地知识主要涵盖工程图纸设计领域。\n\n"
           "建议：\n"
           "1. 在「📚 知识库」页签导入相关文档，加载后我可以结合内容作答\n"
           "2. 尝试更具体的问法，如 [GB/T14689 图幅尺寸是多少]\n"
           "3. 使用 `@standard-query` 技能查询具体标准\n\n"
           "> 💡 输入「帮助」查看完整功能列表";
}

} // namespace chat
} // namespace domain
