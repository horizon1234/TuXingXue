#pragma once

#include "rayimageprovider.h"
#include "spheremodel.h"

#include <QAbstractItemModel>
#include <QObject>
#include <QTimer>

/**
 * @brief 协调 QML 交互、球体模型和 CPU 光线追踪结果。
 *
 * 该对象作为上下文属性 `sceneController` 暴露给 QML。它负责验证用户输入、
 * 合并高频位置变化、触发重新渲染，并在新帧写入 `RayImageProvider` 后递增
 * `renderRevision`，使 QML `Image` 刷新 URL。对象及模型工作在 GUI 线程；
 * 图像提供器内部自行处理潜在的跨线程读取。
 */
class SceneController final : public QObject
{
    Q_OBJECT

    // QML 只读模型以及当前选择状态。
    Q_PROPERTY(QAbstractItemModel *sphereModel READ sphereModel CONSTANT)
    Q_PROPERTY(int selectedSphere READ selectedSphere WRITE setSelectedSphere NOTIFY selectedSphereChanged)
    Q_PROPERTY(float selectedX READ selectedX WRITE setSelectedX NOTIFY selectedPositionChanged)
    Q_PROPERTY(float selectedY READ selectedY WRITE setSelectedY NOTIFY selectedPositionChanged)
    Q_PROPERTY(float selectedZ READ selectedZ WRITE setSelectedZ NOTIFY selectedPositionChanged)

    // 渲染状态。revision 用于使 image:// URL 失效，耗时用于界面性能反馈。
    Q_PROPERTY(int renderRevision READ renderRevision NOTIFY renderRevisionChanged)
    Q_PROPERTY(double renderMilliseconds READ renderMilliseconds NOTIFY renderMillisecondsChanged)
    Q_PROPERTY(int renderWidth READ renderWidth CONSTANT)
    Q_PROPERTY(int renderHeight READ renderHeight CONSTANT)

public:
    /**
     * @brief 创建控制器并立即生成第一帧。
     * @param imageProvider 非空图像提供器；其生命周期由 QML 引擎管理，并长于控制器。
     * @param parent 可选 QObject 父对象。
     */
    explicit SceneController(RayImageProvider *imageProvider, QObject *parent = nullptr);

    /** @return 暴露给 QML 的球体列表模型。 */
    QAbstractItemModel *sphereModel();

    /** @return 当前选中球体的模型行索引。 */
    int selectedSphere() const;

    /** @brief 选择球体；越界索引会被限制到模型的有效范围。 */
    void setSelectedSphere(int index);

    /** @return 当前选中球体的 X 坐标。 */
    float selectedX() const;
    /** @return 当前选中球体的 Y 坐标。 */
    float selectedY() const;
    /** @return 当前选中球体的 Z 坐标。 */
    float selectedZ() const;

    /** @brief 设置当前球体 X 坐标，取值限制为 `[-3, 3]`。 */
    void setSelectedX(float value);
    /** @brief 设置当前球体 Y 坐标，取值限制为 `[-2, 2]`。 */
    void setSelectedY(float value);
    /** @brief 设置当前球体 Z 坐标，取值限制为 `[1.5, 8]`。 */
    void setSelectedZ(float value);

    /** @return 已发布渲染帧的递增版本号。 */
    int renderRevision() const;
    /** @return 最近一帧的 CPU 渲染耗时，单位为毫秒。 */
    double renderMilliseconds() const;
    /** @return 固定渲染宽度，单位为像素。 */
    int renderWidth() const;
    /** @return 固定渲染高度，单位为像素。 */
    int renderHeight() const;

    /**
     * @brief 按场景单位增量移动当前球体，并限制到演示场景的可操作范围。
     * @param deltaX X 方向增量。
     * @param deltaY Y 方向增量。
     * @param deltaZ Z 方向增量。
     */
    Q_INVOKABLE void moveSelectedBy(float deltaX, float deltaY, float deltaZ);

    /** @brief 恢复默认场景并重新选择第一个球体。 */
    Q_INVOKABLE void resetScene();

signals:
    /** @brief 当前球体索引发生变化。 */
    void selectedSphereChanged();
    /** @brief 当前球体位置或当前选择发生变化，需要刷新三个坐标绑定。 */
    void selectedPositionChanged();
    /** @brief 新图像已经写入 provider，QML 可以请求新版本。 */
    void renderRevisionChanged();
    /** @brief 最近一次渲染耗时已经更新。 */
    void renderMillisecondsChanged();

private:
    /**
     * @brief 设置当前球体的单个坐标分量。
     * @param axis `0/1/2` 分别表示 `X/Y/Z`。
     * @param value 请求值；函数会按各坐标允许范围限幅。
     */
    void setSelectedCoordinate(int axis, float value);

    /** @brief 在当前事件循环末尾安排一次渲染，并合并重复请求。 */
    void scheduleRender();

    /** @brief 同步计算完整图像、发布帧并通知 QML。 */
    void renderNow();

    static constexpr int RenderWidth = 512;  ///< CPU 光线追踪画布宽度。
    static constexpr int RenderHeight = 512; ///< CPU 光线追踪画布高度。

    RayImageProvider *m_imageProvider = nullptr; ///< 非拥有指针；所有权属于 QML 引擎。
    SphereModel m_sphereModel;                  ///< 场景数据及 QML 列表模型。
    QTimer m_renderTimer;                       ///< 合并滑块和拖动产生的高频渲染请求。
    int m_selectedSphere = 0;                   ///< 当前选中的模型行。
    int m_renderRevision = 0;                   ///< 已发布帧的单调递增版本号。
    double m_renderMilliseconds = 0.0;          ///< 最近一次 CPU 渲染耗时。
};

