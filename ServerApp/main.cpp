#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickStyle>
#include "serverviewmodel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/resource/icon.ico"));

    QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);

    QQmlApplicationEngine engine;

    qmlRegisterSingletonType<AppEnums>("enums", 1, 0, "AppEnums", [](QQmlEngine*, QJSEngine*) -> QObject* {
        return new AppEnums();
    });

    ServerViewModel serverViewModel(&app);

    engine.rootContext()->setContextProperty("viewModel", &serverViewModel);

    QQuickStyle::setStyle("Universal");
    engine.loadFromModule("ServerApp", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
