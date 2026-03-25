#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <string>
#include "document_id.h"
#include "knowledge_type.h"

namespace domain {
namespace knowledge {

/**
 * @brief 文档实体
 */
class Document {
public:
    Document();
    ~Document();

    // Getter
    DocumentId getId() const { return m_id; }
    std::string getTitle() const { return m_title; }
    std::string getContent() const { return m_content; }
    KnowledgeType getType() const { return m_type; }

    // Setter
    void setId(const DocumentId& id) { m_id = id; }
    void setTitle(const std::string& title) { m_title = title; }
    void setContent(const std::string& content) { m_content = content; }
    void setType(KnowledgeType type) { m_type = type; }

private:
    DocumentId m_id;
    std::string m_title;
    std::string m_content;
    KnowledgeType m_type;
};

} // namespace knowledge
} // namespace domain

#endif // DOCUMENT_H
