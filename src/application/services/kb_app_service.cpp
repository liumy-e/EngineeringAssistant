#include "kb_app_service.h"
#include "../../domain/knowledge/models/document.h"
#include "../../domain/knowledge/models/document_id.h"
#include "../../domain/knowledge/models/knowledge_type.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QRegularExpression>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QTextStream>
#include <QDebug>

namespace application {

KBAppService::KBAppService(const std::string& knowledgeDir)
    : m_knowledgeDir(knowledgeDir)
    , m_loader(std::make_unique<domain::knowledge::KnowledgeLoader>(knowledgeDir))
    , m_nam(new QNetworkAccessManager())   // 复用：整个服务生命周期内只建一次连接池
{}

std::vector<std::string> KBAppService::listKnowledgeBases() const
{
    return m_loader->listAvailableKnowledgeBases();
}

bool KBAppService::loadKnowledgeBase(const std::string& name)
{
    try {
        m_currentKB = m_loader->loadKnowledgeBase(name);
        return m_currentKB && m_currentKB->isLoaded();
    } catch (...) {
        m_currentKB.reset();
        return false;
    }
}

std::shared_ptr<domain::knowledge::KnowledgeBase>
KBAppService::currentKnowledgeBase() const
{
    return m_currentKB;
}

void KBAppService::unloadKnowledgeBase()
{
    m_currentKB.reset();
}

std::vector<domain::knowledge::Document>
KBAppService::search(const std::string& keyword) const
{
    if (!m_currentKB) return {};
    return m_currentKB->searchDocuments(keyword);
}

bool KBAppService::createKnowledgeBase(const std::string& name)
{
    if (name.empty()) return false;

    QString dirPath = QString::fromStdString(m_knowledgeDir) + "/"
                    + QString::fromStdString(name);
    QDir dir;
    if (QDir(dirPath).exists()) return false;   // 已存在
    return dir.mkpath(dirPath);
}

// ----------------------------------------------------------------
int KBAppService::importFile(const std::string& filePath,
                             const std::string& titleHint)
{
    if (!m_currentKB) return 2;

    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "importFile: 无法打开文件" << QString::fromStdString(filePath);
        return 3;
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString content = in.readAll();
    file.close();

    // 标题：优先 titleHint，其次文件名（去掉路径和扩展名）
    QString title = titleHint.empty()
        ? QFileInfo(QString::fromStdString(filePath)).completeBaseName()
        : QString::fromStdString(titleHint);

    // 非 Markdown 文件用代码块包裹
    QString ext = QFileInfo(QString::fromStdString(filePath)).suffix().toLower();
    QString md;
    md += "# " + title + "\n\n";
    md += "> 导入文件: " + QFileInfo(QString::fromStdString(filePath)).fileName() + "\n";
    md += "> 导入时间: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm") + "\n\n";
    md += "---\n\n";
    if (ext != "md" && ext != "markdown" && ext != "txt") {
        md += "```" + ext + "\n";
        md += content;
        md += "\n```\n";
    } else {
        md += content;
    }

    return importText(title.toStdString(), md.toStdString());
}

// ----------------------------------------------------------------
int KBAppService::importText(const std::string& title,
                             const std::string& content)
{
    if (!m_currentKB) return 2;
    if (title.empty() || content.empty()) return 3;

    // 写文件
    QString kbDir = QString::fromStdString(m_knowledgeDir) + "/"
                  + QString::fromStdString(m_currentKB->getName());
    QString safeTitle = QString::fromStdString(title);
    safeTitle.replace(QRegularExpression(R"([/\\:*?"<>|])"), "_");
    safeTitle = safeTitle.left(60);
    QString fileName = "text_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss")
                     + "_" + safeTitle + ".md";
    QString filePath = kbDir + "/" + fileName;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "importText: 无法写入文件" << filePath;
        return 4;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << QString::fromStdString(content);
    file.close();

    // 热更新到内存
    domain::knowledge::Document doc;
    doc.setId(domain::knowledge::DocumentId(filePath.toStdString()));
    doc.setTitle(title);
    doc.setContent(content);
    doc.setType(domain::knowledge::KnowledgeType::DrawingSpecification);
    m_currentKB->addDocument(doc);

    qDebug() << "importText 成功:" << QString::fromStdString(title) << "->" << filePath;
    return 0;
}

// ----------------------------------------------------------------
// 简单 HTML → 纯文本提取
// ----------------------------------------------------------------
static QString htmlToPlainText(const QString& html)
{
    QString text = html;

    // 移除 <script> / <style> 块
    text.remove(QRegularExpression("<script[^>]*>[\\s\\S]*?</script>",
        QRegularExpression::CaseInsensitiveOption));
    text.remove(QRegularExpression("<style[^>]*>[\\s\\S]*?</style>",
        QRegularExpression::CaseInsensitiveOption));

    // <br> / <p> / <div> / <li> 换成换行
    text.replace(QRegularExpression("<br\\s*/?>",
        QRegularExpression::CaseInsensitiveOption), "\n");
    text.replace(QRegularExpression("</(p|div|li|tr|h[1-6])>",
        QRegularExpression::CaseInsensitiveOption), "\n");

    // 移除剩余 HTML 标签
    text.remove(QRegularExpression("<[^>]*>"));

    // HTML 实体
    text.replace("&nbsp;", " ");
    text.replace("&amp;",  "&");
    text.replace("&lt;",   "<");
    text.replace("&gt;",   ">");
    text.replace("&quot;", "\"");
    text.replace("&#39;",  "'");

    // 压缩多余空行
    text.replace(QRegularExpression("\n{3,}"), "\n\n");

    return text.trimmed();
}

// ----------------------------------------------------------------
// 提取 <title> 标签内容
// ----------------------------------------------------------------
static QString extractHtmlTitle(const QString& html)
{
    QRegularExpression re("<title[^>]*>([^<]*)</title>",
        QRegularExpression::CaseInsensitiveOption);
    auto match = re.match(html);
    if (match.hasMatch())
        return match.captured(1).trimmed();
    return QString();
}

// ----------------------------------------------------------------
int KBAppService::handleNetworkReply(QNetworkReply* reply,
                                     const std::string& url,
                                     const std::string& titleHint)
{
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "URL 抓取失败:" << reply->errorString();
        return 1;
    }

    QString htmlContent = QString::fromUtf8(reply->readAll());

    // --- 提取正文 ---
    QString pageTitle = titleHint.empty()
        ? extractHtmlTitle(htmlContent)
        : QString::fromStdString(titleHint);
    if (pageTitle.isEmpty())
        pageTitle = QString::fromStdString(url);

    QString plainText = htmlToPlainText(htmlContent);

    // --- 构建 Markdown 内容 ---
    QString md;
    md += "# " + pageTitle + "\n\n";
    md += "> 来源: " + QString::fromStdString(url) + "\n";
    md += "> 导入时间: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm") + "\n\n";
    md += "---\n\n";
    md += plainText;

    // --- 保存到知识库目录 ---
    QString kbDir = QString::fromStdString(m_knowledgeDir) + "/"
                  + QString::fromStdString(m_currentKB->getName());
    QString safeTitle = pageTitle;
    safeTitle.replace(QRegularExpression("[/\\\\:*?\"<>|]"), "_");
    safeTitle = safeTitle.left(60);
    QString fileName = "web_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss")
                     + "_" + safeTitle + ".md";
    QString filePath = kbDir + "/" + fileName;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "无法写入文件:" << filePath;
        return 3;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << md;
    file.close();

    // --- 加入当前知识库（内存中热更新）---
    domain::knowledge::Document doc;
    doc.setId(domain::knowledge::DocumentId(filePath.toStdString()));
    doc.setTitle(pageTitle.toStdString());
    doc.setContent(md.toStdString());
    doc.setType(domain::knowledge::KnowledgeType::DrawingSpecification);
    m_currentKB->addDocument(doc);

    qDebug() << "网页导入成功:" << pageTitle << "->" << filePath;
    return 0;
}

// ----------------------------------------------------------------
void KBAppService::importFromUrlAsync(const std::string& url,
                                      const std::string& titleHint,
                                      std::function<void(int)> callback)
{
    if (!m_currentKB) {
        if (callback) callback(2);
        return;
    }

    QNetworkRequest request(QUrl(QString::fromStdString(url)));
    request.setRawHeader("User-Agent",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36");
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    QNetworkReply* reply = m_nam->get(request);

    // 用 lambda 捕获上下文，回调执行在主线程 Qt 事件循环里，不阻塞 UI
    QObject::connect(reply, &QNetworkReply::finished,
                     [this, reply, url, titleHint, callback]() {
        int result = handleNetworkReply(reply, url, titleHint);
        reply->deleteLater();
        if (callback) callback(result);
    });
}

} // namespace application
