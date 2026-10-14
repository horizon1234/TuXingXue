#pragma once

#include "sphere.h"

#include <QColor>
#include <QImage>
#include <QVector3D>

#include <limits>
#include <utility>
#include <vector>

class RayTracer
{
public:
    static constexpr float ViewportWidth = 1.0f;
    static constexpr float ViewportHeight = 1.0f;
    static constexpr float ProjectionDistance = 1.0f;

    static QImage render(int width, int height, const std::vector<Sphere> &spheres);

    static QVector3D canvasToViewport(int x, int y, int canvasWidth, int canvasHeight);
    static std::pair<float, float> intersectRaySphere(const QVector3D &origin,
                                                       const QVector3D &direction,
                                                       const Sphere &sphere);
    static QColor traceRay(const QVector3D &origin,
                           const QVector3D &direction,
                           float tMin,
                           float tMax,
                           const std::vector<Sphere> &spheres);

private:
    static const QColor BackgroundColor;
};

