#ifndef KNOWLEDGE_LOADER_H
#define KNOWLEDGE_LOADER_H

#include <string>
#include <vector>
#include <memory>
#include "knowledge_base.h"

namespace domain {
namespace knowledge {

/**
 * @brief 知识库加载器领域服务
 */
class KnowledgeLoader {
public:
    explicit KnowledgeLoader(const std::string& knowledgeDir);
    ~KnowledgeLoader();

    /**
     * @brief 加载知识库
     * @param name 知识库名称
     * @return 知识库对象指针
     */
    std::shared_ptr<KnowledgeBase> loadKnowledgeBase(const std::string& name);

    /**
     * @brief 列出所有可用的知识库
     * @return 知识库名称列表
     */
    std::vector<std::string> listAvailableKnowledgeBases() const;

private:
    std::vector<Document> loadDocuments(const std::string& dirPath);
    Document loadDocument(const std::string& filePath);
    std::string extractTitle(const std::string& content);
    KnowledgeType classifyDocument(const std::string& path);

    std::string m_knowledgeDir;
};

} // namespace knowledge
} // namespace domain

#endif // KNOWLEDGE_LOADER_H
