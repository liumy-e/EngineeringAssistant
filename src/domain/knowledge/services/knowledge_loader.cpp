#include "knowledge_loader.h"
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QDebug>

namespace domain {
namespace knowledge {

KnowledgeLoader::KnowledgeLoader(const std::string& knowledgeDir)
    : m_knowledgeDir(knowledgeDir)
{
}

KnowledgeLoader::~KnowledgeLoader() {
}

std::shared_ptr<KnowledgeBase> KnowledgeLoader::loadKnowledgeBase(
    const std::string& name
) {
    auto kb = std::make_shared<KnowledgeBase>(name);

    QString dirPath = QString::fromStdString(m_knowledgeDir) + "/" +
                      QString::fromStdString(name);
    QDir dir(dirPath);

    if (!dir.exists()) {
        throw std::runtime_error("知识库目录不存在: " + name);
    }

    // 批量加载：收集所有文档后一次性添加，只触发一次索引重建
    auto documents = loadDocuments(dirPath.toStdString());
    kb->addDocuments(documents);

    qDebug() << "加载知识库" << QString::fromStdString(name)
             << "完成，文档数:" << documents.size();

    return kb;
}

std::vector<Document> KnowledgeLoader::loadDocuments(const std::string& dirPath) {
    std::vector<Document> documents;

    QDir dir(QString::fromStdString(dirPath));
    QFileInfoList files = dir.entryInfoList(
        QStringList() << "*.md" << "*.markdown",
        QDir::Files | QDir::NoDotAndDotDot
    );

    for (const QFileInfo& fileInfo : files) {
        try {
            Document doc = loadDocument(fileInfo.absoluteFilePath().toStdString());
            documents.push_back(doc);
        } catch (const std::exception& e) {
            qWarning() << "加载文档失败:" << fileInfo.filePath() << e.what();
        }
    }

    return documents;
}

Document KnowledgeLoader::loadDocument(const std::string& filePath) {
    Document doc;

    // 读取文件内容
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("无法打开文件: " + filePath);
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // 设置文档信息
    doc.setId(DocumentId(filePath));
    doc.setTitle(extractTitle(content.toStdString()));
    doc.setContent(content.toStdString());
    doc.setType(classifyDocument(filePath));

    return doc;
}

std::string KnowledgeLoader::extractTitle(const std::string& content) {
    // 尝试从 Markdown 中提取标题
    size_t pos = content.find("# ");
    if (pos != std::string::npos) {
        size_t end = content.find('\n', pos);
        if (end != std::string::npos) {
            return content.substr(pos + 2, end - pos - 2);
        }
    }

    // 如果没有找到标题，使用第一行
    pos = content.find('\n');
    if (pos != std::string::npos) {
        return content.substr(0, pos);
    }

    return "未命名文档";
}

KnowledgeType KnowledgeLoader::classifyDocument(const std::string& path) {
    std::string lowerPath = path;
    std::transform(lowerPath.begin(), lowerPath.end(),
                  lowerPath.begin(), ::tolower);

    if (lowerPath.find("standard") != std::string::npos ||
        lowerPath.find("gb") != std::string::npos ||
        lowerPath.find("iso") != std::string::npos) {
        return KnowledgeType::DesignStandard;
    } else if (lowerPath.find("process") != std::string::npos ||
               lowerPath.find("flow") != std::string::npos) {
        return KnowledgeType::ProcessGuide;
    } else {
        return KnowledgeType::DrawingSpecification;
    }
}

std::vector<std::string> KnowledgeLoader::listAvailableKnowledgeBases() const {
    std::vector<std::string> bases;

    QDir dir(QString::fromStdString(m_knowledgeDir));
    QFileInfoList subdirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QFileInfo& subdir : subdirs) {
        bases.push_back(subdir.fileName().toStdString());
    }

    return bases;
}

} // namespace knowledge
} // namespace domain
