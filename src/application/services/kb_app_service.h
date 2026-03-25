#ifndef KB_APP_SERVICE_H
#define KB_APP_SERVICE_H

#include <string>
#include <vector>
#include <memory>
#include "../../domain/knowledge/models/knowledge_base.h"
#include "../../domain/knowledge/services/knowledge_loader.h"

namespace application {

/**
 * @brief 知识库应用服务
 *
 * 提供知识库的加载、切换、查询操作。
 */
class KBAppService {
public:
    explicit KBAppService(const std::string& knowledgeDir);
    ~KBAppService() = default;

    /**
     * @brief 列出所有可用的知识库名称
     */
    std::vector<std::string> listKnowledgeBases() const;

    /**
     * @brief 加载指定知识库
     * @param name 知识库名称（对应 data/knowledge/ 下的子目录名）
     * @return 加载是否成功
     */
    bool loadKnowledgeBase(const std::string& name);

    /**
     * @brief 获取当前已加载的知识库
     */
    std::shared_ptr<domain::knowledge::KnowledgeBase> currentKnowledgeBase() const;

    /**
     * @brief 卸载当前知识库
     */
    void unloadKnowledgeBase();

    /**
     * @brief 在当前知识库中搜索
     */
    std::vector<domain::knowledge::Document>
        search(const std::string& keyword) const;

    /**
     * @brief 从 URL 抓取网页内容并导入当前知识库
     * @param url     目标网页 URL
     * @param title   文档标题（可选，为空则从页面提取）
     * @return 0=成功, 1=网络错误, 2=未加载知识库, 3=写文件失败
     */
    int importFromUrl(const std::string& url, const std::string& title = "");

private:
    std::unique_ptr<domain::knowledge::KnowledgeLoader> m_loader;
    std::shared_ptr<domain::knowledge::KnowledgeBase>   m_currentKB;
    std::string m_knowledgeDir;
};

} // namespace application

#endif // KB_APP_SERVICE_H
