#ifndef KB_APP_SERVICE_H
#define KB_APP_SERVICE_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "../../domain/knowledge/models/knowledge_base.h"
#include "../../domain/knowledge/services/knowledge_loader.h"
#include <QNetworkAccessManager>

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
     * @brief 新建知识库（在 knowledgeDir 下创建对应子目录）
     * @param name 知识库名称，只允许字母/数字/下划线/中文
     * @return true=创建成功；false=名称非法或目录已存在或创建失败
     */
    bool createKnowledgeBase(const std::string& name);

    /**
     * @brief 导入本地文件到当前知识库
     * @param filePath  本地文件路径（支持 .md .txt .cpp .h 等文本格式）
     * @param titleHint 标题提示，为空则用文件名
     * @return 0=成功, 2=未加载知识库, 3=读取失败, 4=写文件失败
     */
    int importFile(const std::string& filePath, const std::string& titleHint = "");

    /**
     * @brief 将纯文本内容直接写入当前知识库
     * @param title   文档标题
     * @param content 文档正文（Markdown 格式）
     * @return 0=成功, 2=未加载知识库, 3=标题/内容为空, 4=写文件失败
     */
    int importText(const std::string& title, const std::string& content);

    /**
     * @brief 从 URL 抓取网页内容并导入当前知识库（异步版本，不阻塞 UI）
     * @param url      目标网页 URL
     * @param title    文档标题（可选，为空则从页面提取）
     * @param callback 完成回调，参数为结果码：0=成功, 1=网络错误, 2=未加载知识库, 3=写文件失败
     */
    void importFromUrlAsync(const std::string& url, const std::string& title,
                            std::function<void(int)> callback);

private:
    std::unique_ptr<domain::knowledge::KnowledgeLoader> m_loader;
    std::shared_ptr<domain::knowledge::KnowledgeBase>   m_currentKB;
    std::string m_knowledgeDir;
    QNetworkAccessManager* m_nam = nullptr;  ///< 复用的网络管理器，避免每次重建连接池

    // 网络请求完成后执行的内部处理
    int handleNetworkReply(QNetworkReply* reply, const std::string& url,
                           const std::string& titleHint);
};

} // namespace application

#endif // KB_APP_SERVICE_H
