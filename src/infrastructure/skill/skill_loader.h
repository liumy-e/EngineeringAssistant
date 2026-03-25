#ifndef SKILL_LOADER_H
#define SKILL_LOADER_H

#include <string>
#include <memory>
#include <vector>
#include "../../domain/skill/interfaces/skill_interface.h"
#include "../../domain/skill/models/skill_meta.h"
#include "../../domain/skill/models/skill_registry.h"

namespace infrastructure {
namespace skill {

/**
 * @brief Skill 加载器（基础设施层）
 *
 * 负责：
 *  1. 扫描 Skill 目录（每个子目录一个 Skill）
 *  2. 解析 skill.json 元数据
 *  3. 创建内置 Skill 实例（脚本式扩展留扩展点）
 *  4. 向 SkillRegistry 注册
 */
class SkillLoader {
public:
    SkillLoader() = default;
    ~SkillLoader() = default;

    /**
     * @brief 扫描目录，加载所有 Skill 到注册表
     * @param dir       Skill 根目录
     * @param registry  目标注册表
     * @return 成功加载的数量
     */
    int loadFromDir(const std::string& dir,
                    domain::skill::SkillRegistry& registry);

    /**
     * @brief 加载单个 Skill 目录
     * @param skillDir  单个 Skill 目录路径
     * @param registry  目标注册表
     * @return 是否成功
     */
    bool loadOne(const std::string& skillDir,
                 domain::skill::SkillRegistry& registry);

private:
    domain::skill::SkillMeta parseMeta(const std::string& jsonPath);
    std::shared_ptr<domain::skill::ISkill>
        createSkill(const domain::skill::SkillMeta& meta,
                    const std::string& skillDir);
};

} // namespace skill
} // namespace infrastructure

#endif // SKILL_LOADER_H
