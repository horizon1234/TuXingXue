#include "scenecontroller.h"

#include "raytracer.h"

#include <QElapsedTimer>
#include <QtGlobal>

#include <algorithm>

SceneController::SceneController(RayImageProvider *imageProvider, QObject *parent)
    : QObject(parent)
    , m_imageProvider(imageProvider)
{
    m_renderTimer.setSingleShot(true);
    m_renderTimer.setInterval(0);
    connect(&m_renderTimer, &QTimer::timeout, this, &SceneController::renderNow);
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
    const int clampedIndex = std::clamp(index, 0, m_sphereModel.rowCount() - 1);
    if (m_selectedSphere == clampedIndex) {
        return;
    }

    m_selectedSphere = clampedIndex;
    emit selectedSphereChanged();
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
    const QVector3D next(
        std::clamp(current.x() + deltaX, -3.0f, 3.0f),
        std::clamp(current.y() + deltaY, -2.0f, 2.0f),
        std::clamp(current.z() + deltaZ, 1.5f, 8.0f));

    if (m_sphereModel.setCenter(m_selectedSphere, next)) {
        emit selectedPositionChanged();
        scheduleRender();
    }
}

void SceneController::resetScene()
{
    m_sphereModel.resetToDefaults();
    m_selectedSphere = 0;
    emit selectedSphereChanged();
    emit selectedPositionChanged();
    scheduleRender();
}

void SceneController::setSelectedCoordinate(int axis, float value)
{
    QVector3D center = m_sphereModel.sphereAt(m_selectedSphere).center;
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
    if (!m_renderTimer.isActive()) {
        m_renderTimer.start();
    }
}

void SceneController::renderNow()
{
    QElapsedTimer timer;
    timer.start();
    const QImage image = RayTracer::render(RenderWidth, RenderHeight, m_sphereModel.spheres());
    m_renderMilliseconds = timer.nsecsElapsed() / 1'000'000.0;

    m_imageProvider->setImage(image);
    ++m_renderRevision;
    emit renderMillisecondsChanged();
    emit renderRevisionChanged();
}

