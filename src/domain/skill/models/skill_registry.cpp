#include "skill_registry.h"

namespace domain {
namespace skill {

bool SkillRegistry::registerSkill(std::shared_ptr<ISkill> skill)
{
    if (!skill) return false;
    const std::string key = skill->meta().id.value;
    if (m_skills.count(key)) return false;
    m_skills[key] = std::move(skill);
    return true;
}

void SkillRegistry::unregisterSkill(const SkillId& id)
{
    m_skills.erase(id.value);
}

std::shared_ptr<ISkill> SkillRegistry::getSkill(const SkillId& id) const
{
    auto it = m_skills.find(id.value);
    return it != m_skills.end() ? it->second : nullptr;
}

std::vector<SkillMeta> SkillRegistry::getAllMeta() const
{
    std::vector<SkillMeta> result;
    result.reserve(m_skills.size());
    for (auto& kv : m_skills) {
        result.push_back(kv.second->meta());
    }
    return result;
}

std::optional<SkillMeta> SkillRegistry::getMeta(const SkillId& id) const
{
    auto it = m_skills.find(id.value);
    if (it != m_skills.end())
        return it->second->meta();
    return std::nullopt;
}

bool SkillRegistry::contains(const SkillId& id) const
{
    return m_skills.count(id.value) > 0;
}

size_t SkillRegistry::size() const
{
    return m_skills.size();
}

} // namespace skill
} // namespace domain
