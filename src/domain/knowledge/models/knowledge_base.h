#ifndef KNOWLEDGE_BASE_H
#define KNOWLEDGE_BASE_H

#include <vector>
#include <QMutex>
#include <unordered_map>
#include "knowledge_id.h"
#include "document.h"

namespace domain {
namespace knowledge {

/**
 * @brief 知识库聚合根
 */
class KnowledgeBase {
public:
    explicit KnowledgeBase(const std::string& name);
    ~KnowledgeBase();

    // 业务方法
    void addDocument(const Document& doc);
    void removeDocument(const DocumentId& id);
    std::vector<Document> searchDocuments(const std::string& keyword) const;
    bool isLoaded() const;

    // Getter
    KnowledgeId getId() const { return m_id; }
    std::string getName() const { return m_name; }
    std::vector<Document> getAllDocuments() const;
    size_t getDocumentCount() const;

private:
    void buildSearchIndex();

    mutable QMutex m_mutex;
    KnowledgeId m_id;
    std::string m_name;
    std::vector<Document> m_documents;
    std::unordered_map<std::string, std::vector<DocumentId>> m_searchIndex;
};

} // namespace knowledge
} // namespace domain

#endif // KNOWLEDGE_BASE_H
