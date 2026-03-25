#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QLabel>
#include <memory>

// 前向声明
namespace application {
class ChatAppService;
class KBAppService;
class SkillAppService;
}

class ChatWidget;
class SkillPanel;
class KBPanel;

/**
 * @brief 主窗口
 *
 * 布局：左侧固定宽度导航栏 + 右侧 QStackedWidget 内容区
 * 不使用菜单栏，导航全部集中在侧边栏
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void switchToPage(int index);

private:
    void setupUI();
    void setupServices();
    void setupConnections();

    // 应用服务
    std::shared_ptr<application::ChatAppService>  m_chatService;
    std::shared_ptr<application::KBAppService>    m_kbService;
    std::shared_ptr<application::SkillAppService> m_skillService;

    // UI 面板
    ChatWidget*     m_chatWidget  = nullptr;
    SkillPanel*     m_skillPanel  = nullptr;
    KBPanel*        m_kbPanel     = nullptr;

    // 导航
    QStackedWidget* m_stack       = nullptr;
    QButtonGroup*   m_navGroup    = nullptr;
    QLabel*         m_footerLabel = nullptr;

    int m_currentPage = 0;
};

#endif // MAIN_WINDOW_H
