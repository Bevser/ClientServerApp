#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickStyle>
#include "serverviewmodel.h"
#include "serverfactory.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQuickWindow::setGraphicsApi(QSGRendererInterface::Vulkan);

    QQmlApplicationEngine engine;

    qmlRegisterSingletonType<AppEnums>("enums", 1, 0, "AppEnums", [](QQmlEngine*, QJSEngine*) -> QObject* {
        return new AppEnums();
    });

    IServer* tcpServer = ServerFactory::createServer(ServerType::TCP);

    ServerViewModel serverViewModel(tcpServer);

    engine.rootContext()->setContextProperty("viewModel", &serverViewModel);

    QQuickStyle::setStyle("Universal");
    engine.loadFromModule("ServerApp", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
