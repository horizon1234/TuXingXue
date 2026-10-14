#include "rayimageprovider.h"
#include "scenecontroller.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTimer>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("第二章基础光线追踪"));
    QCoreApplication::setOrganizationName(QStringLiteral("GraphicsStudy"));
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("《计算机图形学入门》第2章交互演示"));
    parser.addHelpOption();
    const QCommandLineOption screenshotOption(
        QStringLiteral("screenshot"),
        QStringLiteral("启动后保存窗口截图并退出。"),
        QStringLiteral("path"));
    parser.addOption(screenshotOption);
    parser.process(app);

    QQmlApplicationEngine engine;
    auto *imageProvider = new RayImageProvider;
    SceneController sceneController(imageProvider);
    engine.addImageProvider(QStringLiteral("raytracer"), imageProvider);
    engine.rootContext()->setContextProperty(QStringLiteral("sceneController"), &sceneController);

    engine.loadFromModule(QStringLiteral("Chapter02RayTracing"), QStringLiteral("Main"));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    if (parser.isSet(screenshotOption)) {
        const QString screenshotPath = QFileInfo(parser.value(screenshotOption)).absoluteFilePath();
        QDir().mkpath(QFileInfo(screenshotPath).absolutePath());
        auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
        QTimer::singleShot(1800, window, [window, screenshotPath] {
            const bool saved = window && window->grabWindow().save(screenshotPath);
            QCoreApplication::exit(saved ? 0 : 2);
        });
    }

    return app.exec();
}
