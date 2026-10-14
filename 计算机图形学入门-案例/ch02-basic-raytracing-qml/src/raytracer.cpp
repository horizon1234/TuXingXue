#include "raytracer.h"

#include <QtGlobal>

#include <cmath>

// 使用接近白色的中性背景，既对应书中示例，也能与 QML 画布边界区分。
const QColor RayTracer::BackgroundColor{248, 250, 252};

QImage RayTracer::render(int width, int height, const std::vector<Sphere> &spheres)
{
    QImage image(width, height, QImage::Format_RGB32);

    // 第 2 章暂时固定相机位置和朝向。方向由视口点本身确定，因此无需相机矩阵。
    const QVector3D cameraOrigin(0.0f, 0.0f, 0.0f);
    const float infinity = std::numeric_limits<float>::infinity();

    for (int screenY = 0; screenY < height; ++screenY) {
        // 直接写扫描线避免逐像素调用 QImage::setPixelColor() 的格式转换开销。
        auto *row = reinterpret_cast<QRgb *>(image.scanLine(screenY));

        // QImage 的 Y 轴向下，而书中的画布以中心为原点且 Y 轴向上。
        const int canvasY = height / 2 - screenY;

        for (int screenX = 0; screenX < width; ++screenX) {
            const int canvasX = screenX - width / 2;
            const QVector3D direction = canvasToViewport(canvasX, canvasY, width, height);

            // t=1 对应视口平面，因此 tMin=1 会忽略相机与视口之间的物体。
            row[screenX] = traceRay(cameraOrigin, direction, 1.0f, infinity, spheres).rgb();
        }
    }

    return image;
}

QVector3D RayTracer::canvasToViewport(int x, int y, int canvasWidth, int canvasHeight)
{
    // 画布和视口中心、轴方向均已对齐，转换只需按各自宽高比例缩放。
    // 视口嵌在三维空间的 z=d 平面上，所以返回值补上固定的 Z 分量。
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
    // 将射线 P(t)=O+tD 代入球体方程 (P-C)·(P-C)=r²，得到：
    //   (D·D)t² + 2((O-C)·D)t + ((O-C)·(O-C)-r²) = 0
    // 下列 a、b、c 与这个标准二次方程逐项对应。
    const QVector3D cameraToCenter = origin - sphere.center;
    const float a = QVector3D::dotProduct(direction, direction);
    const float b = 2.0f * QVector3D::dotProduct(cameraToCenter, direction);
    const float c = QVector3D::dotProduct(cameraToCenter, cameraToCenter)
                    - sphere.radius * sphere.radius;
    const float discriminant = b * b - 4.0f * a * c;

    // 判别式小于零表示直线未穿过球面。用正无穷作为统一的“无交点”哨兵值，
    // 后续范围检查会自然排除它，无需额外的可选类型或状态标志。
    if (discriminant < 0.0f) {
        const float infinity = std::numeric_limits<float>::infinity();
        return {infinity, infinity};
    }

    const float squareRoot = std::sqrt(discriminant);
    const float denominator = 2.0f * a;

    // 两个根分别对应射线进入和离开球体的位置。这里不假设返回顺序，
    // traceRay 会分别检查两个根并选择最小的有效值。
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
    // closestT 同时实现深度比较和物体遮挡：t 越小，交点越靠近射线原点。
    float closestT = std::numeric_limits<float>::infinity();
    const Sphere *closestSphere = nullptr;

    for (const Sphere &sphere : spheres) {
        const auto [t1, t2] = intersectRaySphere(origin, direction, sphere);

        // 两个根都可能有效，例如相机位于球体内部时只有“离开”根位于前方。
        if (t1 >= tMin && t1 <= tMax && t1 < closestT) {
            closestT = t1;
            closestSphere = &sphere;
        }
        if (t2 >= tMin && t2 <= tMax && t2 < closestT) {
            closestT = t2;
            closestSphere = &sphere;
        }
    }

    // 本章尚未引入光照模型，所以命中后直接返回球体固有颜色。
    return closestSphere ? closestSphere->color : BackgroundColor;
}

