#pragma once

#include "rayimageprovider.h"
#include "spheremodel.h"

#include <QAbstractItemModel>
#include <QObject>
#include <QTimer>

class SceneController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel *sphereModel READ sphereModel CONSTANT)
    Q_PROPERTY(int selectedSphere READ selectedSphere WRITE setSelectedSphere NOTIFY selectedSphereChanged)
    Q_PROPERTY(float selectedX READ selectedX WRITE setSelectedX NOTIFY selectedPositionChanged)
    Q_PROPERTY(float selectedY READ selectedY WRITE setSelectedY NOTIFY selectedPositionChanged)
    Q_PROPERTY(float selectedZ READ selectedZ WRITE setSelectedZ NOTIFY selectedPositionChanged)
    Q_PROPERTY(int renderRevision READ renderRevision NOTIFY renderRevisionChanged)
    Q_PROPERTY(double renderMilliseconds READ renderMilliseconds NOTIFY renderMillisecondsChanged)
    Q_PROPERTY(int renderWidth READ renderWidth CONSTANT)
    Q_PROPERTY(int renderHeight READ renderHeight CONSTANT)

public:
    explicit SceneController(RayImageProvider *imageProvider, QObject *parent = nullptr);

    QAbstractItemModel *sphereModel();

    int selectedSphere() const;
    void setSelectedSphere(int index);

    float selectedX() const;
    float selectedY() const;
    float selectedZ() const;
    void setSelectedX(float value);
    void setSelectedY(float value);
    void setSelectedZ(float value);

    int renderRevision() const;
    double renderMilliseconds() const;
    int renderWidth() const;
    int renderHeight() const;

    Q_INVOKABLE void moveSelectedBy(float deltaX, float deltaY, float deltaZ);
    Q_INVOKABLE void resetScene();

signals:
    void selectedSphereChanged();
    void selectedPositionChanged();
    void renderRevisionChanged();
    void renderMillisecondsChanged();

private:
    void setSelectedCoordinate(int axis, float value);
    void scheduleRender();
    void renderNow();

    static constexpr int RenderWidth = 512;
    static constexpr int RenderHeight = 512;

    RayImageProvider *m_imageProvider = nullptr;
    SphereModel m_sphereModel;
    QTimer m_renderTimer;
    int m_selectedSphere = 0;
    int m_renderRevision = 0;
    double m_renderMilliseconds = 0.0;
};

