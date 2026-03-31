#ifndef SKILL_REGISTRY_H
#define SKILL_REGISTRY_H

#include <map>
#include <vector>
#include <memory>
#include <string>
#include <optional>
#include "../interfaces/skill_interface.h"
#include "../models/skill_id.h"
#include "../models/skill_meta.h"

namespace domain {
namespace skill {

/**
 * @brief Skill 注册表（聚合根）
 *
 * 负责维护所有已注册 Skill 的索引，提供注册、注销和查询功能。
 */
class SkillRegistry {
public:
    SkillRegistry()  = default;
    ~SkillRegistry() = default;

    // 禁止拷贝，允许移动
    SkillRegistry(const SkillRegistry&)            = delete;
    SkillRegistry& operator=(const SkillRegistry&) = delete;

    /**
     * @brief 注册一个 Skill
     * @param skill Skill 实例（所有权移交注册表）
     * @return 是否注册成功（若 id 已存在则返回 false）
     */
    bool registerSkill(std::shared_ptr<ISkill> skill);

    /**
     * @brief 注销一个 Skill
     * @param id Skill ID
     */
    void unregisterSkill(const SkillId& id);

    /**
     * @brief 获取 Skill
     * @param id Skill ID
     * @return Skill 指针，不存在则返回 nullptr
     */
    std::shared_ptr<ISkill> getSkill(const SkillId& id) const;

    /**
     * @brief 获取所有已注册 Skill 的元数据列表
     */
    std::vector<SkillMeta> getAllMeta() const;

    /**
     * @brief 按 ID 查询单个 Skill 元数据（O(log N)，无需全量拷贝列表）
     * @return 找到则返回 SkillMeta，否则返回 std::nullopt
     */
    std::optional<SkillMeta> getMeta(const SkillId& id) const;

    /**
     * @brief 检查某 Skill 是否已注册
     */
    bool contains(const SkillId& id) const;

    /**
     * @brief 已注册 Skill 数量
     */
    size_t size() const;

private:
    std::map<std::string, std::shared_ptr<ISkill>> m_skills;
};

} // namespace skill
} // namespace domain

#endif // SKILL_REGISTRY_H
