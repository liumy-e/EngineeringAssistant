#ifndef CHAT_WIDGET_H
#define CHAT_WIDGET_H

#include <QWidget>
#include <QTextBrowser>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QScrollBar>
#include <memory>
#include <functional>

namespace application { class ChatAppService; }

/**
 * @brief 聊天面板
 *
 * 显示消息历史（支持 HTML/Markdown 渲染）和输入框。
 */
class ChatWidget : public QWidget {
    Q_OBJECT

public:
    explicit ChatWidget(QWidget* parent = nullptr);
    ~ChatWidget() = default;

    /**
     * @brief 绑定聊天应用服务
     */
    void setService(std::shared_ptr<application::ChatAppService> service);

signals:
    void messageSent(const QString& text);

private slots:
    void onSend();
    void onClear();
    void onKeyboardSend();

private:
    void setupUI();
    void appendBubble(const QString& role, const QString& htmlContent, const QString& color);
    void appendHtmlReply(const QString& html);
    void appendUserText(const QString& text);
    void appendSystemText(const QString& text);
    void appendErrorText(const QString& text);

    std::shared_ptr<application::ChatAppService> m_service;

    QTextBrowser* m_messageView = nullptr;
    QTextEdit*    m_inputEdit   = nullptr;
    QPushButton*  m_sendBtn     = nullptr;
    QPushButton*  m_clearBtn    = nullptr;
    QLabel*       m_statusLabel = nullptr;
};

#endif // CHAT_WIDGET_H
