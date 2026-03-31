#include "main_window.h"
#include "chat_widget.h"
#include "skill_panel.h"
#include "kb_panel.h"
#include "../application/services/chat_app_service.h"
#include "../application/services/kb_app_service.h"
#include "../application/services/skill_app_service.h"
#include "../domain/chat/models/chat_session.h"
#include "../domain/chat/services/chat_processor.h"
#include "../domain/skill/models/skill_registry.h"
#include "../domain/skill/services/skill_executor.h"
#include "../infrastructure/rendering/simple_markdown_renderer.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QToolButton>
#include <QButtonGroup>
#include <QLabel>
#include <QFrame>
#include <QMessageBox>
#include <QDir>
#include <QMenuBar>
#include <QStatusBar>
#include <QFont>

// ================================================================
// 导航按钮样式常量
// ================================================================
static const char* NAV_BTN_STYLE = R"(
QToolButton {
    border: none;
    border-radius: 8px;
    padding: 10px 4px 6px 4px;
    color: #8a9bb0;
    background: transparent;
    font-size: 11px;
    text-align: center;
    min-width: 60px;
    min-height: 52px;
}
QToolButton:hover {
    color: #4A90D9;
    background: rgba(74,144,217,0.08);
}
QToolButton:checked {
    color: #4A90D9;
    background: rgba(74,144,217,0.12);
    font-weight: bold;
}
)";

// ================================================================
// 创建一个导航按钮（图标文字上下排列）
// ================================================================
static QToolButton* makeNavBtn(const QString& icon, const QString& text, QWidget* parent)
{
    auto* btn = new QToolButton(parent);
    btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    // 用 Unicode 字符作为"图标"，通过字体渲染
    btn->setText(QString("%1\n%2").arg(icon, text));
    btn->setCheckable(true);
    btn->setStyleSheet(NAV_BTN_STYLE);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    return btn;
}

// ================================================================
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("工程图纸设计助手");
    resize(1200, 760);
    setMinimumSize(860, 520);
    menuBar()->hide();   // 不用菜单栏

    setupServices();
    setupUI();
    setupConnections();

    statusBar()->setStyleSheet(
        "QStatusBar { background: #f8f9fb; color: #8a9bb0; font-size: 11px;"
        " border-top: 1px solid #e8ecf0; }");
    statusBar()->showMessage("就绪");
}

MainWindow::~MainWindow() = default;

// ----------------------------------------------------------------
void MainWindow::setupServices()
{
    QString baseDir = QApplication::applicationDirPath();
    std::string kbDir    = (baseDir + "/data/knowledge").toStdString();
    std::string skillDir = (baseDir + "/data/skills").toStdString();

    auto renderer      = std::make_shared<infrastructure::rendering::SimpleMarkdownRenderer>();
    auto skillRegistry = std::make_shared<domain::skill::SkillRegistry>();
    auto skillExecutor = std::make_shared<domain::skill::SkillExecutor>(skillRegistry);

    m_skillService = std::make_shared<application::SkillAppService>(skillDir, skillRegistry);
    m_skillService->loadSkillsFromDir();

    m_kbService   = std::make_shared<application::KBAppService>(kbDir);

    auto session   = std::make_shared<domain::chat::ChatSession>(
        domain::chat::SessionId("default"));
    auto processor = std::make_shared<domain::chat::ChatProcessor>();
    m_chatService  = std::make_shared<application::ChatAppService>(
        session, processor, skillExecutor, renderer);
}

// ----------------------------------------------------------------
void MainWindow::setupUI()
{
    // ===== 根容器 =====
    auto* root = new QWidget(this);
    setCentralWidget(root);
    auto* rootLayout = new QHBoxLayout(root);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ===== 左侧导航栏 =====
    auto* sidebar = new QWidget(root);
    sidebar->setFixedWidth(72);
    sidebar->setStyleSheet(
        "QWidget { background: #1e2a3a; }"
    );
    auto* sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(6, 12, 6, 12);
    sideLayout->setSpacing(4);

    // Logo / 应用名
    auto* logoLabel = new QLabel("⚙️", sidebar);
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setStyleSheet(
        "QLabel { color: white; font-size: 22px; padding: 8px 0 4px 0; background: transparent; }");
    sideLayout->addWidget(logoLabel);

    auto* appNameLabel = new QLabel("助手", sidebar);
    appNameLabel->setAlignment(Qt::AlignCenter);
    appNameLabel->setStyleSheet(
        "QLabel { color: #6b8cae; font-size: 10px; background: transparent; margin-bottom: 12px; }");
    sideLayout->addWidget(appNameLabel);

    // 分隔线
    auto* topSep = new QFrame(sidebar);
    topSep->setFrameShape(QFrame::HLine);
    topSep->setStyleSheet("QFrame { background: #2d3f54; border: none; min-height: 1px; max-height: 1px; }");
    sideLayout->addWidget(topSep);
    sideLayout->addSpacing(8);

    // 导航按钮
    m_navGroup = new QButtonGroup(this);
    m_navGroup->setExclusive(true);

    auto* chatBtn  = makeNavBtn("💬", "聊天", sidebar);
    auto* skillBtn = makeNavBtn("🔧", "技能", sidebar);
    auto* kbBtn    = makeNavBtn("📚", "知识库", sidebar);
    chatBtn->setChecked(true);

    // 覆盖按钮的颜色主题适配深色背景
    auto applyDarkNav = [](QToolButton* btn) {
        btn->setStyleSheet(
            "QToolButton {"
            "  border: none; border-radius: 8px; padding: 8px 2px 5px 2px;"
            "  color: #6b8cae; background: transparent;"
            "  font-size: 11px; text-align: center;"
            "  min-width: 58px; min-height: 50px;"
            "}"
            "QToolButton:hover {"
            "  color: #9ec4e8; background: rgba(255,255,255,0.07);"
            "}"
            "QToolButton:checked {"
            "  color: #ffffff; background: rgba(74,144,217,0.30);"
            "  font-weight: bold;"
            "}"
        );
    };
    applyDarkNav(chatBtn);
    applyDarkNav(skillBtn);
    applyDarkNav(kbBtn);

    m_navGroup->addButton(chatBtn,  0);
    m_navGroup->addButton(skillBtn, 1);
    m_navGroup->addButton(kbBtn,    2);
    sideLayout->addWidget(chatBtn);
    sideLayout->addWidget(skillBtn);
    sideLayout->addWidget(kbBtn);

    sideLayout->addStretch(1);

    // 底部：关于按钮
    auto* aboutBtn = new QToolButton(sidebar);
    aboutBtn->setText("ℹ️\n关于");
    aboutBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    aboutBtn->setStyleSheet(
        "QToolButton {"
        "  border: none; border-radius: 8px; padding: 8px 2px 5px 2px;"
        "  color: #4a5f75; background: transparent;"
        "  font-size: 10px; text-align: center;"
        "  min-width: 58px; min-height: 48px;"
        "}"
        "QToolButton:hover { color: #6b8cae; background: rgba(255,255,255,0.05); }"
    );
    connect(aboutBtn, &QToolButton::clicked, this, [this](){
        QMessageBox::about(this, "关于工程图纸设计助手",
            "<h3>⚙️ 工程图纸设计助手</h3>"
            "<p><b>版本:</b> 1.0.0</p>"
            "<p><b>构建:</b> Qt 5.15.2 + C++14</p>"
            "<p><b>架构:</b> DDD 领域驱动设计</p>"
            "<hr/>"
            "<p style='color:#888;'>支持 RAG 知识检索增强 · Skill 插件扩展</p>");
    });
    sideLayout->addWidget(aboutBtn);

    rootLayout->addWidget(sidebar);

    // ===== 细分隔线 =====
    auto* vLine = new QFrame(root);
    vLine->setFrameShape(QFrame::VLine);
    vLine->setStyleSheet("QFrame { background: #e2e8f0; border: none; min-width: 1px; max-width: 1px; }");
    rootLayout->addWidget(vLine);

    // ===== 右侧内容区 =====
    m_stack = new QStackedWidget(root);
    m_stack->setStyleSheet("QStackedWidget { background: #f7f8fa; }");

    m_chatWidget = new ChatWidget(m_stack);
    m_chatWidget->setService(m_chatService);
    m_stack->addWidget(m_chatWidget);   // index 0

    m_skillPanel = new SkillPanel(m_stack);
    m_skillPanel->setService(m_skillService);
    m_stack->addWidget(m_skillPanel);   // index 1

    m_kbPanel = new KBPanel(m_stack);
    m_kbPanel->setService(m_kbService);
    m_stack->addWidget(m_kbPanel);      // index 2

    rootLayout->addWidget(m_stack, 1);

    // 绑定导航点击
    connect(m_navGroup, QOverload<int>::of(&QButtonGroup::idPressed),
            this, &MainWindow::switchToPage);
}

// ----------------------------------------------------------------
void MainWindow::setupConnections()
{
    connect(m_kbPanel, &KBPanel::knowledgeBaseChanged, this, [this](){
        auto kb = m_kbService->currentKnowledgeBase();   // 只调用一次
        m_chatService->attachKnowledgeBase(kb);
        if (kb) {
            statusBar()->showMessage(
                QString("✅  知识库已加载: %1").arg(
                    QString::fromStdString(kb->getName())));
        } else {
            statusBar()->showMessage("知识库已卸载");
        }
    });
}

// ----------------------------------------------------------------
void MainWindow::switchToPage(int index)
{
    if (m_currentPage == index) return;
    m_currentPage = index;
    m_stack->setCurrentIndex(index);

    if (index == 1 && m_skillPanel) m_skillPanel->refresh();
    if (index == 2 && m_kbPanel)    m_kbPanel->refresh();

    QStringList names = {"聊天", "Skill 管理", "知识库管理"};
    statusBar()->showMessage(names.value(index));
}
