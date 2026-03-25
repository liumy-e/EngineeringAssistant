#include "skill_app_service.h"
#include "../../infrastructure/skill/skill_loader.h"

namespace application {

SkillAppService::SkillAppService(
        const std::string& skillDir,
        std::shared_ptr<domain::skill::SkillRegistry> registry)
    : m_skillDir(skillDir)
    , m_registry(std::move(registry))
    , m_loader(std::make_unique<infrastructure::skill::SkillLoader>())
{}

SkillAppService::~SkillAppService() = default;

int SkillAppService::loadSkillsFromDir()
{
    return m_loader->loadFromDir(m_skillDir, *m_registry);
}

bool SkillAppService::loadSkill(const std::string& skillDirPath)
{
    return m_loader->loadOne(skillDirPath, *m_registry);
}

std::vector<domain::skill::SkillMeta> SkillAppService::listSkills() const
{
    return m_registry->getAllMeta();
}

void SkillAppService::unloadSkill(const std::string& skillId)
{
    m_registry->unregisterSkill(domain::skill::SkillId(skillId));
}

std::shared_ptr<domain::skill::SkillRegistry> SkillAppService::registry() const
{
    return m_registry;
}

} // namespace application
