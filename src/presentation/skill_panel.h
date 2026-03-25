#ifndef SKILL_PANEL_H
#define SKILL_PANEL_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QTextBrowser>
#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QStackedWidget>
#include <QTableWidget>
#include <memory>

namespace application { class SkillAppService; }

// ================================================================
// 对话式创建技能向导对话框
// ================================================================
class SkillCreateWizard : public QDialog {
    Q_OBJECT
public:
    explicit SkillCreateWizard(QWidget* parent = nullptr);

    // 获取填写结果
    QString skillId()   const;
    QString skillName() const;
    QString skillDesc() const;
    QString skillVersion() const;
    QString skillAuthor() const;

    // 参数列表：[[name, type, required(0/1), desc], ...]
    QList<QStringList> params() const;

private slots:
    void onNext();
    void onBack();
    void onAddParam();
    void onRemoveParam();

private:
    void setupUI();
    void updateButtons();

    QStackedWidget* m_stack    = nullptr;
    QPushButton*    m_nextBtn  = nullptr;
    QPushButton*    m_backBtn  = nullptr;
    QLabel*         m_stepLabel = nullptr;

    // Step 1 - 基本信息
    QLineEdit* m_idEdit      = nullptr;
    QLineEdit* m_nameEdit    = nullptr;
    QTextEdit* m_descEdit    = nullptr;
    QLineEdit* m_versionEdit = nullptr;
    QLineEdit* m_authorEdit  = nullptr;

    // Step 2 - 参数列表
    QTableWidget* m_paramTable = nullptr;
};

// ================================================================
// Skill 管理面板
// ================================================================
class SkillPanel : public QWidget {
    Q_OBJECT

public:
    explicit SkillPanel(QWidget* parent = nullptr);
    ~SkillPanel() = default;

    void setService(std::shared_ptr<application::SkillAppService> service);

public slots:
    void refresh();

private slots:
    void onLoadFromDir();
    void onCreateByChat();
    void onSkillSelected(QListWidgetItem* item);

private:
    void setupUI();
    void showSkillDetail(const std::string& skillId);

    std::shared_ptr<application::SkillAppService> m_service;

    QListWidget*  m_skillList   = nullptr;
    QTextBrowser* m_detailView  = nullptr;
    QPushButton*  m_loadBtn     = nullptr;
    QPushButton*  m_createBtn   = nullptr;
    QLabel*       m_countLabel  = nullptr;
};

#endif // SKILL_PANEL_H
