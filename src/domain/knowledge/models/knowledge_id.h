#ifndef KNOWLEDGE_ID_H
#define KNOWLEDGE_ID_H

#include <string>

namespace domain {
namespace knowledge {

/**
 * @brief 知识库ID值对象
 */
struct KnowledgeId {
    std::string value;

    KnowledgeId() : value("") {}
    explicit KnowledgeId(const std::string& val) : value(val) {}

    bool operator==(const KnowledgeId& other) const {
        return value == other.value;
    }

    bool operator!=(const KnowledgeId& other) const {
        return !(*this == other);
    }
};

} // namespace knowledge
} // namespace domain

#endif // KNOWLEDGE_ID_H
