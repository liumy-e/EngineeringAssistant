#ifndef SKILL_STATUS_H
#define SKILL_STATUS_H

namespace domain {
namespace skill {

enum class SkillStatus {
    Active,     ///< 已激活
    Inactive,   ///< 未激活
    Error       ///< 错误状态
};

} // namespace skill
} // namespace domain

#endif // SKILL_STATUS_H
