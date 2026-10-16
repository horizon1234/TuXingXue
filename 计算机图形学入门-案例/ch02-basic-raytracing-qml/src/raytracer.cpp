#include "raytracer.h"

#include <QtGlobal>

#include <algorithm>
#include <cmath>

// 使用接近白色的中性背景，既对应书中示例，也能与 QML 画布边界区分。
const QColor RayTracer::BackgroundColor{248, 250, 252};

namespace {

/**
 * @brief 按演示模式返回第 3 章原书定义的光源组合。
 *
 * 光向量遵循原书约定：方向光的向量从表面点指向光源，而不是光传播的方向。
 */
std::vector<Light> lightsForMode(RayTracer::LightingMode mode)
{
    const Light ambient{LightType::Ambient, 0.2f, {}};
    const Light point{LightType::Point, 0.6f, {2.0f, 1.0f, 0.0f}};
    const Light directional{LightType::Directional, 0.2f, {1.0f, 4.0f, 4.0f}};

    switch (mode) {
    case RayTracer::LightingMode::AmbientOnly:
        return {ambient};
    case RayTracer::LightingMode::PointOnly:
        return {point};
    case RayTracer::LightingMode::DirectionalOnly:
        return {directional};
    case RayTracer::LightingMode::DiffuseCombined:
    case RayTracer::LightingMode::SpecularCombined:
        return {ambient, point, directional};
    case RayTracer::LightingMode::Unlit:
        return {};
    }

    return {};
}

} // namespace

QImage RayTracer::render(int width,
                         int height,
                         const std::vector<Sphere> &spheres,
                         LightingMode mode)
{
    QImage image(width, height, QImage::Format_RGB32);

    // 第 3 章用一个巨大球体模拟平坦地面。它只进入 CPU 求交集合，不进入可拖动模型，
    // 避免左侧教学场景真的创建一个半径 5000 的 Quick 3D 球体。
    std::vector<Sphere> renderSpheres = spheres;
    if (mode != LightingMode::Unlit) {
        renderSpheres.push_back({QStringLiteral("黄色地面"),
                                 {0.0f, -5001.0f, 0.0f},
                                 5000.0f,
                                 QColor(255, 255, 0),
                                 1000.0f});
    }

    const std::vector<Light> lights = lightsForMode(mode);
    const bool includeSpecular = mode == LightingMode::SpecularCombined;

    // 第 2、3 章均固定相机位置和朝向。方向由视口点本身确定，因此无需相机矩阵。
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
            row[screenX] = traceRay(cameraOrigin,
                                    direction,
                                    1.0f,
                                    infinity,
                                    renderSpheres,
                                    lights,
                                    includeSpecular)
                               .rgb();
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
                           const std::vector<Sphere> &spheres,
                           const std::vector<Light> &lights,
                           bool includeSpecular)
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

    if (!closestSphere) {
        return BackgroundColor;
    }

    // 空光源集合只出现在第 2 章基线模式，此时保留原先的纯色输出行为。
    if (lights.empty()) {
        return closestSphere->color;
    }

    const QVector3D point = origin + closestT * direction;
    const QVector3D normal = (point - closestSphere->center).normalized();
    const QVector3D view = -direction;
    const float intensity = computeLighting(point,
                                            normal,
                                            view,
                                            closestSphere->specularExponent,
                                            lights,
                                            includeSpecular);
    return applyIntensity(closestSphere->color, intensity);
}

float RayTracer::computeLighting(const QVector3D &point,
                                 const QVector3D &normal,
                                 const QVector3D &view,
                                 float specularExponent,
                                 const std::vector<Light> &lights,
                                 bool includeSpecular)
{
    float intensity = 0.0f;

    for (const Light &light : lights) {
        if (light.type == LightType::Ambient) {
            // 环境光是对多次间接反射的常量近似，与表面方向和观察方向都无关。
            intensity += light.intensity;
            continue;
        }

        // 点光的向量随交点变化；方向光在场景中处处使用同一个光向量。
        const QVector3D lightVector = light.type == LightType::Point
                                         ? light.vector - point
                                         : light.vector;
        const float normalLength = normal.length();
        const float lightLength = lightVector.length();
        if (normalLength <= 0.0f || lightLength <= 0.0f) {
            continue;
        }

        // Lambert 漫反射项：夹角越接近 0°，单位面积接收的光能越多。
        const float normalDotLight = QVector3D::dotProduct(normal, lightVector);
        if (normalDotLight > 0.0f) {
            intensity += light.intensity * normalDotLight / (normalLength * lightLength);
        }

        if (!includeSpecular || specularExponent < 0.0f) {
            continue;
        }

        // 原书的镜面模型先求光向量关于法线的反射 R，再比较 R 与视线 V。
        const QVector3D reflection = 2.0f * normal * normalDotLight - lightVector;
        const float reflectionDotView = QVector3D::dotProduct(reflection, view);
        const float reflectionLength = reflection.length();
        const float viewLength = view.length();
        if (reflectionDotView > 0.0f && reflectionLength > 0.0f && viewLength > 0.0f) {
            // 理论上的余弦位于 `[0,1]`；限幅可消除浮点舍入导致的轻微越界。
            const float cosine = std::clamp(reflectionDotView / (reflectionLength * viewLength),
                                            0.0f,
                                            1.0f);
            intensity += light.intensity * std::pow(cosine, specularExponent);
        }
    }

    return intensity;
}

QColor RayTracer::applyIntensity(const QColor &color, float intensity)
{
    const auto scaledChannel = [intensity](int channel) {
        return std::clamp(static_cast<int>(std::lround(channel * intensity)), 0, 255);
    };

    return QColor(scaledChannel(color.red()),
                  scaledChannel(color.green()),
                  scaledChannel(color.blue()));
}

