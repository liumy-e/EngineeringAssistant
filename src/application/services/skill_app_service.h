#ifndef SKILL_APP_SERVICE_H
#define SKILL_APP_SERVICE_H

#include <string>
#include <vector>
#include <memory>
#include "../../domain/skill/models/skill_registry.h"
#include "../../domain/skill/models/skill_meta.h"

namespace infrastructure { namespace skill { class SkillLoader; } }

namespace application {

/**
 * @brief Skill 应用服务
 *
 * 提供 Skill 的扫描、加载、注销、列表查询操作。
 */
class SkillAppService {
public:
    explicit SkillAppService(const std::string& skillDir,
                             std::shared_ptr<domain::skill::SkillRegistry> registry);
    ~SkillAppService();

    /**
     * @brief 扫描并加载指定目录中的所有 Skill
     * @return 成功加载的数量
     */
    int loadSkillsFromDir();

    /**
     * @brief 加载单个 Skill（通过目录路径）
     * @param skillDirPath  Skill 的目录路径
     * @return 是否加载成功
     */
    bool loadSkill(const std::string& skillDirPath);

    /**
     * @brief 获取所有已注册 Skill 的元数据列表
     */
    std::vector<domain::skill::SkillMeta> listSkills() const;

    /**
     * @brief 注销一个 Skill
     */
    void unloadSkill(const std::string& skillId);

    /**
     * @brief 注册表（供其他服务使用）
     */
    std::shared_ptr<domain::skill::SkillRegistry> registry() const;

private:
    std::string m_skillDir;
    std::shared_ptr<domain::skill::SkillRegistry> m_registry;
    std::unique_ptr<infrastructure::skill::SkillLoader> m_loader;
};

} // namespace application

#endif // SKILL_APP_SERVICE_H
