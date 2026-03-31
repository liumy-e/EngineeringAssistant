#ifndef KNOWLEDGE_BASE_H
#define KNOWLEDGE_BASE_H

#include <vector>
#include <map>
#include <set>
#include <QMutex>
#include <unordered_map>
#include "knowledge_id.h"
#include "document.h"
#include "chunk.h"

namespace domain {
namespace knowledge {

/**
 * @brief 知识图谱节点
 */
struct GraphNode {
    std::string keyword;          ///< 关键词/实体
    int         freq = 0;         ///< 在知识库中的出现频次
    std::vector<DocumentId> docs; ///< 出现的文档列表（去重）
};

/**
 * @brief 知识图谱边（关键词共现）
 */
struct GraphEdge {
    std::string from;
    std::string to;
    int         weight = 0; ///< 共现次数（出现在同一文档的次数）
};

// ----------------------------------------------------------------

/**
 * @brief 知识库聚合根
 *
 * 职责：
 *   1. 管理文档集合（CRUD）
 *   2. 固定窗口+滑步切片（Sliding-Window Chunking）
 *   3. 关键词倒排索引搜索
 *   4. 关键词共现知识图谱
 */
class KnowledgeBase {
public:
    // ---- 切片参数 ----
    static constexpr int kChunkSize    = 500;  ///< 每片最大字符数
    static constexpr int kChunkOverlap = 100;  ///< 相邻切片重叠字符数

    explicit KnowledgeBase(const std::string& name);
    ~KnowledgeBase();

    // ---- 文档管理 ----
    void addDocument(const Document& doc);
    /** 批量添加文档（推荐使用，只重建一次索引） */
    void addDocuments(const std::vector<Document>& docs);
    void removeDocument(const DocumentId& id);
    std::vector<Document> searchDocuments(const std::string& keyword) const;
    bool isLoaded() const;

    // ---- Getter ----
    KnowledgeId           getId()           const { return m_id; }
    std::string           getName()         const { return m_name; }
    std::vector<Document> getAllDocuments()  const;
    size_t                getDocumentCount() const;

    // ---- 切片 API ----
    /** 获取所有切片 */
    std::vector<Chunk> getAllChunks() const;
    /** 获取指定文档的切片 */
    std::vector<Chunk> getChunks(const DocumentId& docId) const;
    /** 按关键词检索切片（返回包含关键词的切片列表） */
    std::vector<Chunk> searchChunks(const std::string& keyword) const;
    /** 获取切片总数 */
    size_t getChunkCount() const;

    // ---- 知识图谱 API ----
    /** 获取关键词节点列表（按频次降序，最多 maxNodes 个） */
    std::vector<GraphNode> getGraphNodes(int maxNodes = 50) const;
    /** 获取关键词共现边列表 */
    std::vector<GraphEdge> getGraphEdges(int maxEdges = 100) const;

private:
    // ---- 内部构建 ----
    void buildSearchIndex();          ///< 重建倒排索引
    void buildChunks();               ///< 重建切片
    void buildGraph();                ///< 重建知识图谱

    /** 滑动窗口切片单篇文档 */
    std::vector<Chunk> chunkDocument(const Document& doc) const;
    /** 从文本中提取关键词（简单分词：中文按双字词，英文按单词） */
    std::vector<std::string> extractKeywords(const std::string& text) const;

    mutable QMutex m_mutex;
    KnowledgeId    m_id;
    std::string    m_name;

    std::vector<Document> m_documents;
    std::vector<Chunk>    m_chunks;   ///< 所有切片（平铺）

    // 倒排索引：keyword -> [docId, ...]
    std::unordered_map<std::string, std::vector<DocumentId>> m_searchIndex;

    // 知识图谱：节点 & 边
    std::vector<GraphNode> m_graphNodes;
    std::vector<GraphEdge> m_graphEdges;
};

} // namespace knowledge
} // namespace domain

#endif // KNOWLEDGE_BASE_H
