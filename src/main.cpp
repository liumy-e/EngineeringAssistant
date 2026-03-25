#include <QApplication>
#include <QFont>
#include "presentation/main_window.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("工程图纸设计助手");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Engineering");

    // 设置全局字体（中文优化）
    QFont font("Microsoft YaHei", 9);
    font.setStyleHint(QFont::SansSerif);
    app.setFont(font);

    MainWindow window;
    window.show();

    return app.exec();
}
