# 工程图纸设计助手

## 项目简介

基于 **Qt 5.15.2 + C++14** 的桌面智能助手，通过自然语言对话帮助工程师解决图纸设计问题。支持加载工程知识库、管理技能插件，并提供 Markdown 格式内容渲染。

## 核心功能

- 💬 **聊天对话** — 自然语言交互，`@skill_id` 指令直接调用技能
- 📚 **知识库管理** — 加载本地 Markdown 知识库，支持 🌐 网页 URL 一键导入
- 🔧 **技能管理** — 加载/注册 Skill，支持 ✨ 对话式向导创建新技能
- 📝 **Markdown 渲染** — 内置轻量渲染器，支持标题/列表/代码块等常用语法

## 技术栈

| 组件     | 版本   | 说明                               |
| -------- | ------ | ---------------------------------- |
| Qt       | 5.15.2 | UI + 网络（QNetworkAccessManager） |
| C++      | 14     | 语言标准                           |
| 构建工具 | qmake  | 主构建系统（.pro 文件）            |

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
3. 在搜索框输入关键词检索内容
4. 点「🌐 导入网址」可将网页内容自动抓取并导入当前知识库

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

| 文档                 | 内容                                  |
| -------------------- | ------------------------------------- |
| 01-领域模型设计.md   | DDD 边界上下文、聚合设计、领域事件    |
| 02-系统架构设计.md   | 分层架构、模块划分、依赖关系          |
| 03-技术方案设计.md   | Qt 界面、Markdown 渲染、知识库、Skill |
| 04-需求与概要设计.md | 功能需求（PRD）+ 概要设计             |
| 06-详细设计.md       | 类设计、方法签名、数据模型            |
| 项目总结.md          | 开发进度、已实现功能、编译步骤、FAQ   |

## 更新日志

### v1.1.0 (2026-03-25)

- ✨ 消息气泡加头像（用户/AI）
- ✨ 技能面板增加「对话创建」向导
- ✨ 知识库支持网页 URL 导入
- 🐛 修复消息气泡对齐问题
- 🏗 清理空目录，合并冗余文档

### v1.0.0 (2026-03-24)

- 初始版本，实现全部核心功能

## 许可证

MIT License
