#include "knowledge_base.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <map>
#include <set>

namespace domain {
namespace knowledge {

// ================================================================
// 构造 / 析构
// ================================================================
KnowledgeBase::KnowledgeBase(const std::string& name)
    : m_id(KnowledgeId(name))
    , m_name(name)
{}

KnowledgeBase::~KnowledgeBase() {}

// ================================================================
// 文档管理
// ================================================================
void KnowledgeBase::addDocument(const Document& doc) {
    QMutexLocker locker(&m_mutex);
    m_documents.push_back(doc);
    buildSearchIndex();
    buildChunks();
    buildGraph();
}

void KnowledgeBase::addDocuments(const std::vector<Document>& docs) {
    if (docs.empty()) return;
    QMutexLocker locker(&m_mutex);
    m_documents.reserve(m_documents.size() + docs.size());
    for (const auto& doc : docs)
        m_documents.push_back(doc);
    buildSearchIndex();
    buildChunks();
    buildGraph();
}

void KnowledgeBase::removeDocument(const DocumentId& id) {
    QMutexLocker locker(&m_mutex);
    auto it = std::remove_if(m_documents.begin(), m_documents.end(),
        [&id](const Document& doc) { return doc.getId() == id; });
    m_documents.erase(it, m_documents.end());
    buildSearchIndex();
    buildChunks();
    buildGraph();
}

// ================================================================
// 搜索（文档级）
// ================================================================
std::vector<Document> KnowledgeBase::searchDocuments(const std::string& keyword) const {
    QMutexLocker locker(&m_mutex);
    if (keyword.empty()) return m_documents;

    std::string lowerKw = keyword;
    std::transform(lowerKw.begin(), lowerKw.end(), lowerKw.begin(), ::tolower);

    std::vector<Document> results;
    for (const auto& doc : m_documents) {
        std::string lowerTitle   = doc.getTitle();
        std::string lowerContent = doc.getContent();
        std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);
        std::transform(lowerContent.begin(), lowerContent.end(), lowerContent.begin(), ::tolower);

        if (lowerTitle.find(lowerKw) != std::string::npos ||
            lowerContent.find(lowerKw) != std::string::npos)
            results.push_back(doc);
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

// ================================================================
// 切片 API
// ================================================================
std::vector<Chunk> KnowledgeBase::getAllChunks() const {
    QMutexLocker locker(&m_mutex);
    return m_chunks;
}

std::vector<Chunk> KnowledgeBase::getChunks(const DocumentId& docId) const {
    QMutexLocker locker(&m_mutex);
    std::vector<Chunk> result;
    for (const auto& c : m_chunks)
        if (c.docId == docId) result.push_back(c);
    return result;
}

std::vector<Chunk> KnowledgeBase::searchChunks(const std::string& keyword) const {
    QMutexLocker locker(&m_mutex);
    if (keyword.empty()) return m_chunks;

    std::string lowerKw = keyword;
    std::transform(lowerKw.begin(), lowerKw.end(), lowerKw.begin(), ::tolower);

    std::vector<Chunk> results;
    for (const auto& chunk : m_chunks) {
        std::string lowerText = chunk.text;
        std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(), ::tolower);
        if (lowerText.find(lowerKw) != std::string::npos)
            results.push_back(chunk);
    }
    return results;
}

size_t KnowledgeBase::getChunkCount() const {
    QMutexLocker locker(&m_mutex);
    return m_chunks.size();
}

// ================================================================
// 知识图谱 API
// ================================================================
std::vector<GraphNode> KnowledgeBase::getGraphNodes(int maxNodes) const {
    QMutexLocker locker(&m_mutex);
    auto nodes = m_graphNodes;
    if ((int)nodes.size() > maxNodes)
        nodes.resize(maxNodes);
    return nodes;
}

std::vector<GraphEdge> KnowledgeBase::getGraphEdges(int maxEdges) const {
    QMutexLocker locker(&m_mutex);
    auto edges = m_graphEdges;
    if ((int)edges.size() > maxEdges)
        edges.resize(maxEdges);
    return edges;
}

// ================================================================
// 内部构建：倒排索引
// ================================================================
void KnowledgeBase::buildSearchIndex() {
    m_searchIndex.clear();
    for (const auto& doc : m_documents) {
        auto kws = extractKeywords(doc.getTitle() + " " + doc.getContent());
        for (const auto& kw : kws) {
            auto& ids = m_searchIndex[kw];
            // 避免重复
            if (std::find(ids.begin(), ids.end(), doc.getId()) == ids.end())
                ids.push_back(doc.getId());
        }
    }
}

// ================================================================
// 内部构建：切片
// ================================================================
void KnowledgeBase::buildChunks() {
    m_chunks.clear();
    for (const auto& doc : m_documents) {
        auto chunks = chunkDocument(doc);
        for (auto& c : chunks)
            m_chunks.push_back(std::move(c));
    }
}

std::vector<Chunk> KnowledgeBase::chunkDocument(const Document& doc) const {
    const std::string& text = doc.getContent();
    std::vector<Chunk> chunks;

    if (text.empty()) return chunks;

    int step  = kChunkSize - kChunkOverlap;
    if (step <= 0) step = kChunkSize;

    int idx = 0;
    size_t pos = 0;
    while (pos < text.size()) {
        // 尽量在段落/句子边界切分
        size_t end = pos + kChunkSize;
        if (end < text.size()) {
            // 向前找最近的换行符或句号（。是3字节UTF-8）
            size_t boundary = text.rfind('\n', end);
            if (boundary == std::string::npos || boundary <= pos) {
                // 搜索中文句号 "。"（3字节）
                static const std::string period = "\xe3\x80\x82"; // UTF-8: 。
                size_t b2 = text.rfind(period, end);
                if (b2 != std::string::npos && b2 > pos)
                    boundary = b2 + 3; // 跳过3字节
            }
            if (boundary != std::string::npos && boundary > pos)
                end = boundary;
        } else {
            end = text.size();
        }

        Chunk c;
        c.docId = doc.getId();
        c.index = idx++;
        c.text  = text.substr(pos, end - pos);
        chunks.push_back(std::move(c));

        if (end >= text.size()) break;
        // 滑动步长（pos 前进 step 而非 kChunkSize，保留 overlap）
        pos += step;
    }
    return chunks;
}

// ================================================================
// 内部构建：知识图谱（关键词共现）
// ================================================================
void KnowledgeBase::buildGraph() {
    m_graphNodes.clear();
    m_graphEdges.clear();

    // 每篇文档提取关键词集合
    // 节点：keyword → {freq, docs}
    std::map<std::string, GraphNode> nodeMap;
    // 共现：(kwA,kwB) → count
    std::map<std::pair<std::string,std::string>, int> coOccur;

    for (const auto& doc : m_documents) {
        auto kws = extractKeywords(doc.getTitle() + " " + doc.getContent());
        // 去重
        std::set<std::string> kwSet(kws.begin(), kws.end());

        for (const auto& kw : kwSet) {
            auto& node = nodeMap[kw];
            node.keyword = kw;
            node.freq++;
            node.docs.push_back(doc.getId());
        }

        // 共现对（取前 30 个关键词，避免 O(N²) 爆炸）
        std::vector<std::string> kwVec(kwSet.begin(), kwSet.end());
        if (kwVec.size() > 30) kwVec.resize(30);
        for (size_t i = 0; i < kwVec.size(); ++i) {
            for (size_t j = i + 1; j < kwVec.size(); ++j) {
                auto key = std::make_pair(kwVec[i], kwVec[j]);
                coOccur[key]++;
            }
        }
    }

    // 填充节点，按频次排序
    for (auto& [kw, node] : nodeMap)
        m_graphNodes.push_back(std::move(node));
    std::sort(m_graphNodes.begin(), m_graphNodes.end(),
              [](const GraphNode& a, const GraphNode& b){ return a.freq > b.freq; });

    // 填充边，按权重排序
    for (auto& [pair, cnt] : coOccur) {
        if (cnt < 1) continue;
        GraphEdge e;
        e.from   = pair.first;
        e.to     = pair.second;
        e.weight = cnt;
        m_graphEdges.push_back(std::move(e));
    }
    std::sort(m_graphEdges.begin(), m_graphEdges.end(),
              [](const GraphEdge& a, const GraphEdge& b){ return a.weight > b.weight; });
}

// ================================================================
// 关键词提取（简易版：中文双字词 + 英文单词，过滤停用词）
// ================================================================
std::vector<std::string> KnowledgeBase::extractKeywords(const std::string& text) const {
    // 停用词（最小集，只排除高频无意义词）
    static const std::set<std::string> stopWords = {
        "的","了","在","是","我","有","和","就","不","人","都","一","一个",
        "上","也","很","到","说","要","去","你","会","着","没有","看","好",
        "自己","这","那","么","之","以","及","与","其","但","或","而","且",
        "the","a","an","in","on","at","to","of","and","or","is","are",
        "was","were","it","its","for","with","this","that","as","from"
    };

    std::vector<std::string> keywords;

    // 按字节遍历：判断是否中文（UTF-8 三字节序列）
    const unsigned char* p = reinterpret_cast<const unsigned char*>(text.c_str());
    const unsigned char* end = p + text.size();

    std::string wordBuf; // 英文单词缓冲
    std::string prevCH;  // 上一个中文字

    auto flushEnglish = [&]() {
        if (!wordBuf.empty()) {
            // 转小写
            std::string lw = wordBuf;
            std::transform(lw.begin(), lw.end(), lw.begin(), ::tolower);
            if (lw.size() >= 2 && stopWords.find(lw) == stopWords.end())
                keywords.push_back(lw);
            wordBuf.clear();
        }
        prevCH.clear();
    };

    while (p < end) {
        unsigned char c = *p;
        if (c < 0x80) {
            // ASCII
            prevCH.clear();
            if (std::isalpha(c)) {
                wordBuf += (char)c;
            } else {
                flushEnglish();
            }
            p++;
        } else if ((c & 0xE0) == 0xC0) {
            // 2-byte UTF-8
            flushEnglish();
            if (p + 1 < end) {
                std::string ch(reinterpret_cast<const char*>(p), 2);
                prevCH = ch;
                p += 2;
            } else { p++; }
        } else if ((c & 0xF0) == 0xE0) {
            // 3-byte UTF-8 (中文 CJK 区)
            flushEnglish();
            if (p + 2 < end) {
                std::string ch(reinterpret_cast<const char*>(p), 3);
                // 生成双字词
                if (!prevCH.empty()) {
                    std::string bigram = prevCH + ch;
                    if (stopWords.find(bigram) == stopWords.end())
                        keywords.push_back(bigram);
                }
                prevCH = ch;
                p += 3;
            } else { p++; }
        } else if ((c & 0xF8) == 0xF0) {
            // 4-byte UTF-8 (emoji 等)
            flushEnglish();
            p += 4;
        } else {
            flushEnglish();
            p++;
        }
    }
    flushEnglish();

    // 去重
    std::sort(keywords.begin(), keywords.end());
    keywords.erase(std::unique(keywords.begin(), keywords.end()), keywords.end());
    return keywords;
}

} // namespace knowledge
} // namespace domain
