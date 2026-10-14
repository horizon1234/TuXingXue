#pragma once

#include <QImage>
#include <QMutex>
#include <QQuickImageProvider>

/**
 * @brief 在线程安全的共享缓冲区中保存最新光线追踪图像，并提供给 QML。
 *
 * QML 通过 `image://raytracer/...` 请求图像。虽然当前页面通常同步加载图片，
 * Qt 的图像请求可能由加载线程触发，因此读写 `m_image` 时都必须持有互斥锁。
 * `QQmlApplicationEngine::addImageProvider()` 接管该对象的所有权。
 */
class RayImageProvider final : public QQuickImageProvider
{
public:
    RayImageProvider();

    /**
     * @brief 返回当前完整渲染帧，必要时按请求尺寸等比缩放。
     * @param id 图片 URL 中 provider 名称之后的路径；本例无需区分具体 id。
     * @param size 非空时写入原始帧尺寸。
     * @param requestedSize QML 请求的目标尺寸；无效时返回原始尺寸。
     * @return 可安全跨线程传递的 `QImage` 隐式共享副本。
     */
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    /**
     * @brief 用新渲染结果替换共享帧。
     * @param image 已完成写入、后续只读的图像。
     */
    void setImage(const QImage &image);

private:
    QMutex m_mutex; ///< 保护 `m_image`，协调 GUI 线程与潜在图像加载线程。
    QImage m_image; ///< 最近一次完整渲染的帧。
};

