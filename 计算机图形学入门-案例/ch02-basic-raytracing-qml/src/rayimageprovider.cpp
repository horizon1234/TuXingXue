#include "rayimageprovider.h"

#include <QMutexLocker>

RayImageProvider::RayImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QImage RayImageProvider::requestImage(const QString &, QSize *size, const QSize &requestedSize)
{
    QMutexLocker locker(&m_mutex);
    if (size) {
        *size = m_image.size();
    }

    if (requestedSize.isValid() && requestedSize != m_image.size()) {
        return m_image.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return m_image;
}

void RayImageProvider::setImage(const QImage &image)
{
    QMutexLocker locker(&m_mutex);
    m_image = image;
}

