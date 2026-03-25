#ifndef SKILL_EXECUTOR_H
#define SKILL_EXECUTOR_H

#include <memory>
#include "../models/skill_registry.h"
#include "../models/execute_result.h"
#include "../models/skill_param.h"
#include "../models/skill_id.h"

namespace domain {
namespace skill {

/**
 * @brief Skill 执行器领域服务
 *
 * 解析 @skill 指令，从注册表中找到对应 Skill，
 * 传入参数并返回执行结果。
 */
class SkillExecutor {
public:
    explicit SkillExecutor(std::shared_ptr<SkillRegistry> registry);
    ~SkillExecutor() = default;

    /**
     * @brief 执行指定 Skill
     * @param skillId   Skill ID
     * @param params    参数键值对
     * @return 执行结果
     */
    ExecuteResult execute(const SkillId& skillId,
                          const SkillParams& params);

    /**
     * @brief 从用户文本中解析 @skill 指令
     *        格式: @skill_id key1=value1 key2=value2
     * @param text 用户输入文本
     * @param outId     解析出的 Skill ID
     * @param outParams 解析出的参数
     * @return 是否解析成功
     */
    static bool parseCommand(const std::string& text,
                             SkillId& outId,
                             SkillParams& outParams);

private:
    std::shared_ptr<SkillRegistry> m_registry;
};

} // namespace skill
} // namespace domain

#endif // SKILL_EXECUTOR_H
