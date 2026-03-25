#ifndef SKILL_ID_H
#define SKILL_ID_H

#include <string>

namespace domain {
namespace skill {

/**
 * @brief SkillID值对象
 */
struct SkillId {
    std::string value;

    SkillId() : value("") {}
    explicit SkillId(const std::string& val) : value(val) {}

    bool operator==(const SkillId& other) const {
        return value == other.value;
    }

    bool operator!=(const SkillId& other) const {
        return !(*this == other);
    }
};

} // namespace skill
} // namespace domain

#endif // SKILL_ID_H
