#ifndef RENDER_RESULT_H
#define RENDER_RESULT_H

#include <string>

namespace domain {
namespace rendering {

/**
 * @brief Markdown 渲染结果（值对象）
 */
struct RenderResult {
    bool        success = false;
    std::string html;       ///< 渲染后的 HTML
    std::string error;

    static RenderResult ok(const std::string& html) {
        RenderResult r;
        r.success = true;
        r.html    = html;
        return r;
    }

    static RenderResult fail(const std::string& err) {
        RenderResult r;
        r.success = false;
        r.error   = err;
        return r;
    }
};

} // namespace rendering
} // namespace domain

#endif // RENDER_RESULT_H
