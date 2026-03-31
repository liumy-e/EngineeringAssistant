#ifndef CHUNK_H
#define CHUNK_H

#include <string>
#include "document_id.h"

namespace domain {
namespace knowledge {

/**
 * @brief 文档切片（Chunk）
 *
 * 一篇文档被分割为若干固定大小的切片，每个切片保存：
 *   - 所属文档 ID
 *   - 切片在文档中的索引（0-based）
 *   - 切片文本内容
 */
struct Chunk {
    DocumentId  docId;     ///< 所属文档 ID
    int         index = 0; ///< 切片序号
    std::string text;      ///< 切片正文

    // ---- 向量化字段（预留，当前用 TF 简化代替） ----
    std::vector<float> embedding; ///< 稠密向量（可选）

    bool operator==(const Chunk& o) const {
        return docId == o.docId && index == o.index;
    }
};

} // namespace knowledge
} // namespace domain

#endif // CHUNK_H
