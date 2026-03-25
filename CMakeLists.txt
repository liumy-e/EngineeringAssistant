cmake_minimum_required(VERSION 3.5)
project(EngineeringAssistant VERSION 1.0.0 LANGUAGES CXX)

# ============================================================
# C++ 标准
# ============================================================
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# Qt 5.6.3 配置
# ============================================================
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

# ============================================================
# 源文件列表
# ============================================================
set(SOURCES
    # 入口
    src/main.cpp

    # 领域层 - Chat
    src/domain/chat/models/message.cpp
    src/domain/chat/models/chat_session.cpp
    src/domain/chat/services/chat_processor.cpp

    # 领域层 - Knowledge
    src/domain/knowledge/models/document.cpp
    src/domain/knowledge/models/knowledge_base.cpp
    src/domain/knowledge/services/knowledge_loader.cpp

    # 领域层 - Skill
    src/domain/skill/models/skill_registry.cpp
    src/domain/skill/services/skill_executor.cpp

    # 应用层
    src/application/services/chat_app_service.cpp
    src/application/services/kb_app_service.cpp
    src/application/services/skill_app_service.cpp

    # 基础设施层
    src/infrastructure/rendering/simple_markdown_renderer.cpp
    src/infrastructure/skill/skill_loader.cpp
    src/infrastructure/skill/builtin_skills.cpp

    # 表现层
    src/presentation/main_window.cpp
    src/presentation/chat_widget.cpp
    src/presentation/skill_panel.cpp
    src/presentation/kb_panel.cpp
)

# ============================================================
# 包含目录
# ============================================================
include_directories(
    ${CMAKE_SOURCE_DIR}/src
)

# ============================================================
# 可执行目标
# ============================================================
add_executable(EngineeringAssistant ${SOURCES})

# ============================================================
# 链接 Qt 库
# ============================================================
target_link_libraries(EngineeringAssistant
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)

# ============================================================
# 构建后自动复制数据目录
# ============================================================
add_custom_command(TARGET EngineeringAssistant POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/data"
        "$<TARGET_FILE_DIR:EngineeringAssistant>/data"
    COMMENT "Copying data directory..."
)

add_custom_command(TARGET EngineeringAssistant POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/config"
        "$<TARGET_FILE_DIR:EngineeringAssistant>/config"
    COMMENT "Copying config directory..."
)

# ============================================================
# 安装规则
# ============================================================
install(TARGETS EngineeringAssistant RUNTIME DESTINATION bin)
install(DIRECTORY data DESTINATION bin)
