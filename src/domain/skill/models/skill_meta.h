#ifndef SKILL_META_H
#define SKILL_META_H

#include <string>
#include <vector>
#include "skill_id.h"
#include "skill_param.h"

namespace domain {
namespace skill {

/**
 * @brief Skill 元数据（从 skill.json 解析而来）
 */
struct SkillMeta {
    SkillId                  id;
    std::string              name;
    std::string              version;
    std::string              description;
    std::string              author;
    std::vector<SkillParamDef> params;
    std::string              scriptPath;   ///< 脚本路径（Python/JS等）或 "builtin"
};

} // namespace skill
} // namespace domain

#endif // SKILL_META_H
