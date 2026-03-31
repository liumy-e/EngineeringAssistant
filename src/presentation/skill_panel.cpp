#include "skill_panel.h"
#include "../application/services/skill_app_service.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QSplitter>
#include <QDebug>
#include <QScrollArea>
#include <QHeaderView>
#include <QComboBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFrame>
#include <QFormLayout>
#include <QCoreApplication>

// ====================================================================
// SkillCreateWizard 实现
// ====================================================================

SkillCreateWizard::SkillCreateWizard(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("✨ 对话创建技能");
    setMinimumSize(520, 420);
    setupUI();
}

void SkillCreateWizard::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ---------- 顶部进度条标题 ----------
    auto* header = new QWidget(this);
    header->setFixedHeight(52);
    header->setStyleSheet("QWidget { background: #1e2a3a; }");
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(20, 0, 20, 0);

    auto* iconLabel = new QLabel("✨", header);
    iconLabel->setStyleSheet("QLabel { color: #8dc8f8; font-size: 18px; background: transparent; }");
    headerLayout->addWidget(iconLabel);

    auto* titleLabel = new QLabel("创建新技能", header);
    titleLabel->setStyleSheet("QLabel { color: #e8f2fc; font-size: 14px; font-weight: bold; background: transparent; }");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch(1);

    m_stepLabel = new QLabel("第 1 步 / 共 2 步", header);
    m_stepLabel->setStyleSheet("QLabel { color: #6a8aaa; font-size: 12px; background: transparent; }");
    headerLayout->addWidget(m_stepLabel);

    mainLayout->addWidget(header);

    // ---------- 内容区（堆叠） ----------
    m_stack = new QStackedWidget(this);
    m_stack->setStyleSheet("QStackedWidget { background: #f7f8fa; }");

    // --- Step 1：基本信息 ---
    auto* step1 = new QWidget();
    step1->setStyleSheet("QWidget { background: #f7f8fa; }");
    auto* s1Layout = new QVBoxLayout(step1);
    s1Layout->setContentsMargins(28, 24, 28, 16);
    s1Layout->setSpacing(0);

    auto* s1Title = new QLabel("📋  填写技能基本信息", step1);
    s1Title->setStyleSheet("QLabel { font-size: 13px; font-weight: bold; color: #1a2537; background: transparent; margin-bottom: 4px; }");
    s1Layout->addWidget(s1Title);

    auto* s1Hint = new QLabel("技能 ID 用于在聊天中调用（如 @my-skill），建议使用英文+短横线", step1);
    s1Hint->setStyleSheet("QLabel { font-size: 11px; color: #8a9bb0; background: transparent; margin-bottom: 16px; }");
    s1Hint->setWordWrap(true);
    s1Layout->addWidget(s1Hint);

    auto* formWidget = new QWidget(step1);
    auto* form = new QFormLayout(formWidget);
    form->setContentsMargins(0, 0, 0, 0);
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto fieldStyle = QString(
        "QLineEdit, QTextEdit {"
        "  border: 1.5px solid #dde3ec; border-radius: 6px;"
        "  padding: 6px 10px; font-size: 13px; color: #1a2537; background: #ffffff;"
        "}"
        "QLineEdit:focus, QTextEdit:focus { border-color: #4A90D9; }"
    );
    auto labelStyle = QString("QLabel { color: #4a5f75; font-size: 12px; background: transparent; }");

    m_idEdit = new QLineEdit(formWidget);
    m_idEdit->setPlaceholderText("如 my-skill（英文，不含空格）");
    m_idEdit->setStyleSheet(fieldStyle);
    auto* idLabel = new QLabel("技能 ID *", formWidget);
    idLabel->setStyleSheet(labelStyle);
    form->addRow(idLabel, m_idEdit);

    m_nameEdit = new QLineEdit(formWidget);
    m_nameEdit->setPlaceholderText("如 我的自定义技能");
    m_nameEdit->setStyleSheet(fieldStyle);
    auto* nameLabel = new QLabel("技能名称 *", formWidget);
    nameLabel->setStyleSheet(labelStyle);
    form->addRow(nameLabel, m_nameEdit);

    m_descEdit = new QTextEdit(formWidget);
    m_descEdit->setPlaceholderText("描述这个技能的功能...");
    m_descEdit->setFixedHeight(72);
    m_descEdit->setStyleSheet(fieldStyle);
    auto* descLabel = new QLabel("描述", formWidget);
    descLabel->setStyleSheet(labelStyle);
    form->addRow(descLabel, m_descEdit);

    m_versionEdit = new QLineEdit(formWidget);
    m_versionEdit->setText("1.0.0");
    m_versionEdit->setStyleSheet(fieldStyle);
    auto* verLabel = new QLabel("版本号", formWidget);
    verLabel->setStyleSheet(labelStyle);
    form->addRow(verLabel, m_versionEdit);

    m_authorEdit = new QLineEdit(formWidget);
    m_authorEdit->setPlaceholderText("可选");
    m_authorEdit->setStyleSheet(fieldStyle);
    auto* authorLabel = new QLabel("作者", formWidget);
    authorLabel->setStyleSheet(labelStyle);
    form->addRow(authorLabel, m_authorEdit);

    s1Layout->addWidget(formWidget);
    s1Layout->addStretch(1);

    m_stack->addWidget(step1);

    // --- Step 2：参数定义 ---
    auto* step2 = new QWidget();
    step2->setStyleSheet("QWidget { background: #f7f8fa; }");
    auto* s2Layout = new QVBoxLayout(step2);
    s2Layout->setContentsMargins(28, 24, 28, 16);
    s2Layout->setSpacing(8);

    auto* s2Title = new QLabel("⚙️  定义技能参数（可选）", step2);
    s2Title->setStyleSheet("QLabel { font-size: 13px; font-weight: bold; color: #1a2537; background: transparent; }");
    s2Layout->addWidget(s2Title);

    auto* s2Hint = new QLabel("参数将在调用技能时传入，如 @my-skill param1=值。不需要参数可以直接点「完成」", step2);
    s2Hint->setStyleSheet("QLabel { font-size: 11px; color: #8a9bb0; background: transparent; }");
    s2Hint->setWordWrap(true);
    s2Layout->addWidget(s2Hint);

    // 参数操作按钮行
    auto* paramBtnRow = new QHBoxLayout();
    paramBtnRow->setSpacing(8);

    auto* addParamBtn = new QPushButton("+ 添加参数", step2);
    addParamBtn->setFixedHeight(28);
    addParamBtn->setStyleSheet(
        "QPushButton { border: 1.5px solid #4A90D9; border-radius: 5px;"
        " background: #fff; color: #4A90D9; font-size: 12px; font-weight: bold; padding: 0 12px; }"
        "QPushButton:hover { background: #eef5fd; }"
    );
    connect(addParamBtn, &QPushButton::clicked, this, &SkillCreateWizard::onAddParam);
    paramBtnRow->addWidget(addParamBtn);

    auto* removeParamBtn = new QPushButton("- 删除选中", step2);
    removeParamBtn->setFixedHeight(28);
    removeParamBtn->setStyleSheet(
        "QPushButton { border: 1.5px solid #e88c8c; border-radius: 5px;"
        " background: #fff; color: #cc4444; font-size: 12px; padding: 0 12px; }"
        "QPushButton:hover { background: #fff5f5; }"
    );
    connect(removeParamBtn, &QPushButton::clicked, this, &SkillCreateWizard::onRemoveParam);
    paramBtnRow->addWidget(removeParamBtn);
    paramBtnRow->addStretch(1);

    s2Layout->addLayout(paramBtnRow);

    // 参数表格
    m_paramTable = new QTableWidget(0, 4, step2);
    m_paramTable->setHorizontalHeaderLabels({"参数名", "类型", "必填", "描述"});
    m_paramTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_paramTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_paramTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    m_paramTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_paramTable->setColumnWidth(0, 110);
    m_paramTable->setColumnWidth(1, 80);
    m_paramTable->setColumnWidth(2, 55);
    m_paramTable->setFrameShape(QFrame::NoFrame);
    m_paramTable->setAlternatingRowColors(true);
    m_paramTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_paramTable->setStyleSheet(
        "QTableWidget { border: 1px solid #dde3ec; border-radius: 6px;"
        " background: #ffffff; font-size: 12px; gridline-color: #eef2f8; }"
        "QHeaderView::section { background: #f4f6fa; color: #4a5f75;"
        " font-size: 11px; font-weight: bold; padding: 5px 8px;"
        " border: none; border-bottom: 1px solid #dde3ec; }"
        "QTableWidget::item { padding: 4px 8px; }"
        "QTableWidget::item:alternate { background: #f9fafc; }"
        "QTableWidget::item:selected { background: #e6f2ff; color: #1a6fc4; }"
    );
    s2Layout->addWidget(m_paramTable, 1);

    m_stack->addWidget(step2);

    mainLayout->addWidget(m_stack, 1);

    // ---------- 底部按钮区 ----------
    auto* footer = new QWidget(this);
    footer->setFixedHeight(52);
    footer->setStyleSheet(
        "QWidget { background: #ffffff; border-top: 1px solid #e8ecf0; }");
    auto* footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(20, 0, 20, 0);
    footerLayout->setSpacing(8);

    auto* cancelBtn = new QPushButton("取消", footer);
    cancelBtn->setFixedSize(72, 32);
    cancelBtn->setStyleSheet(
        "QPushButton { border: 1px solid #dde3ec; border-radius: 6px;"
        " background: #f7f8fa; color: #8a9bb0; font-size: 13px; }"
        "QPushButton:hover { background: #eff2f7; color: #4A90D9; }"
    );
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    footerLayout->addWidget(cancelBtn);
    footerLayout->addStretch(1);

    m_backBtn = new QPushButton("◀ 上一步", footer);
    m_backBtn->setFixedSize(88, 32);
    m_backBtn->setEnabled(false);
    m_backBtn->setStyleSheet(
        "QPushButton { border: 1.5px solid #dde3ec; border-radius: 6px;"
        " background: #ffffff; color: #4a5f75; font-size: 13px; }"
        "QPushButton:hover { border-color: #4A90D9; color: #4A90D9; }"
        "QPushButton:disabled { color: #c8d4e0; border-color: #eef2f8; }"
    );
    connect(m_backBtn, &QPushButton::clicked, this, &SkillCreateWizard::onBack);
    footerLayout->addWidget(m_backBtn);

    m_nextBtn = new QPushButton("下一步 ▶", footer);
    m_nextBtn->setFixedSize(100, 32);
    m_nextBtn->setStyleSheet(
        "QPushButton { border: none; border-radius: 6px;"
        " background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
        "     stop:0 #5ca0e8, stop:1 #3d82d0);"
        " color: white; font-size: 13px; font-weight: bold; }"
        "QPushButton:hover { background: #4d90e0; }"
        "QPushButton:pressed { background: #2d72c0; }"
    );
    connect(m_nextBtn, &QPushButton::clicked, this, &SkillCreateWizard::onNext);
    footerLayout->addWidget(m_nextBtn);

    mainLayout->addWidget(footer);
}

void SkillCreateWizard::onNext()
{
    int cur = m_stack->currentIndex();
    if (cur == 0) {
        // 校验基本信息
        if (m_idEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "提示", "技能 ID 不能为空！");
            m_idEdit->setFocus();
            return;
        }
        if (m_nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "提示", "技能名称不能为空！");
            m_nameEdit->setFocus();
            return;
        }
        m_stack->setCurrentIndex(1);
        m_stepLabel->setText("第 2 步 / 共 2 步");
        m_backBtn->setEnabled(true);
        m_nextBtn->setText("✅  完成");
    } else {
        // 第二步，接受
        accept();
    }
}

void SkillCreateWizard::onBack()
{
    m_stack->setCurrentIndex(0);
    m_stepLabel->setText("第 1 步 / 共 2 步");
    m_backBtn->setEnabled(false);
    m_nextBtn->setText("下一步 ▶");
}

void SkillCreateWizard::onAddParam()
{
    int row = m_paramTable->rowCount();
    m_paramTable->insertRow(row);

    m_paramTable->setItem(row, 0, new QTableWidgetItem("param_name"));

    auto* typeCombo = new QComboBox(m_paramTable);
    typeCombo->addItems({"string", "number", "boolean", "file"});
    typeCombo->setStyleSheet(
        "QComboBox { border: 1px solid #dde3ec; border-radius: 4px;"
        " padding: 2px 6px; font-size: 12px; background: #fff; }"
    );
    m_paramTable->setCellWidget(row, 1, typeCombo);

    auto* reqCheck = new QCheckBox(m_paramTable);
    reqCheck->setChecked(true);
    reqCheck->setStyleSheet("QCheckBox { margin-left: 12px; }");
    m_paramTable->setCellWidget(row, 2, reqCheck);

    m_paramTable->setItem(row, 3, new QTableWidgetItem("参数说明"));
    m_paramTable->setRowHeight(row, 32);
}

void SkillCreateWizard::onRemoveParam()
{
    auto selected = m_paramTable->selectedItems();
    if (selected.isEmpty()) return;
    int row = selected.first()->row();
    m_paramTable->removeRow(row);
}

void SkillCreateWizard::updateButtons() {}

QString SkillCreateWizard::skillId()      const { return m_idEdit->text().trimmed(); }
QString SkillCreateWizard::skillName()    const { return m_nameEdit->text().trimmed(); }
QString SkillCreateWizard::skillDesc()    const { return m_descEdit->toPlainText().trimmed(); }
QString SkillCreateWizard::skillVersion() const { return m_versionEdit->text().trimmed(); }
QString SkillCreateWizard::skillAuthor()  const { return m_authorEdit->text().trimmed(); }

QList<QStringList> SkillCreateWizard::params() const
{
    QList<QStringList> result;
    for (int r = 0; r < m_paramTable->rowCount(); ++r) {
        QString name = m_paramTable->item(r, 0)
                       ? m_paramTable->item(r, 0)->text().trimmed() : "";
        auto* combo = qobject_cast<QComboBox*>(m_paramTable->cellWidget(r, 1));
        QString type = combo ? combo->currentText() : "string";
        auto* chk = qobject_cast<QCheckBox*>(m_paramTable->cellWidget(r, 2));
        QString req  = (chk && chk->isChecked()) ? "1" : "0";
        QString desc = m_paramTable->item(r, 3)
                       ? m_paramTable->item(r, 3)->text().trimmed() : "";
        if (!name.isEmpty())
            result.append({name, type, req, desc});
    }
    return result;
}

// ====================================================================
// SkillPanel 实现
// ====================================================================

SkillPanel::SkillPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void SkillPanel::setService(std::shared_ptr<application::SkillAppService> service)
{
    m_service = std::move(service);
    refresh();
}

void SkillPanel::setupUI()
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

    auto* titleLabel = new QLabel("🔧  技能管理", titleBar);
    titleLabel->setStyleSheet(
        "QLabel { font-size: 14px; font-weight: bold; color: #1a2537;"
        " background: transparent; border: none; }");
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch(1);

    m_countLabel = new QLabel("0 个技能", titleBar);
    m_countLabel->setStyleSheet(
        "QLabel { color: #8a9bb0; font-size: 11px;"
        " background: #f0f5fb; border: 1px solid #dde8f5;"
        " border-radius: 10px; padding: 2px 10px; }");
    titleLayout->addWidget(m_countLabel);
    titleLayout->addSpacing(8);

    // 对话创建按钮
    m_createBtn = new QPushButton("✨ 对话创建", titleBar);
    m_createBtn->setFixedHeight(28);
    m_createBtn->setStyleSheet(
        "QPushButton {"
        "  border: 1.5px solid #e8a020; border-radius: 5px;"
        "  background: #fffbf0; color: #c87010; font-size: 12px; font-weight: bold;"
        "  padding: 0 10px;"
        "}"
        "QPushButton:hover { background: #fff3d0; }"
        "QPushButton:pressed { background: #ffe8a8; }"
    );
    connect(m_createBtn, &QPushButton::clicked, this, &SkillPanel::onCreateByChat);
    titleLayout->addWidget(m_createBtn);
    titleLayout->addSpacing(6);

    // 加载目录按钮
    m_loadBtn = new QPushButton("+ 加载", titleBar);
    m_loadBtn->setFixedSize(70, 28);
    m_loadBtn->setStyleSheet(
        "QPushButton {"
        "  border: 1.5px solid #4A90D9; border-radius: 5px;"
        "  background: #ffffff; color: #4A90D9; font-size: 12px; font-weight: bold;"
        "}"
        "QPushButton:hover { background: #eef5fd; }"
        "QPushButton:pressed { background: #dceefb; }"
    );
    connect(m_loadBtn, &QPushButton::clicked, this, &SkillPanel::onLoadFromDir);
    titleLayout->addWidget(m_loadBtn);

    layout->addWidget(titleBar);

    // ---- 内容区：左右分割 ----
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(1);
    splitter->setStyleSheet(
        "QSplitter::handle { background: #e8ecf0; }"
    );

    // 左侧：技能列表
    auto* listContainer = new QWidget(splitter);
    listContainer->setStyleSheet("QWidget { background: #f7f8fa; }");
    auto* listLayout = new QVBoxLayout(listContainer);
    listLayout->setContentsMargins(12, 12, 0, 12);
    listLayout->setSpacing(6);

    auto* listTitle = new QLabel("已安装技能", listContainer);
    listTitle->setStyleSheet(
        "QLabel { font-size: 11px; color: #8a9bb0; font-weight: bold;"
        " text-transform: uppercase; background: transparent; }");
    listLayout->addWidget(listTitle);

    m_skillList = new QListWidget(listContainer);
    m_skillList->setFrameShape(QFrame::NoFrame);
    m_skillList->setSpacing(2);
    m_skillList->setStyleSheet(
        "QListWidget { background: transparent; border: none; outline: none; }"
        "QListWidget::item {"
        "  background: #ffffff;"
        "  border: 1px solid #e8ecf0;"
        "  border-radius: 6px;"
        "  padding: 8px 10px;"
        "  margin: 0 0 3px 0;"
        "  color: #2c3e55;"
        "  font-size: 13px;"
        "}"
        "QListWidget::item:hover {"
        "  background: #f0f7ff;"
        "  border-color: #b8d4f0;"
        "}"
        "QListWidget::item:selected {"
        "  background: #e6f2ff;"
        "  border-color: #4A90D9;"
        "  color: #1a6fc4;"
        "}"
    );
    connect(m_skillList, &QListWidget::itemClicked, this, &SkillPanel::onSkillSelected);
    listLayout->addWidget(m_skillList);

    splitter->addWidget(listContainer);

    // 右侧：技能详情
    auto* detailContainer = new QWidget(splitter);
    detailContainer->setStyleSheet("QWidget { background: #ffffff; }");
    auto* detailLayout = new QVBoxLayout(detailContainer);
    detailLayout->setContentsMargins(0, 0, 0, 0);

    m_detailView = new QTextBrowser(detailContainer);
    m_detailView->setFrameShape(QFrame::NoFrame);
    m_detailView->setOpenExternalLinks(true);
    m_detailView->setStyleSheet(
        "QTextBrowser { background: #ffffff; border: none; padding: 20px 24px; }"
        "QScrollBar:vertical { background: transparent; width: 6px; }"
        "QScrollBar::handle:vertical { background: #c8d4e0; border-radius: 3px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    );
    m_detailView->setHtml(
        "<div style='text-align:center; padding: 60px 20px; color: #b0bcc8;'>"
        "<div style='font-size: 36px; margin-bottom: 12px;'>🔧</div>"
        "<div style='font-size: 13px;'>从左侧选择一个技能查看详情<br/>"
        "<span style='font-size:11px;'>或点击「✨ 对话创建」快速创建新技能</span></div>"
        "</div>"
    );
    detailLayout->addWidget(m_detailView);

    splitter->addWidget(detailContainer);
    splitter->setSizes({220, 500});

    layout->addWidget(splitter, 1);
}

void SkillPanel::refresh()
{
    if (!m_service) return;

    m_skillList->clear();
    auto metas = m_service->listSkills();
    for (auto& meta : metas) {
        auto* item = new QListWidgetItem(m_skillList);
        QString icon = QString::fromStdString(meta.scriptPath) == "builtin" ? "⚡" : "📦";
        item->setText(QString("%1  %2").arg(icon).arg(QString::fromStdString(meta.name)));
        item->setData(Qt::UserRole, QString::fromStdString(meta.id.value));
        item->setToolTip(QString::fromStdString(meta.description));
    }

    int cnt = static_cast<int>(metas.size());
    m_countLabel->setText(QString("%1 个技能").arg(cnt));
}

void SkillPanel::onLoadFromDir()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, "选择技能目录", QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    if (dir.isEmpty() || !m_service) return;

    bool ok = m_service->loadSkill(dir.toStdString());
    if (ok) {
        refresh();
        m_detailView->setHtml(
            "<div style='padding:20px;'>"
            "<div style='background:#edfaf3; border:1px solid #a8ddc0; border-radius:8px;"
            " padding:12px 16px; font-size:13px; color:#2e8b57;'>"
            "✅  技能加载成功！"
            "</div></div>"
        );
    } else {
        m_detailView->setHtml(
            "<div style='padding:20px; color:#d9363e; background:#fff2f0;"
            " border:1px solid #ffc8c5; border-radius:8px; margin:20px;'>"
            "⚠️ 加载失败，请确认目录中包含有效的 <code>skill.json</code>"
            "</div>"
        );
    }
}

void SkillPanel::onCreateByChat()
{
    if (!m_service) {
        QMessageBox::warning(this, "提示", "请先配置技能服务后再创建技能。");
        return;
    }

    SkillCreateWizard wizard(this);
    if (wizard.exec() != QDialog::Accepted) return;

    // 确定保存目录
    // 默认存到 data/skills/<id>/skill.json
    QString appDir = QCoreApplication::applicationDirPath();
    // 向上找到项目根目录（包含 data/ 的目录）
    QDir dir(appDir);
    QString skillsDir;
    for (int i = 0; i < 5; ++i) {
        if (QDir(dir.absolutePath() + "/data/skills").exists()) {
            skillsDir = dir.absolutePath() + "/data/skills";
            break;
        }
        dir.cdUp();
    }
    if (skillsDir.isEmpty()) {
        // 退化：放到 executable 同级 data/skills
        skillsDir = appDir + "/data/skills";
        QDir().mkpath(skillsDir);
    }

    QString skillId = wizard.skillId();
    QString skillDir = skillsDir + "/" + skillId;
    if (!QDir().mkpath(skillDir)) {
        QMessageBox::critical(this, "错误", QString("无法创建目录：%1").arg(skillDir));
        return;
    }

    // 构建 skill.json
    QJsonObject root;
    root["id"]          = skillId;
    root["name"]        = wizard.skillName();
    root["version"]     = wizard.skillVersion().isEmpty() ? "1.0.0" : wizard.skillVersion();
    root["description"] = wizard.skillDesc();
    root["author"]      = wizard.skillAuthor();
    root["script"]      = QString("builtin");

    QJsonArray paramsArr;
    for (auto& p : wizard.params()) {
        QJsonObject po;
        po["name"]        = p.value(0, "");
        po["type"]        = p.value(1, "string");
        po["required"]    = (p.value(2, "1") == "1");
        po["description"] = p.value(3, "");
        po["default"]     = QString("");
        paramsArr.append(po);
    }
    root["params"] = paramsArr;

    QString jsonPath = skillDir + "/skill.json";
    QFile file(jsonPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", QString("无法写入文件：%1").arg(jsonPath));
        return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();

    // 加载到注册表
    bool ok = m_service->loadSkill(skillDir.toStdString());
    if (ok) {
        refresh();
        m_detailView->setHtml(QString(
            "<div style='padding: 20px;'>"
            "  <div style='font-size:18px; font-weight:bold; color:#1a2537;"
            "    margin-bottom:8px;'>✨  %1</div>"
            "  <span style='background:#e8f3ff; color:#4A90D9; font-size:11px;"
            "    padding:2px 8px; border-radius:10px; font-family:monospace;'>%2</span>"
            "  <div style='background:#edfaf3; border:1px solid #a8ddc0; border-radius:8px;"
            "    padding:12px 16px; font-size:13px; color:#2e8b57; margin-top:16px;'>"
            "    ✅  技能创建成功！在聊天中使用 <code>@%2</code> 调用"
            "  </div>"
            "  <p style='color:#6a8aaa; font-size:12px; margin-top:12px;'>"
            "    📁 文件保存至: %3"
            "  </p>"
            "</div>"
        ).arg(wizard.skillName(), skillId, jsonPath));

        // 选中新创建的技能
        for (int i = 0; i < m_skillList->count(); ++i) {
            if (m_skillList->item(i)->data(Qt::UserRole).toString() == skillId) {
                m_skillList->setCurrentRow(i);
                break;
            }
        }
    } else {
        // skill.json 已写入但加载失败（可能是 builtin 没有对应实现）
        m_detailView->setHtml(QString(
            "<div style='padding: 20px;'>"
            "  <div style='background:#fff8e8; border:1px solid #f0c060; border-radius:8px;"
            "    padding:12px 16px; font-size:13px; color:#b07010;'>"
            "    ⚠️  skill.json 已生成，但当前版本仅支持内置技能执行。"
            "    <br/>文件路径: <code>%1</code>"
            "    <br/><br/>如需实现自定义逻辑，请在 BuiltinSkillFactory 中注册该 ID。"
            "  </div>"
            "</div>"
        ).arg(jsonPath));
        refresh();
    }
}

void SkillPanel::onSkillSelected(QListWidgetItem* item)
{
    if (!item) return;
    showSkillDetail(item->data(Qt::UserRole).toString().toStdString());
}

void SkillPanel::showSkillDetail(const std::string& skillId)
{
    if (!m_service) return;
    // 按 ID 直接查询，O(log N)，避免全量拷贝所有 SkillMeta
    auto metaOpt = m_service->getSkillMeta(skillId);
    if (!metaOpt) return;
    const auto& meta = *metaOpt;

        QString html;
        html += "<div style='padding: 4px 0 16px 0; border-bottom: 1px solid #eef2f8; margin-bottom: 16px;'>";
        html += QString("<div style='font-size:18px; font-weight:bold; color:#1a2537; margin-bottom:4px;'>%1</div>")
                    .arg(QString::fromStdString(meta.name));
        html += QString("<span style='background:#e8f3ff; color:#4A90D9; font-size:11px; padding:2px 8px;"
                        " border-radius:10px; font-family:monospace;'>%1</span>")
                    .arg(QString::fromStdString(meta.id.value));
        html += QString("&nbsp;&nbsp;<span style='color:#b0bcc8; font-size:11px;'>v%1</span>")
                    .arg(QString::fromStdString(meta.version));
        if (meta.scriptPath == "builtin")
            html += "&nbsp;&nbsp;<span style='background:#fff8e8; color:#c87010; font-size:10px;"
                    " padding:1px 7px; border-radius:8px;'>内置</span>";
        html += "</div>";

        html += QString("<p style='color:#4a5f75; font-size:13px; line-height:1.6; margin:0 0 16px 0;'>%1</p>")
                    .arg(QString::fromStdString(meta.description));

        if (!meta.author.empty()) {
            html += QString("<p style='color:#8a9bb0; font-size:12px;'>作者: %1</p>")
                        .arg(QString::fromStdString(meta.author));
        }

        if (!meta.params.empty()) {
            html += "<div style='margin-top:16px;'>";
            html += "<div style='font-size:12px; font-weight:bold; color:#8a9bb0; margin-bottom:8px;'>参数列表</div>";
            html += "<table style='border-collapse:collapse; width:100%; font-size:12px;'>"
                    "<tr style='background:#f4f6fa;'>"
                    "<th style='padding:6px 10px; text-align:left; color:#4a5f75;'>名称</th>"
                    "<th style='padding:6px 10px; text-align:left; color:#4a5f75;'>类型</th>"
                    "<th style='padding:6px 10px; text-align:left; color:#4a5f75;'>必填</th>"
                    "<th style='padding:6px 10px; text-align:left; color:#4a5f75;'>说明</th>"
                    "</tr>";
            bool odd = true;
            for (auto& p : meta.params) {
                QString rowBg = odd ? "#ffffff" : "#f9fafc";
                html += QString("<tr style='background:%5;'>"
                    "<td style='padding:6px 10px; font-family:monospace; color:#1a6fc4;'>%1</td>"
                    "<td style='padding:6px 10px; color:#6a8aaa;'>%2</td>"
                    "<td style='padding:6px 10px;'>%3</td>"
                    "<td style='padding:6px 10px; color:#4a5f75;'>%4</td>"
                    "</tr>")
                    .arg(QString::fromStdString(p.name))
                    .arg(QString::fromStdString(p.type))
                    .arg(p.required
                         ? "<span style='color:#e88c00;'>必填</span>"
                         : "<span style='color:#8a9bb0;'>可选</span>")
                    .arg(QString::fromStdString(p.description))
                    .arg(rowBg);
                odd = !odd;
            }
            html += "</table></div>";
        }

        html += "<div style='margin-top:20px;'>";
        html += "<div style='font-size:12px; font-weight:bold; color:#8a9bb0; margin-bottom:8px;'>调用示例</div>";
        html += QString("<pre style='background:#1e2a3a; color:#a8c4e0; border-radius:8px;"
                        " padding:12px 16px; font-size:12px; line-height:1.7;'><code>@%1")
                    .arg(QString::fromStdString(meta.id.value));
        for (auto& p : meta.params) {
            if (p.required) {
                html += QString(" %1=&lt;值&gt;").arg(QString::fromStdString(p.name));
            }
        }
        html += "</code></pre></div>";

        m_detailView->setHtml(html);
}
