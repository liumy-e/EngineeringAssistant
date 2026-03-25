#ifndef EXECUTE_RESULT_H
#define EXECUTE_RESULT_H

#include <string>

namespace domain {
namespace skill {

/**
 * @brief Skill 执行结果
 */
struct ExecuteResult {
    bool        success  = false;
    std::string output;     ///< 执行输出（Markdown 文本）
    std::string error;      ///< 错误信息

    static ExecuteResult ok(const std::string& output) {
        ExecuteResult r;
        r.success = true;
        r.output  = output;
        return r;
    }

    static ExecuteResult fail(const std::string& error) {
        ExecuteResult r;
        r.success = false;
        r.error   = error;
        return r;
    }
};

} // namespace skill
} // namespace domain

#endif // EXECUTE_RESULT_H
