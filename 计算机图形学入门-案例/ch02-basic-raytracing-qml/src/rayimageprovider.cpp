#include "rayimageprovider.h"

#include <QMutexLocker>

RayImageProvider::RayImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QImage RayImageProvider::requestImage(const QString &, QSize *size, const QSize &requestedSize)
{
    // QML 图像加载可能不在写入帧的 GUI 线程执行，整个读取过程必须受锁保护。
    QMutexLocker locker(&m_mutex);
    if (size) {
        // 按 Qt image provider 契约返回原始图像尺寸，而不是缩放后的尺寸。
        *size = m_image.size();
    }

    if (requestedSize.isValid() && requestedSize != m_image.size()) {
        // PreserveAspectRatio 防止非正方形请求把球体拉伸成椭圆。
        return m_image.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    // QImage 使用隐式共享；返回副本通常只增加引用计数，不复制整块像素内存。
    return m_image;
}

void RayImageProvider::setImage(const QImage &image)
{
    // 在锁内替换完整帧，读取方不会观察到只写了一部分的图像。
    QMutexLocker locker(&m_mutex);
    m_image = image;
}

