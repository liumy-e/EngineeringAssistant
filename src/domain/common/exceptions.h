#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <string>

namespace domain {

/**
 * @brief 领域异常基类
 */
class DomainException : public std::exception {
public:
    explicit DomainException(const std::string& message)
        : m_message(message)
    {
    }

    virtual ~DomainException() noexcept = default;

    const char* what() const noexcept override {
        return m_message.c_str();
    }

protected:
    std::string m_message;
};

/**
 * @brief 聊天异常
 */
class ChatException : public DomainException {
public:
    explicit ChatException(const std::string& message)
        : DomainException(message)
    {
    }
};

/**
 * @brief 知识库异常
 */
class KnowledgeException : public DomainException {
public:
    explicit KnowledgeException(const std::string& message)
        : DomainException(message)
    {
    }
};

/**
 * @brief 技能异常
 */
class SkillException : public DomainException {
public:
    explicit SkillException(const std::string& message)
        : DomainException(message)
    {
    }
};

/**
 * @brief 渲染异常
 */
class RenderException : public DomainException {
public:
    explicit RenderException(const std::string& message)
        : DomainException(message)
    {
    }
};

/**
 * @brief 存储异常
 */
class StorageException : public DomainException {
public:
    explicit StorageException(const std::string& message)
        : DomainException(message)
    {
    }
};

} // namespace domain

#endif // EXCEPTIONS_H
