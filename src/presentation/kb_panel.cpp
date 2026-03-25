#include "kb_panel.h"
#include "../application/services/kb_app_service.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QInputDialog>
#include <QMessageBox>
#include <QApplication>

KBPanel::KBPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void KBPanel::setService(std::shared_ptr<application::KBAppService> service)
{
    m_service = std::move(service);
    refresh();
}

void KBPanel::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    // ---- 顶部标题栏 ----
    auto* titleBar = new QWidget(this);
    titleBar->setFixedHeight(48);
    titleBar->setStyleSheet(
        "QWidget { background: #ffffff; border-bottom: 1px solid #e8ecf0; }");
    auto* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(20, 0, 16, 0);

    auto* titleLabel = new QLabel("📚  知识库管理", titleBar);
    titleLabel->setStyleSheet(
        "QLabel { font-size: 14px; font-weight: bold; color: #1a2537;"
        " background: transparent; border: none; }");
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch(1);

    m_statusLbl = new QLabel("未加载", titleBar);
    m_statusLbl->setStyleSheet(
        "QLabel { color: #8a9bb0; font-size: 11px;"
        " background: #f0f5fb; border: 1px solid #dde8f5;"
        " border-radius: 10px; padding: 2px 10px; }");
    titleLayout->addWidget(m_statusLbl);
    titleLayout->addSpacing(8);

    // 导入网址按钮
    m_importUrlBtn = new QPushButton("🌐 导入网址", titleBar);
    m_importUrlBtn->setFixedHeight(28);
    m_importUrlBtn->setStyleSheet(
        "QPushButton {"
        "  border: 1.5px solid #20a880; border-radius: 5px;"
        "  background: #f0fbf7; color: #168060; font-size: 12px; font-weight: bold;"
        "  padding: 0 10px;"
        "}"
        "QPushButton:hover { background: #d8f5ec; }"
        "QPushButton:pressed { background: #b8ead8; }"
        "QPushButton:disabled { color: #b0c8c0; border-color: #c8e8dc; }"
    );
    connect(m_importUrlBtn, &QPushButton::clicked, this, &KBPanel::onImportUrl);
    titleLayout->addWidget(m_importUrlBtn);

    layout->addWidget(titleBar);

    // ---- 主体：左侧列表 + 右侧操作/搜索/结果 ----
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(1);
    splitter->setStyleSheet("QSplitter::handle { background: #e8ecf0; }");

    // === 左侧：知识库列表 ===
    auto* leftWidget = new QWidget(splitter);
    leftWidget->setStyleSheet("QWidget { background: #f7f8fa; }");
    auto* leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(12, 12, 0, 12);
    leftLayout->setSpacing(6);

    auto* listTitle = new QLabel("可用知识库", leftWidget);
    listTitle->setStyleSheet(
        "QLabel { font-size: 11px; color: #8a9bb0; font-weight: bold;"
        " background: transparent; }");
    leftLayout->addWidget(listTitle);

    m_kbList = new QListWidget(leftWidget);
    m_kbList->setFrameShape(QFrame::NoFrame);
    m_kbList->setSpacing(2);
    m_kbList->setStyleSheet(
        "QListWidget { background: transparent; border: none; outline: none; }"
        "QListWidget::item {"
        "  background: #ffffff;"
        "  border: 1px solid #e8ecf0;"
        "  border-radius: 6px;"
        "  padding: 8px 10px;"
        "  margin: 0 0 3px 0;"
        "  color: #2c3e55; font-size: 13px;"
        "}"
        "QListWidget::item:hover {"
        "  background: #f0f7ff; border-color: #b8d4f0;"
        "}"
        "QListWidget::item:selected {"
        "  background: #e6f2ff; border-color: #4A90D9; color: #1a6fc4;"
        "}"
    );
    connect(m_kbList, &QListWidget::itemClicked, this, &KBPanel::onKBSelected);
    leftLayout->addWidget(m_kbList, 1);

    // 加载/卸载按钮
    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(8);
    m_loadBtn = new QPushButton("加 载", leftWidget);
    m_loadBtn->setStyleSheet(
        "QPushButton {"
        "  border: 1.5px solid #4A90D9; border-radius: 5px;"
        "  padding: 5px 0; background: #fff; color: #4A90D9;"
        "  font-size: 12px; font-weight: bold;"
        "}"
        "QPushButton:hover { background: #eef5fd; }"
        "QPushButton:pressed { background: #dceefb; }"
    );
    connect(m_loadBtn, &QPushButton::clicked, this, &KBPanel::onLoadKB);

    m_unloadBtn = new QPushButton("卸 载", leftWidget);
    m_unloadBtn->setStyleSheet(
        "QPushButton {"
        "  border: 1.5px solid #e88c8c; border-radius: 5px;"
        "  padding: 5px 0; background: #fff; color: #cc4444;"
        "  font-size: 12px;"
        "}"
        "QPushButton:hover { background: #fff5f5; }"
        "QPushButton:pressed { background: #ffeaea; }"
    );
    connect(m_unloadBtn, &QPushButton::clicked, this, &KBPanel::onUnloadKB);

    btnRow->addWidget(m_loadBtn);
    btnRow->addWidget(m_unloadBtn);
    leftLayout->addLayout(btnRow);

    splitter->addWidget(leftWidget);

    // === 右侧：搜索 + 结果 ===
    auto* rightWidget = new QWidget(splitter);
    rightWidget->setStyleSheet("QWidget { background: #ffffff; }");
    auto* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(16, 14, 16, 14);
    rightLayout->setSpacing(10);

    // 搜索行
    auto* searchRow = new QHBoxLayout();
    searchRow->setSpacing(8);
    m_searchEdit = new QLineEdit(rightWidget);
    m_searchEdit->setPlaceholderText("在知识库中搜索文档...");
    m_searchEdit->setFixedHeight(34);
    m_searchEdit->setStyleSheet(
        "QLineEdit {"
        "  border: 1.5px solid #dde3ec; border-radius: 6px;"
        "  padding: 0 12px; font-size: 13px; color: #1a2537;"
        "  background: #f9fafc;"
        "}"
        "QLineEdit:focus { border-color: #4A90D9; background: #ffffff; }"
    );
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &KBPanel::onSearch);
    searchRow->addWidget(m_searchEdit, 1);

    auto* searchBtn = new QPushButton("搜索", rightWidget);
    searchBtn->setFixedSize(64, 34);
    searchBtn->setStyleSheet(
        "QPushButton {"
        "  border: none; border-radius: 6px;"
        "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
        "      stop:0 #5ca0e8, stop:1 #3d82d0);"
        "  color: white; font-size: 12px; font-weight: bold;"
        "}"
        "QPushButton:hover { background: #4d90e0; }"
        "QPushButton:pressed { background: #2d72c0; }"
    );
    connect(searchBtn, &QPushButton::clicked, this, &KBPanel::onSearch);
    searchRow->addWidget(searchBtn);
    rightLayout->addLayout(searchRow);

    // 结果显示
    m_resultView = new QTextBrowser(rightWidget);
    m_resultView->setFrameShape(QFrame::NoFrame);
    m_resultView->setStyleSheet(
        "QTextBrowser { background: #ffffff; border: none; padding: 4px 0; }"
        "QScrollBar:vertical { background: transparent; width: 6px; }"
        "QScrollBar::handle:vertical { background: #c8d4e0; border-radius: 3px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    );
    m_resultView->setHtml(
        "<div style='text-align:center; padding: 60px 20px; color: #b0bcc8;'>"
        "<div style='font-size: 36px; margin-bottom: 12px;'>📚</div>"
        "<div style='font-size: 13px;'>从左侧选择并加载知识库<br/>然后在此处搜索文档</div>"
        "</div>"
    );
    rightLayout->addWidget(m_resultView, 1);

    splitter->addWidget(rightWidget);
    splitter->setSizes({200, 520});

    layout->addWidget(splitter, 1);
}

void KBPanel::refresh()
{
    if (!m_service) return;

    m_kbList->clear();
    auto names = m_service->listKnowledgeBases();
    for (auto& name : names) {
        auto* item = new QListWidgetItem(
            QString("📚  %1").arg(QString::fromStdString(name)), m_kbList);
        item->setData(Qt::UserRole, QString::fromStdString(name));
    }

    auto kb = m_service->currentKnowledgeBase();
    if (kb) {
        m_statusLbl->setText(
            QString("📗  %1 · %2 篇")
            .arg(QString::fromStdString(kb->getName()))
            .arg(kb->getDocumentCount()));
        m_statusLbl->setStyleSheet(
            "QLabel { color: #2e8b57; font-size: 11px;"
            " background: #edfaf3; border: 1px solid #a8ddc0;"
            " border-radius: 10px; padding: 2px 10px; }");
    } else {
        m_statusLbl->setText("未加载");
        m_statusLbl->setStyleSheet(
            "QLabel { color: #8a9bb0; font-size: 11px;"
            " background: #f0f5fb; border: 1px solid #dde8f5;"
            " border-radius: 10px; padding: 2px 10px; }");
    }
}

void KBPanel::onKBSelected(QListWidgetItem* item)
{
    if (!item || !m_service) return;
    QString name = item->data(Qt::UserRole).toString();
    bool ok = m_service->loadKnowledgeBase(name.toStdString());
    if (ok) {
        emit knowledgeBaseChanged();
        refresh();
        auto kb = m_service->currentKnowledgeBase();
        if (kb) {
            m_resultView->setHtml(QString(
                "<div style='padding:16px;'>"
                "<div style='font-size:16px; font-weight:bold; color:#1a2537; margin-bottom:8px;'>"
                "📚  %1"
                "</div>"
                "<p style='color:#4a5f75; font-size:13px;'>文档数量: <b>%2</b> 篇</p>"
                "<div style='background:#edfaf3; border:1px solid #a8ddc0; border-radius:8px;"
                " padding:10px 14px; font-size:12px; color:#2e8b57; margin-top:12px;'>"
                "✅  知识库已加载，聊天时将自动检索相关内容"
                "</div>"
                "</div>"
            ).arg(QString::fromStdString(kb->getName())).arg(kb->getDocumentCount()));
        }
    }
}

void KBPanel::onLoadKB()
{
    auto* item = m_kbList->currentItem();
    if (item) onKBSelected(item);
}

void KBPanel::onUnloadKB()
{
    if (!m_service) return;
    m_service->unloadKnowledgeBase();
    emit knowledgeBaseChanged();
    refresh();
    m_resultView->setHtml(
        "<div style='text-align:center; padding:60px 20px; color:#b0bcc8;'>"
        "<div style='font-size:32px; margin-bottom:10px;'>📤</div>"
        "<div style='font-size:13px;'>知识库已卸载</div>"
        "</div>"
    );
}

void KBPanel::onSearch()
{
    if (!m_service) return;
    QString keyword = m_searchEdit->text().trimmed();
    if (keyword.isEmpty()) return;

    auto docs = m_service->search(keyword.toStdString());
    if (docs.empty()) {
        m_resultView->setHtml(
            "<div style='text-align:center; padding:40px 20px; color:#b0bcc8;'>"
            "<div style='font-size:28px; margin-bottom:8px;'>🔍</div>"
            "<div style='font-size:13px;'>未找到相关内容<br/>"
            "<span style='font-size:11px;'>请先加载知识库，或尝试其他关键词</span></div>"
            "</div>"
        );
        return;
    }

    QString html = QString(
        "<div style='padding:0 0 12px 0; border-bottom:1px solid #eef2f8; margin-bottom:12px;'>"
        "<span style='font-size:13px; color:#4a5f75;'>搜索 </span>"
        "<b style='color:#1a2537;'>\"%1\"</b>"
        "<span style='color:#8a9bb0; font-size:12px; margin-left:8px;'>%2 篇结果</span>"
        "</div>"
    ).arg(keyword.toHtmlEscaped()).arg(docs.size());

    for (auto& doc : docs) {
        html += QString(
            "<div style='border:1px solid #e8ecf0; border-radius:8px;"
            " padding:12px 14px; margin:0 0 10px 0;"
            " background:#ffffff;'>"
            "  <div style='font-size:13px; font-weight:bold; color:#1a2537;"
            "    margin-bottom:5px;'>%1</div>"
            "  <div style='font-size:12px; color:#6a8aaa; line-height:1.6;'>%2...</div>"
            "</div>"
        ).arg(
            QString::fromStdString(doc.getTitle()),
            QString::fromStdString(
                doc.getContent().substr(0, std::min(doc.getContent().size(), size_t(200)))
            ).toHtmlEscaped()
        );
    }
    m_resultView->setHtml(html);
}

void KBPanel::onImportUrl()
{
    if (!m_service) {
        QMessageBox::warning(this, "提示", "请先加载一个知识库再导入网址。");
        return;
    }
    if (!m_service->currentKnowledgeBase()) {
        QMessageBox::warning(this, "提示",
            "请先从左侧列表选中并加载一个知识库，再导入网址。");
        return;
    }

    // 弹出 URL 输入框
    bool ok = false;
    QString url = QInputDialog::getText(
        this,
        "🌐 导入网页到知识库",
        QString("将网页内容导入到知识库「%1」\n\n请输入网址（支持 http/https）：")
            .arg(QString::fromStdString(m_service->currentKnowledgeBase()->getName())),
        QLineEdit::Normal,
        "https://",
        &ok
    );

    if (!ok || url.trimmed().isEmpty()) return;
    url = url.trimmed();
    if (!url.startsWith("http://") && !url.startsWith("https://")) {
        url = "https://" + url;
    }

    // 显示正在加载
    m_resultView->setHtml(
        "<div style='text-align:center; padding:60px 20px; color:#6a8aaa;'>"
        "<div style='font-size:30px; margin-bottom:10px;'>🌐</div>"
        "<div style='font-size:13px;'>正在抓取网页内容...</div>"
        "<div style='font-size:11px; color:#b0bcc8; margin-top:6px;'>" +
        url.toHtmlEscaped() + "</div>"
        "</div>"
    );
    QApplication::processEvents();

    int result = m_service->importFromUrl(url.toStdString());

    switch (result) {
    case 0: {
        auto kb = m_service->currentKnowledgeBase();
        refresh();
        m_resultView->setHtml(QString(
            "<div style='padding:20px;'>"
            "  <div style='background:#edfaf3; border:1px solid #a8ddc0;"
            "    border-radius:8px; padding:14px 18px; font-size:13px; color:#2e8b57;'>"
            "    ✅  网页导入成功！<br/>"
            "    <span style='font-size:11px; color:#5aaa80;'>"
            "    当前知识库共 %1 篇文档</span>"
            "  </div>"
            "  <div style='margin-top:14px; padding:10px 14px;"
            "    background:#f4f6fa; border-radius:6px; font-size:12px; color:#6a8aaa;'>"
            "    📎 来源: <a href='%2' style='color:#4A90D9;'>%2</a>"
            "  </div>"
            "</div>"
        ).arg(kb ? (int)kb->getDocumentCount() : 0).arg(url.toHtmlEscaped()));
        break;
    }
    case 1:
        QMessageBox::warning(this, "导入失败",
            QString("网络请求失败，请检查：\n• 网址是否正确\n• 网络连接是否正常\n• 该网站是否允许爬取\n\n网址: %1").arg(url));
        m_resultView->setHtml(
            "<div style='padding:20px; color:#d9363e; background:#fff2f0;"
            " border:1px solid #ffc8c5; border-radius:8px;'>"
            "⚠️ 网络请求失败，请检查网址和网络连接"
            "</div>"
        );
        break;
    case 2:
        QMessageBox::warning(this, "导入失败", "请先加载知识库再导入网址。");
        break;
    case 3:
        QMessageBox::critical(this, "导入失败",
            "文件写入失败，请检查知识库目录权限。");
        break;
    default:
        QMessageBox::warning(this, "导入失败", "未知错误。");
        break;
    }
}
