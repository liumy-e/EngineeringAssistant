#ifndef SKILL_PARAM_H
#define SKILL_PARAM_H

#include <string>
#include <map>

namespace domain {
namespace skill {

/**
 * @brief Skill 参数描述（元信息）
 */
struct SkillParamDef {
    std::string name;
    std::string type;           ///< "string" / "int" / "bool"
    std::string description;
    bool        required = true;
    std::string defaultValue;
};

/**
 * @brief Skill 执行时传入的参数值 key->value
 */
using SkillParams = std::map<std::string, std::string>;

} // namespace skill
} // namespace domain

#endif // SKILL_PARAM_H
