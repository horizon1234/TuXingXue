#include "scenecontroller.h"

#include "raytracer.h"

#include <QElapsedTimer>
#include <QtGlobal>

#include <algorithm>

namespace {

/**
 * @brief 根据相机位置和观察目标建立右、上、前三个正交基向量。
 *
 * 这里使用右手坐标约定：`right = worldUp × forward`，再由 `forward × right`
 * 得到真正与前方向垂直的上方向。这样相机旋转只影响射线方向，不会修改场景数据。
 */
CameraPose lookAtCamera(const QVector3D &position,
                        const QVector3D &target,
                        const QVector3D &worldUp = QVector3D(0.0f, 1.0f, 0.0f))
{
    const QVector3D forward = (target - position).normalized();
    const QVector3D right = QVector3D::crossProduct(worldUp, forward).normalized();
    const QVector3D up = QVector3D::crossProduct(forward, right).normalized();
    return {position, right, up, forward};
}

/**
 * @brief 返回第 5 章用于对比的三个相机姿态。
 *
 * 三个预设都看向球体组的中心附近，保证切换相机时右侧画布仍能看到有意义的内容。
 */
CameraPose cameraForPreset(int preset)
{
    switch (preset) {
    case 1:
        return lookAtCamera({-4.0f, 1.8f, -1.5f}, {0.0f, -0.2f, 4.0f});
    case 2:
        return lookAtCamera({0.0f, 5.0f, -2.0f}, {0.0f, -0.4f, 4.0f});
    case 0:
    default:
        return lookAtCamera({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 4.0f});
    }
}

} // namespace

SceneController::SceneController(RayImageProvider *imageProvider, QObject *parent)
    : QObject(parent)
    , m_imageProvider(imageProvider)
{
    // 位置滑块和鼠标拖动会在一个事件循环内连续产生多个值。单次、零间隔定时器
    // 把这些变化合并为一帧，避免每个输入事件都立即执行 512x512 次射线追踪。
    m_renderTimer.setSingleShot(true);
    m_renderTimer.setInterval(0);
    connect(&m_renderTimer, &QTimer::timeout, this, &SceneController::renderNow);

    // QML 首次解析 Image.source 前必须已有可请求的图像。
    renderNow();
}

QAbstractItemModel *SceneController::sphereModel()
{
    return &m_sphereModel;
}

int SceneController::selectedSphere() const
{
    return m_selectedSphere;
}

void SceneController::setSelectedSphere(int index)
{
    // QML ComboBox、三维拾取和模型重置都可能改变选择；在边界处统一校验索引。
    const int clampedIndex = std::clamp(index, 0, m_sphereModel.rowCount() - 1);
    if (m_selectedSphere == clampedIndex) {
        return;
    }

    m_selectedSphere = clampedIndex;
    emit selectedSphereChanged();

    // 选择变化虽然没有修改球体，但 X/Y/Z 属性现在指向另一行，必须全部刷新。
    emit selectedPositionChanged();
}

float SceneController::selectedX() const
{
    return m_sphereModel.sphereAt(m_selectedSphere).center.x();
}

float SceneController::selectedY() const
{
    return m_sphereModel.sphereAt(m_selectedSphere).center.y();
}

float SceneController::selectedZ() const
{
    return m_sphereModel.sphereAt(m_selectedSphere).center.z();
}

void SceneController::setSelectedX(float value)
{
    setSelectedCoordinate(0, value);
}

void SceneController::setSelectedY(float value)
{
    setSelectedCoordinate(1, value);
}

void SceneController::setSelectedZ(float value)
{
    setSelectedCoordinate(2, value);
}

QStringList SceneController::lightingModeNames() const
{
    // 顺序必须与 RayTracer::LightingMode 保持一致，QML ComboBox 的索引会直接传回控制器。
    return {
        QStringLiteral("第2章 · 纯色基线"),
        QStringLiteral("环境光 · I=0.2"),
        QStringLiteral("点光 · I=0.6"),
        QStringLiteral("方向光 · I=0.2"),
        QStringLiteral("漫反射 · 三光源"),
        QStringLiteral("镜面高光 · 完整模型"),
        QStringLiteral("第4章 · 阴影射线"),
        QStringLiteral("第4章 · 一次反射"),
        QStringLiteral("第4章 · 三层递归反射")
    };
}

int SceneController::lightingMode() const
{
    return m_lightingMode;
}

void SceneController::setLightingMode(int mode)
{
    constexpr int firstMode = static_cast<int>(RayTracer::LightingMode::Unlit);
    constexpr int lastMode = static_cast<int>(RayTracer::LightingMode::ReflectionRecursive);
    const int clampedMode = std::clamp(mode, firstMode, lastMode);
    if (m_lightingMode == clampedMode) {
        return;
    }

    m_lightingMode = clampedMode;
    emit lightingModeChanged();
    scheduleRender();
}

QStringList SceneController::cameraPresetNames() const
{
    return {
        QStringLiteral("相机 · 默认正视"),
        QStringLiteral("相机 · 左前方"),
        QStringLiteral("相机 · 俯视")
    };
}

int SceneController::cameraPreset() const
{
    return m_cameraPreset;
}

void SceneController::setCameraPreset(int preset)
{
    const int clampedPreset = std::clamp(preset, 0, 2);
    if (m_cameraPreset == clampedPreset) {
        return;
    }

    m_cameraPreset = clampedPreset;
    emit cameraPresetChanged();
    scheduleRender();
}

QStringList SceneController::samplingModeNames() const
{
    return {
        QStringLiteral("采样 · 1×1"),
        QStringLiteral("采样 · 2×2 超采样")
    };
}

int SceneController::samplingMode() const
{
    return m_samplingMode;
}

void SceneController::setSamplingMode(int mode)
{
    const int clampedMode = std::clamp(mode, 0, 1);
    if (m_samplingMode == clampedMode) {
        return;
    }

    m_samplingMode = clampedMode;
    emit samplingModeChanged();
    scheduleRender();
}

int SceneController::renderRevision() const
{
    return m_renderRevision;
}

double SceneController::renderMilliseconds() const
{
    return m_renderMilliseconds;
}

int SceneController::renderWidth() const
{
    return RenderWidth;
}

int SceneController::renderHeight() const
{
    return RenderHeight;
}

void SceneController::moveSelectedBy(float deltaX, float deltaY, float deltaZ)
{
    const QVector3D current = m_sphereModel.sphereAt(m_selectedSphere).center;

    // 限制范围不是光线追踪算法要求，而是为了让球体始终停留在教学场景和滑块
    // 可以表达的区域内。Z 下限位于视口平面之后，避免球体包围相机。
    const QVector3D next(
        std::clamp(current.x() + deltaX, -3.0f, 3.0f),
        std::clamp(current.y() + deltaY, -2.0f, 2.0f),
        std::clamp(current.z() + deltaZ, 1.5f, 8.0f));

    if (m_sphereModel.setCenter(m_selectedSphere, next)) {
        // 模型通知负责更新左侧 3D 位置；属性通知负责同步下方三个滑块。
        emit selectedPositionChanged();
        scheduleRender();
    }
}

void SceneController::resetScene()
{
    m_sphereModel.resetToDefaults();
    m_selectedSphere = 0;

    // 模型重置不会自动改变控制器属性，显式通知 QML 重新读取选择和坐标。
    emit selectedSphereChanged();
    emit selectedPositionChanged();
    scheduleRender();
}

void SceneController::setSelectedCoordinate(int axis, float value)
{
    QVector3D center = m_sphereModel.sphereAt(m_selectedSphere).center;

    // 所有绝对坐标设置都经过同一个入口，保证滑块赋值和其他调用遵循相同边界。
    if (axis == 0) {
        center.setX(std::clamp(value, -3.0f, 3.0f));
    } else if (axis == 1) {
        center.setY(std::clamp(value, -2.0f, 2.0f));
    } else {
        center.setZ(std::clamp(value, 1.5f, 8.0f));
    }

    if (m_sphereModel.setCenter(m_selectedSphere, center)) {
        emit selectedPositionChanged();
        scheduleRender();
    }
}

void SceneController::scheduleRender()
{
    // 定时器已经激活时无需重新启动；现有回调会读取模型中的最新位置。
    if (!m_renderTimer.isActive()) {
        m_renderTimer.start();
    }
}

void SceneController::renderNow()
{
    QElapsedTimer timer;
    timer.start();

    // 当前示例同步渲染，足以保持实现与书中伪代码一一对应。第 5 章的 2×2
    // 超采样会按样本数增加耗时，因此界面同时显示最近一帧耗时供比较。
    // 场景复杂后可把这一调用迁移到工作线程，但不得在工作线程直接访问 QML 对象。
    const auto lightingMode = static_cast<RayTracer::LightingMode>(m_lightingMode);
    const CameraPose camera = cameraForPreset(m_cameraPreset);
    const int sampleGrid = m_samplingMode == 1 ? 2 : 1;
    const QImage image = RayTracer::render(RenderWidth,
                                           RenderHeight,
                                           m_sphereModel.spheres(),
                                           lightingMode,
                                           camera,
                                           sampleGrid);
    m_renderMilliseconds = timer.nsecsElapsed() / 1'000'000.0;

    // 必须先发布完整图像，再递增 URL 版本。否则 QML 可能以新 URL 请求到旧帧。
    m_imageProvider->setImage(image);
    ++m_renderRevision;
    emit renderMillisecondsChanged();
    emit renderRevisionChanged();
}

