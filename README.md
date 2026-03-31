# 工程图纸设计助手

## 项目简介

基于 **Qt 5.15.2 + C++17** 的桌面智能助手，通过自然语言对话帮助工程师解决图纸设计问题。支持加载工程知识库、管理技能插件、原生知识图谱可视化，并提供 Markdown 格式内容渲染。

## 核心功能

- 💬 **聊天对话** — 自然语言交互，`@skill_id` 指令直接调用技能，自动结合知识库内容（RAG）回答问题
- 📚 **知识库管理** — 加载本地 Markdown 知识库，支持 🌐 网页 URL 一键导入，支持全文检索和切片查看
- 🔧 **技能管理** — 加载/注册 Skill，支持 ✨ 对话式向导创建新技能
- 📝 **Markdown 渲染** — 内置轻量渲染器，支持标题/列表/代码块等常用语法
- 🕸 **知识图谱** — 原生 Qt 绘制（QPainter），支持拖拽、缩放、悬停提示
- 🎨 **优化布局** — 聊天气泡自动换行、左右对齐、带头像标识

## 技术栈

| 组件 | 版本 | 说明 |
|------|------|------|
| Qt | 5.15.2 | UI + 网络（QNetworkAccessManager） + 绘图（QPainter） |
| C++ | 17 | 语言标准 |
| 构建工具 | qmake | 主构建系统（.pro 文件） |

## 项目结构

```
EngineeringAssistant/
├── EngineeringAssistant.pro   # qmake 构建配置（主用）
├── CMakeLists.txt             # CMake 备用配置
├── README.md
├── docs/
│   ├── 01-领域模型设计.md
│   ├── 02-系统架构设计.md
│   ├── 03-技术方案设计.md
│   ├── 04-需求与概要设计.md   ← 需求 PRD + 概要设计合并
│   ├── 06-详细设计.md
│   └── 项目总结.md            ← 开发进度、已实现功能、FAQ
├── src/
│   ├── main.cpp
│   ├── presentation/          # 表现层（Qt Widgets）
│   │   ├── main_window.{h,cpp}
│   │   ├── chat_widget.{h,cpp}
│   │   ├── skill_panel.{h,cpp}
│   │   └── kb_panel.{h,cpp}
│   ├── application/services/  # 应用层（业务协调）
│   │   ├── chat_app_service.{h,cpp}
│   │   ├── skill_app_service.{h,cpp}
│   │   └── kb_app_service.{h,cpp}
│   ├── domain/                # 领域层（核心业务）
│   │   ├── chat/
│   │   ├── knowledge/
│   │   ├── skill/
│   │   ├── rendering/
│   │   └── common/
│   └── infrastructure/        # 基础设施层（技术实现）
│       ├── rendering/
│       └── skill/
├── data/
│   ├── knowledge/             # 知识库（Markdown 文档）
│   │   ├── standards/
│   │   └── processes/
│   └── skills/                # 技能配置（skill.json）
│       ├── drawing-review/
│       ├── drawing-dimension-check/
│       └── standard-query/
└── build/                     # 编译输出
```

## 核心技术实现

### 1. 聊天气泡自动布局（`chat_widget.cpp`）

**问题**：之前使用 `width='1'` 导致聊天气泡被压缩到每行只显示一个字。

**解决方案**：
- 用户气泡：采用百分比布局（20% 留白 + 72% 内容）
- AI 气泡：对称布局（36% 头像 + 72% 内容 + 20% 留白）
- 使用 `<div>` 替换内嵌 `<table>`，添加 `word-wrap:break-word` 自动换行
- CSS 样式：蓝白配色、圆角、阴影区分用户/AI

### 2. 知识图谱原生绘制（`kb_panel.{h,cpp}`）

**问题**：`QTextBrowser` 不支持 JavaScript，HTML Canvas 代码无法执行。

**解决方案**：创建 `GraphCanvas` 类（继承 `QWidget`）

| 特性 | 实现方式 |
|------|----------|
| 力导向布局 | 100 次迭代，含节点斥力 + 边引力 + 中心引力 |
| 交互功能 | 滚轮缩放（0.3x~4x）、鼠标拖拽平移 |
| 悬停提示 | 鼠标悬停节点显示详细信息的 Tooltip |
| 绘制引擎 | QPainter（原生 Qt，无需第三方库） |

**数据结构**：
```cpp
struct GraphNode { QString id, label; int freq; double x, y, r; };
struct GraphEdge { QString from, to; double w; };
```

### 3. RAG 知识库上下文解析（`chat_processor.cpp`）

**问题**：纯关键词匹配，无法利用知识库检索结果。

**解决方案**：新增 RAG 上下文检测逻辑

```cpp
const std::string kbTag = "【知识库参考】";
const std::string userTag = "**用户问题：** ";
```

当检测到 `kbTag` 时：
1. 提取用户实际问题（`userTag` 之后）
2. 提取知识库内容（`kbTag` 与 `---` 分隔线之间）
3. 组织成 Markdown 格式回复：`根据知识库检索结果...`

**关键词匹配兜底**：所有回复统一使用 Markdown 格式（表格、列表、标题），提升可读性。

## 编译与运行

### macOS

```bash
cd EngineeringAssistant
mkdir -p build && cd build
~/Qt/5.15.2/clang_64/bin/qmake ../EngineeringAssistant.pro
make -j4

# 运行
DYLD_FRAMEWORK_PATH=~/Qt/5.15.2/clang_64/lib ./EngineeringAssistant
```

### 注意事项

- Qt 路径需根据实际安装位置调整
- C++17 标准由 `CONFIG += c++17` 在 `.pro` 文件中配置
- `data` 目录会自动复制到构建目录

## 使用说明

### 聊天对话
- 输入框输入问题，**Enter** 发送，**Shift+Enter** 换行
- 使用 `@技能ID 参数名=参数值` 调用技能，例如：
  ```
  @drawing-review part=主轴零件
  @standard-query standard=GB/T14689
  ```

### 知识库
1. 切换到「📚 知识库」面板
2. 选中知识库 → 点「加载」
3. **文档检索**：在搜索框输入关键词，高亮展示匹配结果
4. **切片查看**：切换到「切片」页签，查看分块内容和统计信息
5. **知识图谱**：切换到「图谱」页签，可视化展示概念关系
   - 滚轮缩放、拖拽平移
   - 鼠标悬停节点查看详情
6. **导入内容**：
   - 点「🌐 导入网址」自动抓取网页正文
   - 点「📄 导入文件」加载本地文档
   - 点「📝 新建文本」手动编辑内容

### 技能管理
1. 切换到「🔧 技能」面板
2. 点「加载目录」从 `data/skills/` 加载技能
3. 点「✨ 对话创建」按向导填写信息，自动生成 skill.json 并注册

### 知识库格式（本地文件）
在 `data/knowledge/<库名>/` 下放入 `.md` 格式文档即可加载。

### Skill 格式
在 `data/skills/<id>/skill.json` 中配置：
```json
{
  "id": "my-skill",
  "name": "我的技能",
  "version": "1.0.0",
  "description": "技能描述",
  "author": "作者",
  "parameters": [
    { "name": "param1", "type": "string", "required": true, "description": "参数说明" }
  ]
}
```

## 文档索引

| 文档 | 内容 |
|------|------|
| 01-领域模型设计.md | DDD 边界上下文、聚合设计、领域事件 |
| 02-系统架构设计.md | 分层架构、模块划分、依赖关系 |
| 03-技术方案设计.md | Qt 界面、Markdown 渲染、知识库、Skill |
| 04-需求与概要设计.md | 功能需求（PRD）+ 概要设计 |
| 06-详细设计.md | 类设计、方法签名、数据模型 |
| 项目总结.md | 开发进度、已实现功能、编译步骤、FAQ |

## 更新日志

### v1.2.0 (2026-03-31)
- ✨ 新增知识图谱：原生 Qt 绘制（QPainter），支持拖拽、缩放、悬停
- ✨ RAG 上下文解析：自动检测 `【知识库参考】` 标签，结合知识库内容回答问题
- 🐛 修复聊天气泡宽度问题：每行只显示一个字 → 自动换行布局
- 🎨 优化聊天气泡样式：百分比布局 + 头像 + 阴影
- 🔧 升级 C++ 标准：C++14 → C++17
- 📝 完善文档：更新 README，新增技术实现说明

### v1.1.0 (2026-03-25)
- ✨ 消息气泡加头像（用户/AI）
- ✨ 技能面板增加「对话创建」向导
- ✨ 知识库支持网页 URL 导入
- 🐛 修复消息气泡对齐问题
- 🏗 清理空目录，合并冗余文档

### v1.0.0 (2026-03-24)
- 初始版本，实现全部核心功能

## 常见问题

### Q1: 知识图谱显示不出来？
**A**: 已在 v1.2.0 中修复。之前使用 HTML Canvas + JS 方案，`QTextBrowser` 不支持 JavaScript。现已改为原生 `GraphCanvas` 类，使用 `QPainter` 直接绘制，确保所有平台兼容。

### Q2: 聊天消息每行只显示一个字？
**A**: 已在 v1.2.0 中修复。之前使用 `width='1'` 导致气泡被压缩。现已改为百分比布局（用户 72%，AI 72%），并添加 `word-wrap:break-word` 自动换行。

### Q3: 不能正常回答问题，缺少配置吗？
**A**: 建议流程：
1. 先在「📚 知识库」面板导入文档并加载
2. 在聊天中提问，系统会自动检索知识库
3. 检测到 `【知识库参考】` 标签时，会结合知识库内容组织回复
4. 无知识库时，关键词匹配仍可回答基础问题

### Q4: 如何添加新的知识库？
**A**: 在 `data/knowledge/` 下创建目录，放入 Markdown 文档即可。支持以下方式：
- 本地文件导入
- 网页 URL 抓取（自动转为 Markdown）
- 手动新建文本

## 许可证

MIT License
