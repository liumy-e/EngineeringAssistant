#include "chat_widget.h"
#include "../application/services/chat_app_service.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QDateTime>
#include <QDebug>
#include <QFrame>
#include <QScrollBar>
#include <QSizePolicy>

// ----------------------------------------------------------------
// 自定义输入框：Enter 发送，Shift+Enter 换行
// ----------------------------------------------------------------
class ChatInputEdit : public QTextEdit {
    Q_OBJECT
public:
    explicit ChatInputEdit(QWidget* parent = nullptr) : QTextEdit(parent) {}

signals:
    void sendRequested();

protected:
    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            if (event->modifiers() & Qt::ShiftModifier) {
                QTextEdit::keyPressEvent(event);
            } else {
                emit sendRequested();
            }
            return;
        }
        QTextEdit::keyPressEvent(event);
    }
};

#include "chat_widget.moc"

// ================================================================
ChatWidget::ChatWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    appendSystemText(
        "欢迎使用 **工程图纸设计助手** ✨\n\n"
        "你可以直接提问，或用 `@skill_id` 指令调用内置技能：\n\n"
        "- `@drawing-dimension-check drawing=图纸名` — 尺寸合规检查\n"
        "- `@standard-query standard=GB/T14689` — 工程标准查询\n"
        "- `@drawing-review part=零件名` — 自动生成审查报告"
    );
}

void ChatWidget::setService(std::shared_ptr<application::ChatAppService> service)
{
    m_service = std::move(service);
}

// ================================================================
void ChatWidget::setupUI()
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

    auto* titleLabel = new QLabel("💬  对话", titleBar);
    titleLabel->setStyleSheet(
        "QLabel { font-size: 14px; font-weight: bold; color: #1a2537;"
        " background: transparent; border: none; }");
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch(1);

    // 清空按钮放到顶部
    m_clearBtn = new QPushButton("清空", titleBar);
    m_clearBtn->setFixedSize(52, 28);
    m_clearBtn->setStyleSheet(
        "QPushButton { border: 1px solid #dde3ec; border-radius: 5px;"
        " padding: 0; background: #f7f8fa; color: #8a9bb0; font-size: 12px; }"
        "QPushButton:hover { background: #eff2f7; color: #4A90D9; border-color: #b8cee8; }"
        "QPushButton:pressed { background: #e0eaf5; }"
    );
    connect(m_clearBtn, &QPushButton::clicked, this, &ChatWidget::onClear);
    titleLayout->addWidget(m_clearBtn);

    layout->addWidget(titleBar);

    // ---- 消息显示区 ----
    m_messageView = new QTextBrowser(this);
    m_messageView->setOpenExternalLinks(true);
    m_messageView->setOpenLinks(false);
    m_messageView->setReadOnly(true);
    m_messageView->setStyleSheet(
        "QTextBrowser {"
        "  background: #f4f6fa;"
        "  border: none;"
        "  padding: 12px 16px;"
        "}"
        "QScrollBar:vertical {"
        "  background: transparent; width: 6px; margin: 0;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #c8d4e0; border-radius: 3px; min-height: 24px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0;"
        "}"
    );
    layout->addWidget(m_messageView, 1);

    // ---- 输入区（白色卡片风格）----
    auto* inputCard = new QWidget(this);
    inputCard->setStyleSheet(
        "QWidget#inputCard {"
        "  background: #ffffff;"
        "  border-top: 1px solid #e8ecf0;"
        "}"
    );
    inputCard->setObjectName("inputCard");
    auto* inputLayout = new QVBoxLayout(inputCard);
    inputLayout->setContentsMargins(16, 10, 16, 12);
    inputLayout->setSpacing(8);

    // 输入框
    auto* inputEdit = new ChatInputEdit(inputCard);
    m_inputEdit = inputEdit;
    m_inputEdit->setFixedHeight(80);
    m_inputEdit->setPlaceholderText("请输入问题，或使用 @skill_id 调用技能...");
    m_inputEdit->setStyleSheet(
        "QTextEdit {"
        "  border: 1.5px solid #dde3ec;"
        "  border-radius: 8px;"
        "  padding: 8px 12px;"
        "  font-size: 13px;"
        "  color: #1a2537;"
        "  background: #f9fafc;"
        "  line-height: 1.6;"
        "}"
        "QTextEdit:focus {"
        "  border-color: #4A90D9;"
        "  background: #ffffff;"
        "}"
    );
    connect(inputEdit, &ChatInputEdit::sendRequested, this, &ChatWidget::onSend);
    inputLayout->addWidget(m_inputEdit);

    // 底部操作行
    auto* bottomRow = new QHBoxLayout();
    bottomRow->setSpacing(8);

    m_statusLabel = new QLabel("Enter 发送  ·  Shift+Enter 换行", inputCard);
    m_statusLabel->setStyleSheet(
        "QLabel { color: #b0bcc8; font-size: 11px; background: transparent; }");
    bottomRow->addWidget(m_statusLabel);
    bottomRow->addStretch(1);

    m_sendBtn = new QPushButton("发 送", inputCard);
    m_sendBtn->setFixedSize(80, 32);
    m_sendBtn->setStyleSheet(
        "QPushButton {"
        "  border: none; border-radius: 6px;"
        "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
        "      stop:0 #5ca0e8, stop:1 #3d82d0);"
        "  color: white; font-size: 13px; font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
        "      stop:0 #6cb0f8, stop:1 #4d92e0);"
        "}"
        "QPushButton:pressed {"
        "  background: #2d72c0;"
        "}"
        "QPushButton:disabled {"
        "  background: #c8d8e8; color: #e8eef5;"
        "}"
    );
    connect(m_sendBtn, &QPushButton::clicked, this, &ChatWidget::onSend);
    bottomRow->addWidget(m_sendBtn);

    inputLayout->addLayout(bottomRow);
    layout->addWidget(inputCard);
}

// ----------------------------------------------------------------
void ChatWidget::onSend()
{
    if (!m_service) {
        appendErrorText("服务未初始化，请先配置聊天服务。");
        return;
    }

    QString text = m_inputEdit->toPlainText().trimmed();
    if (text.isEmpty()) return;

    m_inputEdit->clear();
    appendUserText(text);
    m_statusLabel->setText("处理中...");
    m_sendBtn->setEnabled(false);

    m_service->handleUserInput(
        text.toStdString(),
        [this](const std::string& html) {
            appendHtmlReply(QString::fromStdString(html));
        }
    );

    m_statusLabel->setText("Enter 发送  ·  Shift+Enter 换行");
    m_sendBtn->setEnabled(true);
}

void ChatWidget::onClear()
{
    if (m_service) m_service->clearSession();
    m_messageView->clear();
    appendSystemText("对话已清空，可以开始新的问题");
}

void ChatWidget::onKeyboardSend()
{
    onSend();
}

// ----------------------------------------------------------------
// 消息气泡渲染（含头像）
// ----------------------------------------------------------------
void ChatWidget::appendUserText(const QString& text)
{
    QString escaped = text.toHtmlEscaped().replace("\n", "<br/>");
    QString timeStr = QDateTime::currentDateTime().toString("hh:mm");

    // Qt 富文本引擎不支持 width:100% / display:inline-block / flex
    // 正确做法：外层 table align=right，用固定像素列宽推开左侧空间
    // 结构：[空白撑开列(伸缩)] | [气泡内容列, 固定宽] | [头像列, 40px]
    QString html = QString(
        "<table width='100%' cellspacing='0' cellpadding='0'"
        "       style='margin:10px 0 4px 0;'>"
        "<tr>"
        "  <td></td>"  // 左侧空白，自动占满剩余
        "  <td width='1' valign='top' style='padding-right:6px;'>"
        "    <table cellspacing='0' cellpadding='0'><tr><td"
        "      style='background:#4A90D9; color:#ffffff;"
        "             border-radius:14px 14px 2px 14px;"
        "             padding:9px 14px;"
        "             font-size:13px; line-height:1.65;'>"
        "      %1"
        "    </td></tr></table>"
        "    <p align='right'"
        "       style='margin:2px 0 0 0; font-size:10px; color:#b0bcc8;'>"
        "      %2"
        "    </p>"
        "  </td>"
        "  <td width='36' valign='top' align='center'"
        "      style='padding-top:2px;'>"
        "    <table cellspacing='0' cellpadding='0'><tr><td width='32' height='32'"
        "      align='center' valign='middle'"
        "      style='background:#4A90D9; border-radius:16px;"
        "             color:#ffffff; font-size:12px; font-weight:bold;'>"
        "      我"
        "    </td></tr></table>"
        "  </td>"
        "</tr>"
        "</table>"
    ).arg(escaped, timeStr);

    m_messageView->append(html);
    m_messageView->verticalScrollBar()->setValue(
        m_messageView->verticalScrollBar()->maximum());
}

void ChatWidget::appendHtmlReply(const QString& replyHtml)
{
    QString timeStr = QDateTime::currentDateTime().toString("hh:mm");

    // 结构：[头像列, 40px] | [气泡内容列, 固定宽] | [空白撑开列(伸缩)]
    QString wrapper = QString(
        "<table width='100%' cellspacing='0' cellpadding='0'"
        "       style='margin:10px 0 4px 0;'>"
        "<tr>"
        "  <td width='36' valign='top' align='center'"
        "      style='padding-top:2px; padding-right:6px;'>"
        "    <table cellspacing='0' cellpadding='0'><tr><td width='32' height='32'"
        "      align='center' valign='middle'"
        "      style='background:#1e2a3a; border-radius:16px;"
        "             color:#8dc8f8; font-size:11px; font-weight:bold;'>"
        "      AI"
        "    </td></tr></table>"
        "  </td>"
        "  <td width='1' valign='top'>"
        "    <table cellspacing='0' cellpadding='0'><tr><td"
        "      style='background:#ffffff; border:1px solid #dde6f0;"
        "             border-radius:2px 14px 14px 14px;"
        "             padding:10px 15px;"
        "             font-size:13px; line-height:1.7;'>"
        "      %1"
        "    </td></tr></table>"
        "    <p align='left'"
        "       style='margin:2px 0 0 0; font-size:10px; color:#b0bcc8;'>"
        "      助手 · %2"
        "    </p>"
        "  </td>"
        "  <td></td>"  // 右侧空白
        "</tr>"
        "</table>"
    ).arg(replyHtml, timeStr);

    m_messageView->append(wrapper);
    m_messageView->verticalScrollBar()->setValue(
        m_messageView->verticalScrollBar()->maximum());
}

void ChatWidget::appendSystemText(const QString& text)
{
    // 简单处理 Markdown 粗体和换行
    QString escaped = text.toHtmlEscaped()
                          .replace("\n\n", "</p><p style='margin:3px 0;'>")
                          .replace("\n", "<br/>");

    QString html = QString(
        "<div style='text-align: center; margin: 14px 8px 8px 8px;'>"
        "  <div style='display: inline-block;"
        "    background: #eef4fc;"
        "    color: #7a90aa;"
        "    border: 1px solid #d4e4f4;"
        "    border-radius: 8px;"
        "    padding: 10px 18px;"
        "    font-size: 12px; line-height: 1.7;"
        "    max-width: 88%; text-align: left;'>"
        "    <p style='margin: 0;'>%1</p>"
        "  </div>"
        "</div>"
    ).arg(escaped);

    m_messageView->append(html);
}

void ChatWidget::appendErrorText(const QString& text)
{
    QString html = QString(
        "<div style='margin: 8px 0;'>"
        "  <div style='background: #fff2f0; color: #d9363e;"
        "    border: 1px solid #ffc8c5;"
        "    border-radius: 8px; padding: 9px 14px; font-size: 13px;'>"
        "    ⚠️  %1"
        "  </div>"
        "</div>"
    ).arg(text.toHtmlEscaped());

    m_messageView->append(html);
}
