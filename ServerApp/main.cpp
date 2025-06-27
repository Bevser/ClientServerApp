#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickStyle>
#include "serverviewmodel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQuickWindow::setGraphicsApi(QSGRendererInterface::Vulkan);

    QQmlApplicationEngine engine;

    ServerViewModel serverViewModel;
    engine.rootContext()->setContextProperty("viewModel", &serverViewModel);

    QQuickStyle::setStyle("Universal");

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("ServerApp", "Main");

    return app.exec();
}
