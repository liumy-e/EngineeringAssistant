#ifndef BUILTIN_SKILLS_H
#define BUILTIN_SKILLS_H

#include <memory>
#include <string>
#include "../../domain/skill/interfaces/skill_interface.h"
#include "../../domain/skill/models/skill_meta.h"

namespace infrastructure {
namespace skill {

/**
 * @brief 内置 Skill 工厂
 *
 * 根据 skill id 返回对应的内置 Skill 实例，
 * 未知 id 返回 nullptr（由脚本加载器接管）。
 */
class BuiltinSkillFactory {
public:
    static std::shared_ptr<domain::skill::ISkill>
        create(const std::string& id, const domain::skill::SkillMeta& meta);
};

// ----------------------------------------------------------------
// 示例内置 Skill 1：图纸尺寸检查
// ----------------------------------------------------------------
class DrawingDimensionCheckSkill : public domain::skill::ISkill {
public:
    explicit DrawingDimensionCheckSkill(const domain::skill::SkillMeta& meta);
    const domain::skill::SkillMeta& meta() const override { return m_meta; }
    domain::skill::ExecuteResult execute(const domain::skill::SkillParams& params) override;

private:
    domain::skill::SkillMeta m_meta;
};

// ----------------------------------------------------------------
// 示例内置 Skill 2：标准查询
// ----------------------------------------------------------------
class StandardQuerySkill : public domain::skill::ISkill {
public:
    explicit StandardQuerySkill(const domain::skill::SkillMeta& meta);
    const domain::skill::SkillMeta& meta() const override { return m_meta; }
    domain::skill::ExecuteResult execute(const domain::skill::SkillParams& params) override;

private:
    domain::skill::SkillMeta m_meta;
};

// ----------------------------------------------------------------
// 示例内置 Skill 3：图纸审查报告生成
// ----------------------------------------------------------------
class DrawingReviewSkill : public domain::skill::ISkill {
public:
    explicit DrawingReviewSkill(const domain::skill::SkillMeta& meta);
    const domain::skill::SkillMeta& meta() const override { return m_meta; }
    domain::skill::ExecuteResult execute(const domain::skill::SkillParams& params) override;

private:
    domain::skill::SkillMeta m_meta;
};

} // namespace skill
} // namespace infrastructure

#endif // BUILTIN_SKILLS_H
