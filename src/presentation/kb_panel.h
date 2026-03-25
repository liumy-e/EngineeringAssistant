#ifndef KB_PANEL_H
#define KB_PANEL_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextBrowser>
#include <QProgressBar>
#include <memory>

namespace application { class KBAppService; }

/**
 * @brief 知识库管理面板
 *
 * 显示可用的知识库列表，支持加载/卸载、关键词搜索和网址导入。
 */
class KBPanel : public QWidget {
    Q_OBJECT

public:
    explicit KBPanel(QWidget* parent = nullptr);
    ~KBPanel() = default;

    void setService(std::shared_ptr<application::KBAppService> service);
    void refresh();

signals:
    void knowledgeBaseChanged();

private slots:
    void onLoadKB();
    void onUnloadKB();
    void onSearch();
    void onKBSelected(QListWidgetItem* item);
    void onImportUrl();

private:
    void setupUI();

    std::shared_ptr<application::KBAppService> m_service;

    QListWidget*  m_kbList      = nullptr;
    QTextBrowser* m_resultView  = nullptr;
    QLineEdit*    m_searchEdit  = nullptr;
    QPushButton*  m_loadBtn     = nullptr;
    QPushButton*  m_unloadBtn   = nullptr;
    QPushButton*  m_importUrlBtn = nullptr;
    QLabel*       m_statusLbl   = nullptr;
};

#endif // KB_PANEL_H
