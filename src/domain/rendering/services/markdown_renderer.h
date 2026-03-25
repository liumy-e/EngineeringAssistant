#ifndef MARKDOWN_RENDERER_H
#define MARKDOWN_RENDERER_H

#include <string>
#include "../models/render_result.h"

namespace domain {
namespace rendering {

/**
 * @brief Markdown 渲染接口
 *
 * 领域层只定义接口，基础设施层提供具体实现。
 * 当前提供一个纯 C++11 的简易实现（不依赖第三方库），
 * 后续可替换为 cmark-gfm 版本。
 */
class IMarkdownRenderer {
public:
    virtual ~IMarkdownRenderer() = default;
    virtual RenderResult render(const std::string& markdown) = 0;
};

} // namespace rendering
} // namespace domain

#endif // MARKDOWN_RENDERER_H
