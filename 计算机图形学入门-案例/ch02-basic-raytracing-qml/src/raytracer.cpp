#include "raytracer.h"

#include <QtGlobal>

#include <cmath>

const QColor RayTracer::BackgroundColor{248, 250, 252};

QImage RayTracer::render(int width, int height, const std::vector<Sphere> &spheres)
{
    QImage image(width, height, QImage::Format_RGB32);
    const QVector3D cameraOrigin(0.0f, 0.0f, 0.0f);
    const float infinity = std::numeric_limits<float>::infinity();

    for (int screenY = 0; screenY < height; ++screenY) {
        auto *row = reinterpret_cast<QRgb *>(image.scanLine(screenY));
        const int canvasY = height / 2 - screenY;

        for (int screenX = 0; screenX < width; ++screenX) {
            const int canvasX = screenX - width / 2;
            const QVector3D direction = canvasToViewport(canvasX, canvasY, width, height);
            row[screenX] = traceRay(cameraOrigin, direction, 1.0f, infinity, spheres).rgb();
        }
    }

    return image;
}

QVector3D RayTracer::canvasToViewport(int x, int y, int canvasWidth, int canvasHeight)
{
    return {
        x * ViewportWidth / static_cast<float>(canvasWidth),
        y * ViewportHeight / static_cast<float>(canvasHeight),
        ProjectionDistance
    };
}

std::pair<float, float> RayTracer::intersectRaySphere(const QVector3D &origin,
                                                       const QVector3D &direction,
                                                       const Sphere &sphere)
{
    const QVector3D cameraToCenter = origin - sphere.center;
    const float a = QVector3D::dotProduct(direction, direction);
    const float b = 2.0f * QVector3D::dotProduct(cameraToCenter, direction);
    const float c = QVector3D::dotProduct(cameraToCenter, cameraToCenter)
                    - sphere.radius * sphere.radius;
    const float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0.0f) {
        const float infinity = std::numeric_limits<float>::infinity();
        return {infinity, infinity};
    }

    const float squareRoot = std::sqrt(discriminant);
    const float denominator = 2.0f * a;
    return {
        (-b + squareRoot) / denominator,
        (-b - squareRoot) / denominator
    };
}

QColor RayTracer::traceRay(const QVector3D &origin,
                           const QVector3D &direction,
                           float tMin,
                           float tMax,
                           const std::vector<Sphere> &spheres)
{
    float closestT = std::numeric_limits<float>::infinity();
    const Sphere *closestSphere = nullptr;

    for (const Sphere &sphere : spheres) {
        const auto [t1, t2] = intersectRaySphere(origin, direction, sphere);

        if (t1 >= tMin && t1 <= tMax && t1 < closestT) {
            closestT = t1;
            closestSphere = &sphere;
        }
        if (t2 >= tMin && t2 <= tMax && t2 < closestT) {
            closestT = t2;
            closestSphere = &sphere;
        }
    }

    return closestSphere ? closestSphere->color : BackgroundColor;
}

