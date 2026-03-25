#include "kb_app_service.h"
#include "../../domain/knowledge/models/document.h"
#include "../../domain/knowledge/models/document_id.h"
#include "../../domain/knowledge/models/knowledge_type.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QUrl>
#include <QRegularExpression>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QTextStream>
#include <QDebug>

namespace application {

KBAppService::KBAppService(const std::string& knowledgeDir)
    : m_knowledgeDir(knowledgeDir)
    , m_loader(std::make_unique<domain::knowledge::KnowledgeLoader>(knowledgeDir))
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
int KBAppService::importFromUrl(const std::string& url, const std::string& titleHint)
{
    if (!m_currentKB) return 2;  // 未加载知识库

    // --- 1. 网络请求（同步，使用 QEventLoop）---
    QNetworkAccessManager nam;
    QNetworkRequest request(QUrl(QString::fromStdString(url)));
    request.setRawHeader("User-Agent",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36");
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    QEventLoop loop;
    QNetworkReply* reply = nam.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "URL 抓取失败:" << reply->errorString();
        reply->deleteLater();
        return 1;
    }

    QString htmlContent = QString::fromUtf8(reply->readAll());
    reply->deleteLater();

    // --- 2. 提取正文 ---
    QString pageTitle = titleHint.empty()
        ? extractHtmlTitle(htmlContent)
        : QString::fromStdString(titleHint);
    if (pageTitle.isEmpty())
        pageTitle = QString::fromStdString(url);

    QString plainText = htmlToPlainText(htmlContent);

    // --- 3. 构建 Markdown 内容 ---
    QString md;
    md += "# " + pageTitle + "\n\n";
    md += "> 来源: " + QString::fromStdString(url) + "\n";
    md += "> 导入时间: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm") + "\n\n";
    md += "---\n\n";
    md += plainText;

    // --- 4. 保存到知识库目录 ---
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

    // --- 5. 加入当前知识库（内存中热更新）---
    domain::knowledge::Document doc;
    doc.setId(domain::knowledge::DocumentId(filePath.toStdString()));
    doc.setTitle(pageTitle.toStdString());
    doc.setContent(md.toStdString());
    doc.setType(domain::knowledge::KnowledgeType::DrawingSpecification);
    m_currentKB->addDocument(doc);

    qDebug() << "网页导入成功:" << pageTitle << "->" << filePath;
    return 0;
}

} // namespace application
