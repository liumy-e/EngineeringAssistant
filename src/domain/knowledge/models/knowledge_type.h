#ifndef KNOWLEDGE_TYPE_H
#define KNOWLEDGE_TYPE_H

namespace domain {
namespace knowledge {

/**
 * @brief 知识类型枚举
 */
enum class KnowledgeType {
    DesignStandard,      ///< 设计标准
    DrawingSpecification, ///< 图纸规范
    ProcessGuide         ///< 流程指南
};

} // namespace knowledge
} // namespace domain

#endif // KNOWLEDGE_TYPE_H
