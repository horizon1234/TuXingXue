#pragma once

#include <QImage>
#include <QMutex>
#include <QQuickImageProvider>

class RayImageProvider final : public QQuickImageProvider
{
public:
    RayImageProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    void setImage(const QImage &image);

private:
    QMutex m_mutex;
    QImage m_image;
};

