#include "kb_panel.h"
#include "../application/services/kb_app_service.h"
#include "../domain/knowledge/models/knowledge_base.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QFormLayout>
#include <QRegularExpression>
#include <QTimer>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QPainterPath>
#include <QToolTip>
#include <QSet>
#include <cmath>
#include <algorithm>
#include <map>

// ============================================================
// GraphCanvas 实现
// ============================================================
GraphCanvas::GraphCanvas(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setMinimumHeight(320);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet("background: #f8f9fb;");
}

void GraphCanvas::clearGraph()
{
    m_nodes.clear();
    m_edges.clear();
    m_empty  = true;
    m_scale  = 1.0;
    m_offset = QPointF(0, 0);
    update();
}

void GraphCanvas::setGraph(const std::vector<GraphNode>& nodes,
                           const std::vector<GraphEdge>& edges)
{
    m_nodes = nodes;
    m_edges = edges;
    m_empty = nodes.empty();
    m_scale = 1.0;
    m_offset = QPointF(0, 0);
    layoutNodes();
    update();
}

// 力导向布局（80 次迭代）
void GraphCanvas::layoutNodes()
{
    if (m_nodes.empty()) return;

    int W = std::max(width(),  400);
    int H = std::max(height(), 320);
    int N = (int)m_nodes.size();

    // 初始圆形分布
    for (int i = 0; i < N; ++i) {
        double angle = 2.0 * M_PI * i / N;
        m_nodes[i].x = W / 2.0 + (W * 0.36) * std::cos(angle);
        m_nodes[i].y = H / 2.0 + (H * 0.36) * std::sin(angle);
    }

    // 建立 id -> index 映射
    std::map<QString, int> idx;
    for (int i = 0; i < N; ++i) idx[m_nodes[i].id] = i;

    std::vector<double> vx(N, 0), vy(N, 0);
    const double K = 55.0, DAMP = 0.88;

    for (int iter = 0; iter < 100; ++iter) {
        std::fill(vx.begin(), vx.end(), 0);
        std::fill(vy.begin(), vy.end(), 0);

        // 斥力
        for (int i = 0; i < N; ++i)
        for (int j = i + 1; j < N; ++j) {
            double dx = m_nodes[j].x - m_nodes[i].x;
            double dy = m_nodes[j].y - m_nodes[i].y;
            double d  = std::sqrt(dx*dx + dy*dy) + 0.01;
            double f  = K * K / d;
            vx[i] -= f * dx / d;  vy[i] -= f * dy / d;
            vx[j] += f * dx / d;  vy[j] += f * dy / d;
        }
        // 引力
        for (auto& e : m_edges) {
            auto ai = idx.find(e.from), bi = idx.find(e.to);
            if (ai == idx.end() || bi == idx.end()) continue;
            int a = ai->second, b = bi->second;
            double dx = m_nodes[b].x - m_nodes[a].x;
            double dy = m_nodes[b].y - m_nodes[a].y;
            double d  = std::sqrt(dx*dx + dy*dy) + 0.01;
            double f  = (d - K) * 0.06;
            vx[a] += f * dx / d;  vy[a] += f * dy / d;
            vx[b] -= f * dx / d;  vy[b] -= f * dy / d;
        }
        // 中心引力（防止漂移）
        for (int i = 0; i < N; ++i) {
            vx[i] += (W / 2.0 - m_nodes[i].x) * 0.01;
            vy[i] += (H / 2.0 - m_nodes[i].y) * 0.01;
        }
        // 更新
        for (int i = 0; i < N; ++i) {
            vx[i] *= DAMP; vy[i] *= DAMP;
            m_nodes[i].x += vx[i];
            m_nodes[i].y += vy[i];
            double margin = m_nodes[i].r + 8;
            m_nodes[i].x = std::max(margin, std::min(W - margin, m_nodes[i].x));
            m_nodes[i].y = std::max(margin, std::min(H - margin, m_nodes[i].y));
        }
    }
}

void GraphCanvas::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int W = width(), H = height();
    p.fillRect(0, 0, W, H, QColor("#f8f9fb"));

    if (m_empty) {
        p.setPen(QColor("#b0bcc8"));
        p.setFont(QFont("sans-serif", 20));
        p.drawText(QRect(0, H/2 - 30, W, 36), Qt::AlignCenter, "🕸️");
        p.setFont(QFont("sans-serif", 12));
        p.drawText(QRect(0, H/2 + 10, W, 24), Qt::AlignCenter, "加载知识库后可查看知识图谱");
        return;
    }

    p.save();
    p.translate(W / 2.0 + m_offset.x(), H / 2.0 + m_offset.y());
    p.scale(m_scale, m_scale);
    p.translate(-W / 2.0, -H / 2.0);

    // 建 id -> index
    std::map<QString, int> idx;
    for (int i = 0; i < (int)m_nodes.size(); ++i) idx[m_nodes[i].id] = i;

    // 画边
    for (auto& e : m_edges) {
        auto ai = idx.find(e.from), bi = idx.find(e.to);
        if (ai == idx.end() || bi == idx.end()) continue;
        const auto& a = m_nodes[ai->second];
        const auto& b = m_nodes[bi->second];
        QColor ec(74, 144, 217, 55);
        p.setPen(QPen(ec, e.w, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(a.x, a.y), QPointF(b.x, b.y));
    }

    // 节点颜色表
    static const QColor palettes[] = {
        QColor("#4A90D9"), QColor("#5aaa60"), QColor("#e08030"),
        QColor("#a050d0"), QColor("#d04050"), QColor("#20a880")
    };
    constexpr int NC = 6;

    // 画节点
    for (int i = 0; i < (int)m_nodes.size(); ++i) {
        const auto& n = m_nodes[i];
        QColor base = palettes[i % NC];
        QColor fill = base;  fill.setAlpha(50);

        QPainterPath path;
        path.addEllipse(QPointF(n.x, n.y), n.r, n.r);

        p.fillPath(path, fill);

        QPen border(base, i == m_hoveredNode ? 2.5 : 1.5);
        p.setPen(border);
        p.drawPath(path);

        // 标签
        QString lbl = n.label.length() > 6 ? n.label.left(5) + "…" : n.label;
        double fs = std::min(n.r * 0.85, 12.0);
        QFont f("sans-serif", (int)fs);
        f.setBold(true);
        p.setFont(f);
        p.setPen(QColor("#1a2537"));
        p.drawText(QRectF(n.x - n.r, n.y - n.r, n.r * 2, n.r * 2),
                   Qt::AlignCenter, lbl);
    }

    p.restore();

    // 提示文字
    p.setPen(QColor("#b0bcc8"));
    p.setFont(QFont("sans-serif", 10));
    p.drawText(QRect(8, H - 22, 280, 18), Qt::AlignLeft | Qt::AlignVCenter,
               "滚轮缩放  拖动平移  悬停查看详情");
}

void GraphCanvas::mouseMoveEvent(QMouseEvent* ev)
{
    if (m_dragging) {
        QPointF delta = ev->pos() - m_dragStart;
        m_offset = m_dragOffset + delta;
        update();
        return;
    }

    // 命中检测
    int W = width(), H = height();
    int hit = -1;
    for (int i = 0; i < (int)m_nodes.size(); ++i) {
        const auto& n = m_nodes[i];
        // 将屏幕坐标反变换到画布坐标
        double cx = (ev->x() - W / 2.0 - m_offset.x()) / m_scale + W / 2.0;
        double cy = (ev->y() - H / 2.0 - m_offset.y()) / m_scale + H / 2.0;
        double dx = cx - n.x, dy = cy - n.y;
        if (std::sqrt(dx*dx + dy*dy) <= n.r) { hit = i; break; }
    }
    if (hit != m_hoveredNode) {
        m_hoveredNode = hit;
        update();
    }
    if (hit >= 0) {
        QToolTip::showText(ev->globalPos(),
            QString("%1（频次: %2）").arg(m_nodes[hit].label).arg(m_nodes[hit].freq),
            this);
    } else {
        QToolTip::hideText();
    }
}

void GraphCanvas::mousePressEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        m_dragging   = true;
        m_dragStart  = ev->pos();
        m_dragOffset = m_offset;
        setCursor(Qt::ClosedHandCursor);
    }
}

void GraphCanvas::mouseReleaseEvent(QMouseEvent*)
{
    m_dragging = false;
    setCursor(Qt::ArrowCursor);
}

void GraphCanvas::wheelEvent(QWheelEvent* ev)
{
    double factor = ev->angleDelta().y() > 0 ? 1.12 : 0.89;
    m_scale = std::max(0.3, std::min(4.0, m_scale * factor));
    update();
}


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
            QString("📗  %1 · %2 篇 · %3 块")
            .arg(QString::fromStdString(kb->getName()))
            .arg(kb->getDocumentCount())
            .arg(kb->getChunkCount()));
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

    // 刷新当前可见 tab
    onTabChanged(m_tabWidget->currentIndex());
}

// ============================================================
// UI 搭建
// ============================================================
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
    layout->addWidget(titleBar);

    // ---- 主体：左侧列表 + 右侧页签 ----
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
        "  background: #ffffff; border: 1px solid #e8ecf0; border-radius: 6px;"
        "  padding: 8px 10px; margin: 0 0 3px 0; color: #2c3e55; font-size: 13px;"
        "}"
        "QListWidget::item:hover { background: #f0f7ff; border-color: #b8d4f0; }"
        "QListWidget::item:selected { background: #e6f2ff; border-color: #4A90D9; color: #1a6fc4; }"
    );
    connect(m_kbList, &QListWidget::itemClicked, this, &KBPanel::onKBSelected);
    leftLayout->addWidget(m_kbList, 1);

    // 新建 / 加载 / 卸载 同一排
    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(6);

    m_newKBBtn = new QPushButton("＋ 新建", leftWidget);
    m_newKBBtn->setFixedHeight(30);
    m_newKBBtn->setStyleSheet(
        "QPushButton { border: 1.5px solid #4A90D9; border-radius: 5px;"
        "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #5ca0e8,stop:1 #3d82d0);"
        "  color: white; font-size: 12px; font-weight: bold; padding: 0 6px; }"
        "QPushButton:hover { background: #4d90e0; }"
        "QPushButton:pressed { background: #2d72c0; }"
    );
    connect(m_newKBBtn, &QPushButton::clicked, this, &KBPanel::onCreateKB);

    m_loadBtn = new QPushButton("加 载", leftWidget);
    m_loadBtn->setFixedHeight(30);
    m_loadBtn->setStyleSheet(
        "QPushButton { border: 1.5px solid #4A90D9; border-radius: 5px;"
        "  padding: 0 6px; background: #fff; color: #4A90D9; font-size: 12px; font-weight: bold; }"
        "QPushButton:hover { background: #eef5fd; }"
    );
    connect(m_loadBtn, &QPushButton::clicked, this, &KBPanel::onLoadKB);

    m_unloadBtn = new QPushButton("卸 载", leftWidget);
    m_unloadBtn->setFixedHeight(30);
    m_unloadBtn->setStyleSheet(
        "QPushButton { border: 1.5px solid #e88c8c; border-radius: 5px;"
        "  padding: 0 6px; background: #fff; color: #cc4444; font-size: 12px; }"
        "QPushButton:hover { background: #fff5f5; }"
    );
    connect(m_unloadBtn, &QPushButton::clicked, this, &KBPanel::onUnloadKB);

    btnRow->addWidget(m_newKBBtn, 1);
    btnRow->addWidget(m_loadBtn, 1);
    btnRow->addWidget(m_unloadBtn, 1);
    leftLayout->addLayout(btnRow);
    splitter->addWidget(leftWidget);

    // === 右侧：页签 ===
    m_tabWidget = new QTabWidget(splitter);
    m_tabWidget->setStyleSheet(
        "QTabWidget::pane { border: none; background: #ffffff; }"
        "QTabWidget::tab-bar { alignment: left; }"
        "QTabBar { background: #f7f8fa; border-bottom: 1px solid #e4eaf2; }"
        "QTabBar::tab {"
        "  min-height: 38px;"
        "  min-width: 90px;"
        "  padding: 6px 18px;"
        "  font-size: 13px;"
        "  color: #6a8aaa;"
        "  background: #f7f8fa;"
        "  border: none;"
        "  border-bottom: 2px solid transparent;"
        "  margin-right: 2px;"
        "}"
        "QTabBar::tab:selected {"
        "  color: #1a6fc4;"
        "  background: #ffffff;"
        "  border-bottom: 2px solid #4A90D9;"
        "  font-weight: bold;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "  color: #2c5fa8;"
        "  background: #edf4fd;"
        "}"
    );
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &KBPanel::onTabChanged);

    // 文档管理 tab
    auto* docTab = new QWidget(m_tabWidget);
    setupDocTab(docTab);
    m_tabWidget->addTab(docTab, "📄 文档管理");

    // 切片查看 tab
    auto* chunkTab = new QWidget(m_tabWidget);
    setupChunkTab(chunkTab);
    m_tabWidget->addTab(chunkTab, "✂️ 切片查看");

    // 知识图谱 tab
    auto* graphTab = new QWidget(m_tabWidget);
    setupGraphTab(graphTab);
    m_tabWidget->addTab(graphTab, "🕸️ 知识图谱");

    splitter->addWidget(m_tabWidget);
    splitter->setSizes({200, 560});
    layout->addWidget(splitter, 1);
}

// ============================================================
// Tab 0: 文档管理
// ============================================================
void KBPanel::setupDocTab(QWidget* parent)
{
    auto* layout = new QVBoxLayout(parent);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(10);

    // 操作按钮行
    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(8);

    m_importFileBtn = new QPushButton("📂 导入文件", parent);
    m_importFileBtn->setFixedHeight(30);
    m_importFileBtn->setStyleSheet(
        "QPushButton { border: 1.5px solid #4A90D9; border-radius: 5px;"
        "  background: #eef5fd; color: #2a6fc4; font-size: 12px; font-weight: bold; padding: 0 10px; }"
        "QPushButton:hover { background: #d8ecfd; }"
        "QPushButton:disabled { color: #aac0d8; border-color: #ccddf0; }"
    );
    connect(m_importFileBtn, &QPushButton::clicked, this, &KBPanel::onImportFile);
    btnRow->addWidget(m_importFileBtn);

    m_importUrlBtn = new QPushButton("🌐 导入网址", parent);
    m_importUrlBtn->setFixedHeight(30);
    m_importUrlBtn->setStyleSheet(
        "QPushButton { border: 1.5px solid #20a880; border-radius: 5px;"
        "  background: #f0fbf7; color: #168060; font-size: 12px; font-weight: bold; padding: 0 10px; }"
        "QPushButton:hover { background: #d8f5ec; }"
        "QPushButton:disabled { color: #b0c8c0; border-color: #c8e8dc; }"
    );
    connect(m_importUrlBtn, &QPushButton::clicked, this, &KBPanel::onImportUrl);
    btnRow->addWidget(m_importUrlBtn);

    m_newTextBtn = new QPushButton("✏️ 新建文本", parent);
    m_newTextBtn->setFixedHeight(30);
    m_newTextBtn->setStyleSheet(
        "QPushButton { border: 1.5px solid #e0963a; border-radius: 5px;"
        "  background: #fffaf0; color: #c07020; font-size: 12px; font-weight: bold; padding: 0 10px; }"
        "QPushButton:hover { background: #ffeedd; }"
        "QPushButton:disabled { color: #c8b090; border-color: #e8d0b0; }"
    );
    connect(m_newTextBtn, &QPushButton::clicked, this, &KBPanel::onNewText);
    btnRow->addWidget(m_newTextBtn);
    btnRow->addStretch(1);
    layout->addLayout(btnRow);

    // 搜索行
    auto* searchRow = new QHBoxLayout();
    searchRow->setSpacing(8);
    m_searchEdit = new QLineEdit(parent);
    m_searchEdit->setPlaceholderText("在知识库中搜索文档...");
    m_searchEdit->setFixedHeight(34);
    m_searchEdit->setStyleSheet(
        "QLineEdit { border: 1.5px solid #dde3ec; border-radius: 6px;"
        "  padding: 0 12px; font-size: 13px; color: #1a2537; background: #f9fafc; }"
        "QLineEdit:focus { border-color: #4A90D9; background: #ffffff; }"
    );
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &KBPanel::onSearch);
    searchRow->addWidget(m_searchEdit, 1);

    auto* searchBtn = new QPushButton("搜索", parent);
    searchBtn->setFixedSize(64, 34);
    searchBtn->setStyleSheet(
        "QPushButton { border: none; border-radius: 6px;"
        "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #5ca0e8,stop:1 #3d82d0);"
        "  color: white; font-size: 12px; font-weight: bold; }"
        "QPushButton:hover { background: #4d90e0; }"
    );
    connect(searchBtn, &QPushButton::clicked, this, &KBPanel::onSearch);
    searchRow->addWidget(searchBtn);
    layout->addLayout(searchRow);

    // 结果区
    m_resultView = new QTextBrowser(parent);
    m_resultView->setFrameShape(QFrame::NoFrame);
    m_resultView->setOpenExternalLinks(true);
    m_resultView->setStyleSheet(
        "QTextBrowser { background: #ffffff; border: none; padding: 4px 0; }"
        "QScrollBar:vertical { background: transparent; width: 6px; }"
        "QScrollBar::handle:vertical { background: #c8d4e0; border-radius: 3px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    );
    m_resultView->setHtml(
        "<div style='text-align:center; padding:60px 20px; color:#b0bcc8;'>"
        "<div style='font-size:36px; margin-bottom:12px;'>📚</div>"
        "<div style='font-size:13px;'>从左侧选择并加载知识库<br/>然后在此处搜索文档</div>"
        "</div>"
    );
    layout->addWidget(m_resultView, 1);
}

// ============================================================
// Tab 1: 切片查看
// ============================================================
void KBPanel::setupChunkTab(QWidget* parent)
{
    auto* layout = new QVBoxLayout(parent);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(8);

    // 顶部统计 + 搜索
    auto* topRow = new QHBoxLayout();
    m_chunkStats = new QLabel("尚未加载知识库", parent);
    m_chunkStats->setStyleSheet("QLabel { color: #8a9bb0; font-size: 12px; background: transparent; }");
    topRow->addWidget(m_chunkStats);
    topRow->addStretch(1);

    m_chunkSearch = new QLineEdit(parent);
    m_chunkSearch->setPlaceholderText("过滤切片内容...");
    m_chunkSearch->setFixedWidth(200);
    m_chunkSearch->setFixedHeight(30);
    m_chunkSearch->setStyleSheet(
        "QLineEdit { border: 1.5px solid #dde3ec; border-radius: 5px;"
        "  padding: 0 10px; font-size: 12px; background: #f9fafc; }"
        "QLineEdit:focus { border-color: #4A90D9; }"
    );
    connect(m_chunkSearch, &QLineEdit::textChanged, this, &KBPanel::onChunkSearch);
    topRow->addWidget(m_chunkSearch);
    layout->addLayout(topRow);

    // 切片列表
    m_chunkView = new QTextBrowser(parent);
    m_chunkView->setFrameShape(QFrame::NoFrame);
    m_chunkView->setStyleSheet(
        "QTextBrowser { background: #f9fafc; border: 1px solid #e8ecf0; border-radius: 6px; padding: 8px; }"
        "QScrollBar:vertical { background: transparent; width: 6px; }"
        "QScrollBar::handle:vertical { background: #c8d4e0; border-radius: 3px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    );
    m_chunkView->setHtml(
        "<div style='text-align:center; padding:40px 20px; color:#b0bcc8;'>"
        "<div style='font-size:28px; margin-bottom:8px;'>✂️</div>"
        "<div style='font-size:13px;'>加载知识库后可查看切片内容</div>"
        "</div>"
    );
    layout->addWidget(m_chunkView, 1);
}

// ============================================================
// Tab 2: 知识图谱
// ============================================================
void KBPanel::setupGraphTab(QWidget* parent)
{
    auto* layout = new QVBoxLayout(parent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 顶部操作栏
    auto* topBar = new QWidget(parent);
    topBar->setFixedHeight(40);
    topBar->setStyleSheet("QWidget { background: #f7f8fa; border-bottom: 1px solid #e8ecf0; }");
    auto* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(14, 0, 14, 0);

    m_graphStats = new QLabel("关键词共现图 · 节点大小=频次 · 连线粗细=共现次数", topBar);
    m_graphStats->setStyleSheet("QLabel { font-size: 11px; color: #8a9bb0; background: transparent; }");
    topLayout->addWidget(m_graphStats);
    topLayout->addStretch(1);

    m_refreshGraphBtn = new QPushButton("🔄 刷新", topBar);
    m_refreshGraphBtn->setFixedHeight(26);
    m_refreshGraphBtn->setStyleSheet(
        "QPushButton { border: 1px solid #ccd4e0; border-radius: 4px;"
        "  background: #ffffff; color: #4a5f75; font-size: 11px; padding: 0 10px; }"
        "QPushButton:hover { background: #eef5fd; }"
    );
    connect(m_refreshGraphBtn, &QPushButton::clicked, this, [this]{ refreshGraphTab(); });
    topLayout->addWidget(m_refreshGraphBtn);
    layout->addWidget(topBar);

    // 原生绘图画布
    m_graphCanvas = new GraphCanvas(parent);
    layout->addWidget(m_graphCanvas, 1);
}

// ============================================================
// 刷新各 Tab 内容
// ============================================================
void KBPanel::onTabChanged(int index)
{
    if (!m_service || !m_service->currentKnowledgeBase()) return;
    switch (index) {
    case 0: refreshDocTab(); break;
    case 1: refreshChunkTab(m_chunkSearch ? m_chunkSearch->text() : ""); break;
    case 2: refreshGraphTab(); break;
    }
}

void KBPanel::refreshDocTab()
{
    // 显示知识库概述（文档列表）
    auto kb = m_service ? m_service->currentKnowledgeBase() : nullptr;
    if (!kb) return;

    auto docs = kb->getAllDocuments();
    if (docs.empty()) {
        m_resultView->setHtml(
            "<div style='padding:16px;'>"
            "<div style='font-size:14px; font-weight:bold; color:#1a2537;'>📚 " +
            QString::fromStdString(kb->getName()) +
            "</div><p style='color:#8a9bb0; font-size:13px; margin-top:8px;'>"
            "知识库为空，可通过上方按钮导入文件、网址或新建文本。</p></div>"
        );
        return;
    }

    QString html = QString(
        "<div style='padding:0 0 10px 0; border-bottom:1px solid #eef2f8; margin-bottom:10px;'>"
        "<b style='color:#1a2537; font-size:13px;'>📚 %1</b>"
        "<span style='color:#8a9bb0; font-size:12px; margin-left:8px;'>共 %2 篇文档 · %3 个切片</span>"
        "</div>"
    ).arg(QString::fromStdString(kb->getName()))
     .arg(kb->getDocumentCount())
     .arg(kb->getChunkCount());

    for (auto& doc : docs) {
        const auto& content = doc.getContent();
        QString preview = QString::fromStdString(
            content.size() > 180 ? content.substr(0, 180) + "..." : content
        ).toHtmlEscaped();
        html += QString(
            "<div style='border:1px solid #e8ecf0; border-radius:8px;"
            " padding:10px 14px; margin:0 0 8px 0; background:#fff;'>"
            "  <div style='font-size:13px; font-weight:bold; color:#1a2537; margin-bottom:4px;'>%1</div>"
            "  <div style='font-size:11px; color:#8a9bb0; margin-bottom:5px;'>%2</div>"
            "  <div style='font-size:12px; color:#6a8aaa; line-height:1.6;'>%3</div>"
            "</div>"
        ).arg(QString::fromStdString(doc.getTitle()).toHtmlEscaped())
         .arg(QString::fromStdString(doc.getId().value).toHtmlEscaped())
         .arg(preview);
    }
    m_resultView->setHtml(html);
}

void KBPanel::refreshChunkTab(const QString& filter)
{
    auto kb = m_service ? m_service->currentKnowledgeBase() : nullptr;
    if (!kb) {
        m_chunkStats->setText("尚未加载知识库");
        return;
    }

    std::vector<domain::knowledge::Chunk> chunks;
    if (filter.trimmed().isEmpty()) {
        chunks = kb->getAllChunks();
    } else {
        chunks = kb->searchChunks(filter.trimmed().toStdString());
    }

    m_chunkStats->setText(
        QString("共 %1 个切片（每片 ≤%2 字，重叠 %3 字）%4")
        .arg(kb->getChunkCount())
        .arg(domain::knowledge::KnowledgeBase::kChunkSize)
        .arg(domain::knowledge::KnowledgeBase::kChunkOverlap)
        .arg(filter.isEmpty() ? "" : QString(" · 过滤后 %1 个").arg(chunks.size()))
    );

    if (chunks.empty()) {
        m_chunkView->setHtml(
            "<div style='text-align:center; padding:40px; color:#b0bcc8;'>"
            "<div style='font-size:24px;'>🔍</div>"
            "<div style='font-size:13px; margin-top:8px;'>没有符合条件的切片</div>"
            "</div>"
        );
        return;
    }

    QString html;
    int shown = 0;
    for (const auto& chunk : chunks) {
        if (shown++ >= 200) break; // 最多显示200块，避免卡顿
        QString docId = QString::fromStdString(chunk.docId.value);
        QString docName = docId.section('/', -1); // 取文件名部分
        QString text = QString::fromStdString(chunk.text).toHtmlEscaped();

        // 高亮过滤词
        if (!filter.isEmpty()) {
            QString esc = QRegularExpression::escape(filter.trimmed());
            text.replace(QRegularExpression(esc, QRegularExpression::CaseInsensitiveOption),
                         "<mark style='background:#fff3b0; border-radius:2px;'>\\0</mark>");
        }

        html += QString(
            "<div style='border:1px solid #e4eaf2; border-radius:6px;"
            " padding:8px 12px; margin:0 0 6px 0; background:#ffffff;'>"
            "  <div style='display:flex; justify-content:space-between; margin-bottom:4px;'>"
            "    <span style='font-size:11px; color:#4A90D9; font-weight:bold;'>切片 #%1</span>"
            "    <span style='font-size:10px; color:#b0bcc8;'>%2</span>"
            "  </div>"
            "  <div style='font-size:12px; color:#2c3e55; line-height:1.7;"
            "    white-space:pre-wrap; word-break:break-all;'>%3</div>"
            "  <div style='font-size:10px; color:#c0ccd8; margin-top:4px;'>"
            "    %4 字</div>"
            "</div>"
        ).arg(chunk.index)
         .arg(docName.toHtmlEscaped())
         .arg(text)
         .arg(chunk.text.size());
    }
    if (shown >= 200) {
        html += "<div style='text-align:center; color:#b0bcc8; font-size:11px; padding:8px;'>"
                "仅显示前 200 个切片</div>";
    }
    m_chunkView->setHtml(html);
}

void KBPanel::refreshGraphTab()
{
    auto kb = m_service ? m_service->currentKnowledgeBase() : nullptr;
    if (!kb) {
        m_graphCanvas->clearGraph();
        m_graphStats->setText("关键词共现图 · 节点大小=频次 · 连线粗细=共现次数");
        return;
    }

    auto rawNodes = kb->getGraphNodes(40);
    auto rawEdges = kb->getGraphEdges(80);

    if (rawNodes.empty()) {
        m_graphCanvas->clearGraph();
        m_graphStats->setText("知识库内容不足，无法生成图谱");
        return;
    }

    int maxFreq = rawNodes.empty() ? 1 : rawNodes[0].freq;

    std::vector<GraphNode> nodes;
    nodes.reserve(rawNodes.size());
    for (auto& n : rawNodes) {
        GraphNode gn;
        gn.id    = QString::fromStdString(n.keyword);
        gn.label = gn.id;
        gn.freq  = n.freq;
        gn.r     = 12.0 + 20.0 * n.freq / std::max(maxFreq, 1);
        nodes.push_back(gn);
    }

    // id 集合（快速查找）
    QSet<QString> nodeSet;
    for (auto& gn : nodes) nodeSet.insert(gn.id);

    int maxW = rawEdges.empty() ? 1 : rawEdges[0].weight;
    std::vector<GraphEdge> edges;
    for (auto& e : rawEdges) {
        QString f = QString::fromStdString(e.from);
        QString t = QString::fromStdString(e.to);
        if (!nodeSet.contains(f) || !nodeSet.contains(t)) continue;
        GraphEdge ge;
        ge.from = f;
        ge.to   = t;
        ge.w    = 1.0 + 3.0 * e.weight / std::max(maxW, 1);
        edges.push_back(ge);
    }

    m_graphCanvas->setGraph(nodes, edges);
    m_graphStats->setText(
        QString("节点 %1 个 · 边 %2 个 · 滚轮缩放 · 拖动平移")
        .arg(nodes.size()).arg(edges.size()));
}

// ============================================================
// 槽函数
// ============================================================
void KBPanel::onKBSelected(QListWidgetItem* item)
{
    if (!item || !m_service) return;
    QString name = item->data(Qt::UserRole).toString();
    bool ok = m_service->loadKnowledgeBase(name.toStdString());
    if (ok) {
        emit knowledgeBaseChanged();
        refresh();
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
    m_statusLbl->setText("未加载");
    m_statusLbl->setStyleSheet(
        "QLabel { color: #8a9bb0; font-size: 11px;"
        " background: #f0f5fb; border: 1px solid #dde8f5;"
        " border-radius: 10px; padding: 2px 10px; }");
    m_resultView->setHtml(
        "<div style='text-align:center; padding:60px 20px; color:#b0bcc8;'>"
        "<div style='font-size:32px; margin-bottom:10px;'>📤</div>"
        "<div style='font-size:13px;'>知识库已卸载</div>"
        "</div>"
    );
    m_chunkView->setHtml(
        "<div style='text-align:center; padding:40px; color:#b0bcc8;'>"
        "<div style='font-size:24px;'>✂️</div>"
        "<div style='font-size:13px; margin-top:8px;'>加载知识库后可查看切片</div>"
        "</div>"
    );
    m_graphCanvas->clearGraph();
    m_graphStats->setText("关键词共现图 · 节点大小=频次 · 连线粗细=共现次数");
}

void KBPanel::onSearch()
{
    if (!m_service) return;
    QString keyword = m_searchEdit->text().trimmed();
    if (keyword.isEmpty()) { refreshDocTab(); return; }

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
        "<div style='padding:0 0 10px 0; border-bottom:1px solid #eef2f8; margin-bottom:10px;'>"
        "<span style='font-size:13px; color:#4a5f75;'>搜索 </span>"
        "<b style='color:#1a2537;'>\"%1\"</b>"
        "<span style='color:#8a9bb0; font-size:12px; margin-left:8px;'>%2 篇结果</span>"
        "</div>"
    ).arg(keyword.toHtmlEscaped()).arg(docs.size());

    for (auto& doc : docs) {
        const auto& c = doc.getContent();
        html += QString(
            "<div style='border:1px solid #e8ecf0; border-radius:8px;"
            " padding:12px 14px; margin:0 0 10px 0; background:#fff;'>"
            "  <div style='font-size:13px; font-weight:bold; color:#1a2537; margin-bottom:5px;'>%1</div>"
            "  <div style='font-size:12px; color:#6a8aaa; line-height:1.6;'>%2...</div>"
            "</div>"
        ).arg(
            QString::fromStdString(doc.getTitle()).toHtmlEscaped(),
            QString::fromStdString(c.substr(0, std::min(c.size(), size_t(200)))).toHtmlEscaped()
        );
    }
    m_resultView->setHtml(html);
}

void KBPanel::onChunkSearch()
{
    if (m_tabWidget->currentIndex() == 1)
        refreshChunkTab(m_chunkSearch->text());
}

// ---- 导入文件 ----
void KBPanel::onImportFile()
{
    if (!m_service || !m_service->currentKnowledgeBase()) {
        QMessageBox::warning(this, "提示", "请先加载一个知识库再导入文件。");
        return;
    }

    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "选择要导入的文件",
        "",
        "文本/代码文件 (*.md *.markdown *.txt *.h *.cpp *.py *.java *.json *.xml *.csv);;"
        "所有文件 (*)"
    );
    if (files.isEmpty()) return;

    int success = 0, fail = 0;
    m_importFileBtn->setEnabled(false);
    for (const QString& fp : files) {
        int ret = m_service->importFile(fp.toStdString());
        if (ret == 0) success++;
        else fail++;
    }
    m_importFileBtn->setEnabled(true);

    refresh();
    QString msg = QString("导入完成：成功 %1 个").arg(success);
    if (fail > 0) msg += QString("，失败 %1 个").arg(fail);

    m_resultView->setHtml(QString(
        "<div style='padding:16px;'>"
        "  <div style='background:#edfaf3; border:1px solid #a8ddc0; border-radius:8px;"
        "    padding:12px 16px; font-size:13px; color:#2e8b57;'>"
        "    ✅ %1<br/>"
        "    <span style='font-size:11px; color:#5aaa80;'>当前知识库共 %2 篇文档 · %3 个切片</span>"
        "  </div>"
        "</div>"
    ).arg(msg)
     .arg(m_service->currentKnowledgeBase()->getDocumentCount())
     .arg(m_service->currentKnowledgeBase()->getChunkCount()));
}

// ---- 新建文本 ----
void KBPanel::onNewText()
{
    if (!m_service || !m_service->currentKnowledgeBase()) {
        QMessageBox::warning(this, "提示", "请先加载一个知识库再新建文本。");
        return;
    }

    // 弹出文本编辑对话框
    QDialog dlg(this);
    dlg.setWindowTitle("✏️ 新建文本文档");
    dlg.resize(600, 500);

    auto* layout = new QVBoxLayout(&dlg);
    layout->setSpacing(10);
    layout->setContentsMargins(16, 16, 16, 16);

    auto* titleEdit = new QLineEdit(&dlg);
    titleEdit->setPlaceholderText("文档标题（必填）");
    titleEdit->setFixedHeight(34);
    titleEdit->setStyleSheet(
        "QLineEdit { border: 1.5px solid #dde3ec; border-radius: 6px;"
        "  padding: 0 12px; font-size: 13px; background: #f9fafc; }"
        "QLineEdit:focus { border-color: #4A90D9; }"
    );
    layout->addWidget(titleEdit);

    auto* hintLabel = new QLabel("支持 Markdown 格式，# 开头为标题，**粗体**，`代码` 等", &dlg);
    hintLabel->setStyleSheet("QLabel { font-size: 11px; color: #8a9bb0; }");
    layout->addWidget(hintLabel);

    auto* contentEdit = new QPlainTextEdit(&dlg);
    contentEdit->setPlaceholderText("在此输入文档内容（支持 Markdown）...");
    contentEdit->setStyleSheet(
        "QPlainTextEdit { border: 1.5px solid #dde3ec; border-radius: 6px;"
        "  padding: 8px 12px; font-size: 13px; font-family: monospace; background: #f9fafc; }"
        "QPlainTextEdit:focus { border-color: #4A90D9; background: #fff; }"
    );
    layout->addWidget(contentEdit, 1);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    buttons->button(QDialogButtonBox::Ok)->setText("保 存");
    buttons->button(QDialogButtonBox::Cancel)->setText("取 消");
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(buttons);

    if (dlg.exec() != QDialog::Accepted) return;

    QString title   = titleEdit->text().trimmed();
    QString content = contentEdit->toPlainText().trimmed();

    if (title.isEmpty()) {
        QMessageBox::warning(this, "提示", "标题不能为空。");
        return;
    }
    if (content.isEmpty()) {
        QMessageBox::warning(this, "提示", "内容不能为空。");
        return;
    }

    // 构建 Markdown 内容（如果没有标题行则自动加）
    QString md = content;
    if (!md.startsWith('#'))
        md = "# " + title + "\n\n" + md;

    int ret = m_service->importText(title.toStdString(), md.toStdString());
    if (ret != 0) {
        QMessageBox::critical(this, "保存失败", "文件写入失败，请检查知识库目录权限。");
        return;
    }

    refresh();
    m_resultView->setHtml(QString(
        "<div style='padding:16px;'>"
        "  <div style='background:#edfaf3; border:1px solid #a8ddc0; border-radius:8px;"
        "    padding:12px 16px; font-size:13px; color:#2e8b57;'>"
        "    ✅ 文档「%1」已保存！<br/>"
        "    <span style='font-size:11px; color:#5aaa80;'>当前知识库共 %2 篇文档 · %3 个切片</span>"
        "  </div>"
        "</div>"
    ).arg(title.toHtmlEscaped())
     .arg(m_service->currentKnowledgeBase()->getDocumentCount())
     .arg(m_service->currentKnowledgeBase()->getChunkCount()));
}

// ---- 导入网址 ----
void KBPanel::onImportUrl()
{
    if (!m_service || !m_service->currentKnowledgeBase()) {
        QMessageBox::warning(this, "提示", "请先加载一个知识库再导入网址。");
        return;
    }

    bool ok = false;
    QString url = QInputDialog::getText(
        this,
        "🌐 导入网页到知识库",
        QString("将网页内容导入到知识库「%1」\n\n请输入网址（支持 http/https）：")
            .arg(QString::fromStdString(m_service->currentKnowledgeBase()->getName())),
        QLineEdit::Normal, "https://", &ok
    );
    if (!ok || url.trimmed().isEmpty()) return;
    url = url.trimmed();
    if (!url.startsWith("http://") && !url.startsWith("https://"))
        url = "https://" + url;

    m_importUrlBtn->setEnabled(false);
    m_resultView->setHtml(
        "<div style='text-align:center; padding:60px 20px; color:#6a8aaa;'>"
        "<div style='font-size:30px; margin-bottom:10px;'>🌐</div>"
        "<div style='font-size:13px;'>正在抓取网页内容...</div>"
        "<div style='font-size:11px; color:#b0bcc8; margin-top:6px;'>" +
        url.toHtmlEscaped() + "</div></div>"
    );

    m_service->importFromUrlAsync(url.toStdString(), "", [this, url](int result) {
        m_importUrlBtn->setEnabled(true);
        if (result == 0) {
            refresh();
            m_resultView->setHtml(QString(
                "<div style='padding:20px;'>"
                "  <div style='background:#edfaf3; border:1px solid #a8ddc0; border-radius:8px;"
                "    padding:14px 18px; font-size:13px; color:#2e8b57;'>"
                "    ✅ 网页导入成功！<br/>"
                "    <span style='font-size:11px; color:#5aaa80;'>"
                "    当前知识库共 %1 篇文档 · %2 个切片</span>"
                "  </div>"
                "  <div style='margin-top:12px; padding:10px 14px;"
                "    background:#f4f6fa; border-radius:6px; font-size:12px; color:#6a8aaa;'>"
                "    📎 来源: <a href='%3' style='color:#4A90D9;'>%3</a>"
                "  </div>"
                "</div>"
            ).arg(m_service->currentKnowledgeBase() ?
                  (int)m_service->currentKnowledgeBase()->getDocumentCount() : 0)
             .arg(m_service->currentKnowledgeBase() ?
                  (int)m_service->currentKnowledgeBase()->getChunkCount() : 0)
             .arg(url.toHtmlEscaped()));
        } else {
            QMessageBox::warning(this, "导入失败",
                result == 1 ? "网络请求失败，请检查网址和网络连接。"
              : result == 2 ? "请先加载知识库再导入网址。"
              : "文件写入失败，请检查知识库目录权限。");
        }
    });
}

// ---- 新建知识库 ----
void KBPanel::onCreateKB()
{
    if (!m_service) return;

    bool ok = false;
    QString name = QInputDialog::getText(
        this, "新建知识库",
        "请输入知识库名称：\n（支持中文、字母、数字、下划线，不能含 / \\ : * ? \" < > |）",
        QLineEdit::Normal, "", &ok
    );
    if (!ok || name.trimmed().isEmpty()) return;
    name = name.trimmed();

    static const QRegularExpression invalidChars(R"([/\\:*?"<>|])");
    if (name.contains(invalidChars)) {
        QMessageBox::warning(this, "新建失败",
            "知识库名称包含非法字符，请避免使用：/ \\ : * ? \" < > |");
        return;
    }

    if (!m_service->createKnowledgeBase(name.toStdString())) {
        QMessageBox::warning(this, "新建失败",
            QString("知识库「%1」已存在，或目录创建失败。").arg(name));
        return;
    }

    refresh();
    for (int i = 0; i < m_kbList->count(); ++i) {
        if (m_kbList->item(i)->data(Qt::UserRole).toString() == name) {
            m_kbList->setCurrentRow(i);
            break;
        }
    }
    m_resultView->setHtml(QString(
        "<div style='padding:20px;'>"
        "  <div style='background:#edfaf3; border:1px solid #a8ddc0; border-radius:8px;"
        "    padding:14px 18px; font-size:13px; color:#2e8b57;'>"
        "    ✅  知识库「%1」已创建！<br/>"
        "    <span style='font-size:11px; color:#5aaa80;'>"
        "    加载后通过上方按钮导入内容</span>"
        "  </div>"
        "</div>"
    ).arg(name.toHtmlEscaped()));
}
