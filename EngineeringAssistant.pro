QT += core gui widgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET   = EngineeringAssistant
TEMPLATE = app

CONFIG += c++14
CONFIG -= app_bundle

INCLUDEPATH += src \
    src/domain/chat/models \
    src/domain/chat/services \
    src/domain/chat/repositories \
    src/domain/knowledge/models \
    src/domain/knowledge/services \
    src/domain/knowledge/repositories \
    src/domain/rendering/models \
    src/domain/rendering/services \
    src/domain/skill/models \
    src/domain/skill/interfaces \
    src/domain/skill/services \
    src/domain/common \
    src/application/services \
    src/infrastructure/rendering \
    src/infrastructure/skill \
    src/presentation

SOURCES += \
    src/main.cpp \
    src/domain/chat/models/message.cpp \
    src/domain/chat/models/chat_session.cpp \
    src/domain/chat/services/chat_processor.cpp \
    src/domain/knowledge/models/document.cpp \
    src/domain/knowledge/models/knowledge_base.cpp \
    src/domain/knowledge/services/knowledge_loader.cpp \
    src/domain/skill/models/skill_registry.cpp \
    src/domain/skill/services/skill_executor.cpp \
    src/application/services/chat_app_service.cpp \
    src/application/services/kb_app_service.cpp \
    src/application/services/skill_app_service.cpp \
    src/infrastructure/rendering/simple_markdown_renderer.cpp \
    src/infrastructure/skill/skill_loader.cpp \
    src/infrastructure/skill/builtin_skills.cpp \
    src/presentation/main_window.cpp \
    src/presentation/chat_widget.cpp \
    src/presentation/skill_panel.cpp \
    src/presentation/kb_panel.cpp

HEADERS += \
    src/domain/common/exceptions.h \
    src/domain/chat/models/message_type.h \
    src/domain/chat/models/message_id.h \
    src/domain/chat/models/session_id.h \
    src/domain/chat/models/message.h \
    src/domain/chat/models/chat_session.h \
    src/domain/chat/repositories/chat_session_repository.h \
    src/domain/chat/services/chat_processor.h \
    src/domain/knowledge/models/document_id.h \
    src/domain/knowledge/models/knowledge_id.h \
    src/domain/knowledge/models/knowledge_type.h \
    src/domain/knowledge/models/document.h \
    src/domain/knowledge/models/knowledge_base.h \
    src/domain/knowledge/repositories/knowledge_base_repository.h \
    src/domain/knowledge/services/knowledge_loader.h \
    src/domain/rendering/models/render_result.h \
    src/domain/rendering/services/markdown_renderer.h \
    src/domain/skill/models/skill_id.h \
    src/domain/skill/models/skill_status.h \
    src/domain/skill/models/skill_param.h \
    src/domain/skill/models/skill_meta.h \
    src/domain/skill/models/execute_result.h \
    src/domain/skill/models/skill_registry.h \
    src/domain/skill/interfaces/skill_interface.h \
    src/domain/skill/services/skill_executor.h \
    src/application/services/chat_app_service.h \
    src/application/services/kb_app_service.h \
    src/application/services/skill_app_service.h \
    src/infrastructure/rendering/simple_markdown_renderer.h \
    src/infrastructure/skill/skill_loader.h \
    src/infrastructure/skill/builtin_skills.h \
    src/presentation/main_window.h \
    src/presentation/chat_widget.h \
    src/presentation/skill_panel.h \
    src/presentation/kb_panel.h

# 复制 data 目录到构建目录
data.files     = data
data.path      = .
INSTALLS      += data

# Windows
win32: RC_ICONS += resources/icon.ico
