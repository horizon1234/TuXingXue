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

/**
 * @brief 创建 Qt Quick 应用、连接 C++ 场景后端并进入事件循环。
 *
 * 可选参数 `--screenshot <path>` 用于自动化视觉验证：窗口完成首帧渲染后保存
 * 截图并退出；`--lighting-mode <0..5>` 用于直接选择第 2、3 章的渐进场景。
 * 正常启动时程序持续运行，由用户通过 QML 界面切换光照并移动球体。
 */
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("第2至3章光线追踪与光照"));
    QCoreApplication::setOrganizationName(QStringLiteral("GraphicsStudy"));

    // 固定使用跨平台 Basic 样式，避免原生控件风格改变布局尺寸和截图结果。
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    // 截图参数只服务构建验证，不影响正常交互模式。
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("《计算机图形学入门》第2至3章交互演示"));
    parser.addHelpOption();
    const QCommandLineOption screenshotOption(
        QStringLiteral("screenshot"),
        QStringLiteral("启动后保存窗口截图并退出。"),
        QStringLiteral("path"));
    parser.addOption(screenshotOption);

    const QCommandLineOption lightingModeOption(
        QStringList{QStringLiteral("m"), QStringLiteral("lighting-mode")},
        QStringLiteral("选择光照场景，取值范围为 0 到 5。"),
        QStringLiteral("index"));
    parser.addOption(lightingModeOption);
    parser.process(app);

    // 控制器必须早于 engine 构造，使析构顺序变为“先销毁 QML，再销毁控制器”。
    // 这样关闭程序时 QML 绑定不会短暂读到已经失效的上下文属性。
    auto *imageProvider = new RayImageProvider;
    SceneController sceneController(imageProvider);

    if (parser.isSet(lightingModeOption)) {
        bool isNumber = false;
        const int requestedMode = parser.value(lightingModeOption).toInt(&isNumber);
        if (!isNumber || requestedMode < 0 || requestedMode > 5) {
            // 无效自动化参数属于调用错误，不能静默回退到默认场景。
            return 2;
        }
        sceneController.setLightingMode(requestedMode);
    }

    QQmlApplicationEngine engine;

    // addImageProvider() 会把 provider 所有权移交给 engine；控制器只保留非拥有指针。
    engine.addImageProvider(QStringLiteral("raytracer"), imageProvider);

    // 使用上下文属性保持示例直接：QML 可绑定控制器属性，但渲染数学仍完全留在 C++。
    engine.rootContext()->setContextProperty(QStringLiteral("sceneController"), &sceneController);

    engine.loadFromModule(QStringLiteral("Chapter02RayTracing"), QStringLiteral("Main"));
    if (engine.rootObjects().isEmpty()) {
        // QML 语法错误或模块缺失时没有可显示窗口，立即以非零状态退出。
        return -1;
    }

    if (parser.isSet(screenshotOption)) {
        const QString screenshotPath = QFileInfo(parser.value(screenshotOption)).absoluteFilePath();

        // 自动创建目标目录，使 CI 或本地验证可以直接传入尚不存在的 artifacts 路径。
        QDir().mkpath(QFileInfo(screenshotPath).absolutePath());
        auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());

        // 等待 Qt Quick 3D 管线和异步场景资源完成首帧，再抓取整个窗口。
        QTimer::singleShot(1800, window, [window, screenshotPath] {
            const bool saved = window && window->grabWindow().save(screenshotPath);
            QCoreApplication::exit(saved ? 0 : 2);
        });
    }

    return app.exec();
}
