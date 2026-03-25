#include "skill_executor.h"
#include <sstream>
#include <algorithm>

namespace domain {
namespace skill {

SkillExecutor::SkillExecutor(std::shared_ptr<SkillRegistry> registry)
    : m_registry(std::move(registry))
{}

ExecuteResult SkillExecutor::execute(const SkillId& skillId,
                                     const SkillParams& params)
{
    auto skill = m_registry->getSkill(skillId);    if (!skill) {
        return ExecuteResult::fail("未找到 Skill: " + skillId.value);
    }
    return skill->execute(params);
}

bool SkillExecutor::parseCommand(const std::string& text,
                                 SkillId& outId,
                                 SkillParams& outParams)
{
    // 必须以 @开头
    if (text.empty() || text[0] != '@') return false;

    std::istringstream iss(text.substr(1));
    std::string token;

    // 第一个 token 是 skill id
    if (!(iss >> token)) return false;
    outId = SkillId(token);

    // 后续 token 是 key=value
    while (iss >> token) {
        auto pos = token.find('=');
        if (pos == std::string::npos) {
            // 单独的 key，视为 key=""
            outParams[token] = "";
        } else {
            std::string key   = token.substr(0, pos);
            std::string value = token.substr(pos + 1);
            outParams[key]    = value;
        }
    }
    return true;
}

} // namespace skill
} // namespace domain
