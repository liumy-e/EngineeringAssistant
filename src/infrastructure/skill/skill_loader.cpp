#include "skill_loader.h"
#include "builtin_skills.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace infrastructure {
namespace skill {

// ----------------------------------------------------------------
// 公共工具：读取 JSON 文件
// ----------------------------------------------------------------
static QJsonObject readJsonFile(const std::string& path)
{
    QFile file(QString::fromStdString(path));
    if (!file.open(QIODevice::ReadOnly)) return {};
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError) return {};
    return doc.object();
}

// ----------------------------------------------------------------
int SkillLoader::loadFromDir(const std::string& dir,
                             domain::skill::SkillRegistry& registry)
{
    QDir qdir(QString::fromStdString(dir));
    if (!qdir.exists()) return 0;

    int count = 0;
    for (auto& entry : qdir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        std::string skillDir = dir + "/" + entry.toStdString();
        if (loadOne(skillDir, registry)) ++count;
    }
    return count;
}

// ----------------------------------------------------------------
bool SkillLoader::loadOne(const std::string& skillDir,
                          domain::skill::SkillRegistry& registry)
{
    std::string jsonPath = skillDir + "/skill.json";
    QFile jsonFile(QString::fromStdString(jsonPath));
    if (!jsonFile.exists()) {
        qWarning() << "skill.json not found:" << QString::fromStdString(jsonPath);
        return false;
    }

    try {
        auto meta = parseMeta(jsonPath);
        auto skillPtr = createSkill(meta, skillDir);
        if (!skillPtr) return false;
        return registry.registerSkill(std::move(skillPtr));
    } catch (const std::exception& e) {
        qWarning() << "Failed to load skill:" << e.what();
        return false;
    }
}

// ----------------------------------------------------------------
domain::skill::SkillMeta SkillLoader::parseMeta(const std::string& jsonPath)
{
    auto obj = readJsonFile(jsonPath);
    domain::skill::SkillMeta meta;
    meta.id          = domain::skill::SkillId(obj["id"].toString().toStdString());
    meta.name        = obj["name"].toString().toStdString();
    meta.version     = obj["version"].toString("1.0.0").toStdString();
    meta.description = obj["description"].toString().toStdString();
    meta.author      = obj["author"].toString().toStdString();
    meta.scriptPath  = obj["script"].toString("builtin").toStdString();

    auto paramsArr = obj["params"].toArray();
    for (auto p : paramsArr) {
        auto po = p.toObject();
        domain::skill::SkillParamDef pd;
        pd.name         = po["name"].toString().toStdString();
        pd.type         = po["type"].toString("string").toStdString();
        pd.description  = po["description"].toString().toStdString();
        pd.required     = po["required"].toBool(true);
        pd.defaultValue = po["default"].toString().toStdString();
        meta.params.push_back(pd);
    }
    return meta;
}

// ----------------------------------------------------------------
std::shared_ptr<domain::skill::ISkill>
SkillLoader::createSkill(const domain::skill::SkillMeta& meta,
                         const std::string& skillDir)
{
    // 内置 Skill 工厂（按 id 创建）
    auto builtin = BuiltinSkillFactory::create(meta.id.value, meta);
    if (builtin) return builtin;

    // 未来扩展：脚本 Skill、插件 Skill ...
    qWarning() << "No implementation for skill:" << QString::fromStdString(meta.id.value);
    return nullptr;
}

} // namespace skill
} // namespace infrastructure
