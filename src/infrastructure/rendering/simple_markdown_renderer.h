#ifndef SIMPLE_MARKDOWN_RENDERER_H
#define SIMPLE_MARKDOWN_RENDERER_H

#include "../../domain/rendering/services/markdown_renderer.h"

namespace infrastructure {
namespace rendering {

/**
 * @brief 简易 Markdown 渲染器（不依赖第三方库）
 *
 * 支持特性：
 *  - 标题 (# ~ ######)
 *  - 粗体 (**text**)、斜体 (*text*)
 *  - 行内代码 (`code`)
 *  - 代码块 (```...```)
 *  - 无序列表 (- / *)
 *  - 有序列表 (1. 2. 3.)
 *  - 分隔线 (---)
 *  - 链接 [text](url)
 *  - 段落
 */
class SimpleMarkdownRenderer : public domain::rendering::IMarkdownRenderer {
public:
    SimpleMarkdownRenderer()  = default;
    ~SimpleMarkdownRenderer() = default;

    domain::rendering::RenderResult render(const std::string& markdown) override;

private:
    // --- 行级处理 ---
    std::string processLine(const std::string& line);
    std::string processInline(const std::string& text);
    static std::string escapeHtml(const std::string& text);

    // --- 状态 ---
    bool m_inCodeBlock   = false;
    bool m_inUl          = false;
    bool m_inOl          = false;
    bool m_inParagraph   = false;

    void closeOpenBlocks(std::string& html);
};

} // namespace rendering
} // namespace infrastructure

#endif // SIMPLE_MARKDOWN_RENDERER_H
