#ifndef DOCUMENT_ID_H
#define DOCUMENT_ID_H

#include <string>

namespace domain {
namespace knowledge {

/**
 * @brief 文档ID值对象
 */
struct DocumentId {
    std::string value;

    DocumentId() : value("") {}
    explicit DocumentId(const std::string& val) : value(val) {}

    bool operator==(const DocumentId& other) const {
        return value == other.value;
    }

    bool operator!=(const DocumentId& other) const {
        return !(*this == other);
    }
};

} // namespace knowledge
} // namespace domain

#endif // DOCUMENT_ID_H
