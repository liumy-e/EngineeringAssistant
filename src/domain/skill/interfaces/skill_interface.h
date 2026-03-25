#ifndef SKILL_INTERFACE_H
#define SKILL_INTERFACE_H

#include <string>
#include <vector>
#include "../models/skill_meta.h"
#include "../models/skill_param.h"
#include "../models/execute_result.h"

namespace domain {
namespace skill {

/**
 * @brief Skill 插件接口（所有 Skill 必须继承此接口）
 *
 * 内置 Skill 直接继承并在代码中实现；
 * 外部 Skill 通过脚本执行器适配到此接口。
 */
class ISkill {
public:
    virtual ~ISkill() = default;

    virtual const SkillMeta& meta() const = 0;

    /**
     * @brief 执行 Skill
     * @param params 执行参数
     * @return 执行结果
     */
    virtual ExecuteResult execute(const SkillParams& params) = 0;
};

} // namespace skill
} // namespace domain

#endif // SKILL_INTERFACE_H
