#ifndef KB_PANEL_H
#define KB_PANEL_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextBrowser>
#include <QTabWidget>
#include <QProgressBar>
#include <QDialog>
#include <QPlainTextEdit>
#include <QPainter>
#include <QScrollArea>
#include <memory>
#include <vector>
#include <string>

namespace application { class KBAppService; }

// ============================================================
// 知识图谱节点/边数据结构（供 GraphCanvas 使用）
// ============================================================
struct GraphNode {
    QString  id;
    QString  label;
    int      freq  = 1;
    double   x = 0, y = 0;   // 布局后坐标
    double   r = 12;          // 半径
};

struct GraphEdge {
    QString from, to;
    double  w = 1.0;
};

// ============================================================
// 原生 Qt 知识图谱画布（QPainter 绘制，不依赖 JS/Canvas）
// ============================================================
class GraphCanvas : public QWidget {
    Q_OBJECT
public:
    explicit GraphCanvas(QWidget* parent = nullptr);

    void setGraph(const std::vector<GraphNode>& nodes,
                  const std::vector<GraphEdge>& edges);
    void clearGraph();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void layoutNodes();   // 简单力导向布局

    std::vector<GraphNode> m_nodes;
    std::vector<GraphEdge> m_edges;

    // 拖拽 / 缩放状态
    double  m_scale  = 1.0;
    QPointF m_offset;
    bool    m_dragging = false;
    QPoint  m_dragStart;
    QPointF m_dragOffset;

    int     m_hoveredNode = -1;
    bool    m_empty = true;
};

// ============================================================
// 知识库管理面板
// ============================================================
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
    void onImportFile();
    void onNewText();
    void onCreateKB();
    void onTabChanged(int index);
    void onChunkSearch();

private:
    void setupUI();
    void setupDocTab(QWidget* parent);
    void setupChunkTab(QWidget* parent);
    void setupGraphTab(QWidget* parent);

    void refreshDocTab();
    void refreshChunkTab(const QString& filter = "");
    void refreshGraphTab();

    std::shared_ptr<application::KBAppService> m_service;

    // 左侧
    QListWidget*  m_kbList       = nullptr;
    QPushButton*  m_newKBBtn     = nullptr;
    QPushButton*  m_loadBtn      = nullptr;
    QPushButton*  m_unloadBtn    = nullptr;
    QLabel*       m_statusLbl    = nullptr;

    // 右侧页签
    QTabWidget*   m_tabWidget    = nullptr;

    // Tab 0: 文档管理
    QLineEdit*    m_searchEdit   = nullptr;
    QTextBrowser* m_resultView   = nullptr;
    QPushButton*  m_importUrlBtn = nullptr;
    QPushButton*  m_importFileBtn= nullptr;
    QPushButton*  m_newTextBtn   = nullptr;

    // Tab 1: 切片查看
    QLineEdit*    m_chunkSearch  = nullptr;
    QTextBrowser* m_chunkView    = nullptr;
    QLabel*       m_chunkStats   = nullptr;

    // Tab 2: 知识图谱（原生 QPainter）
    GraphCanvas*  m_graphCanvas  = nullptr;
    QLabel*       m_graphStats   = nullptr;
    QPushButton*  m_refreshGraphBtn = nullptr;
};

#endif // KB_PANEL_H
