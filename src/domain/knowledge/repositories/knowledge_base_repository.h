#ifndef KNOWLEDGE_BASE_REPOSITORY_H
#define KNOWLEDGE_BASE_REPOSITORY_H

#include <memory>
#include "knowledge_id.h"
#include "knowledge_base.h"

namespace domain {
namespace knowledge {

/**
 * @brief 知识库仓储接口
 */
class KnowledgeBaseRepository {
public:
    virtual ~KnowledgeBaseRepository() = default;

    /**
     * @brief 保存知识库
     * @param kb 知识库对象
     */
    virtual void save(const KnowledgeBase& kb) = 0;

    /**
     * @brief 加载知识库
     * @param id 知识库ID
     * @return 知识库对象指针，如果不存在返回nullptr
     */
    virtual std::shared_ptr<KnowledgeBase> load(const KnowledgeId& id) = 0;

    /**
     * @brief 删除知识库
     * @param id 知识库ID
     */
    virtual void remove(const KnowledgeId& id) = 0;
};

} // namespace knowledge
} // namespace domain

#endif // KNOWLEDGE_BASE_REPOSITORY_H
