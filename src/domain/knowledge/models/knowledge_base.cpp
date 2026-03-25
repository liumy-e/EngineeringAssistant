#include "knowledge_base.h"
#include <algorithm>
#include <cctype>

namespace domain {
namespace knowledge {

KnowledgeBase::KnowledgeBase(const std::string& name)
    : m_id(KnowledgeId(name))
    , m_name(name)
{
}

KnowledgeBase::~KnowledgeBase() {
}

void KnowledgeBase::addDocument(const Document& doc) {
    QMutexLocker locker(&m_mutex);
    m_documents.push_back(doc);
    buildSearchIndex();
}

void KnowledgeBase::removeDocument(const DocumentId& id) {
    QMutexLocker locker(&m_mutex);

    auto it = std::remove_if(m_documents.begin(), m_documents.end(),
        [&id](const Document& doc) {
            return doc.getId() == id;
        });
    m_documents.erase(it, m_documents.end());
    buildSearchIndex();
}

std::vector<Document> KnowledgeBase::searchDocuments(const std::string& keyword) const {
    QMutexLocker locker(&m_mutex);

    if (keyword.empty()) {
        return m_documents;
    }

    std::vector<Document> results;
    std::string lowerKeyword = keyword;
    std::transform(lowerKeyword.begin(), lowerKeyword.end(),
                  lowerKeyword.begin(), ::tolower);

    for (const auto& doc : m_documents) {
        std::string lowerTitle = doc.getTitle();
        std::string lowerContent = doc.getContent();

        std::transform(lowerTitle.begin(), lowerTitle.end(),
                      lowerTitle.begin(), ::tolower);
        std::transform(lowerContent.begin(), lowerContent.end(),
                      lowerContent.begin(), ::tolower);

        if (lowerTitle.find(lowerKeyword) != std::string::npos ||
            lowerContent.find(lowerKeyword) != std::string::npos) {
            results.push_back(doc);
        }
    }

    return results;
}

bool KnowledgeBase::isLoaded() const {
    QMutexLocker locker(&m_mutex);
    return !m_documents.empty();
}

std::vector<Document> KnowledgeBase::getAllDocuments() const {
    QMutexLocker locker(&m_mutex);
    return m_documents;
}

size_t KnowledgeBase::getDocumentCount() const {
    QMutexLocker locker(&m_mutex);
    return m_documents.size();
}

void KnowledgeBase::buildSearchIndex() {
    // TODO: 实现倒排索引构建
    // 这里可以建立更高效的搜索索引
}

} // namespace knowledge
} // namespace domain
