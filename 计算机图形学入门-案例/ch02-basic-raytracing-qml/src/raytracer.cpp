#include "raytracer.h"

#include <QtGlobal>

#include <algorithm>
#include <cmath>

// 使用接近白色的中性背景，既对应书中示例，也能与 QML 画布边界区分。
const QColor RayTracer::BackgroundColor{248, 250, 252};

namespace {

/**
 * @brief 按演示模式返回原书定义的光源组合。
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
    case RayTracer::LightingMode::Shadows:
    case RayTracer::LightingMode::ReflectionOneBounce:
    case RayTracer::LightingMode::ReflectionRecursive:
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

    // 第 3、4 章用一个巨大球体模拟平坦地面。它只进入 CPU 求交集合，不进入
    // 可拖动模型，避免左侧教学场景真的创建一个半径 5000 的 Quick 3D 球体。
    std::vector<Sphere> renderSpheres = spheres;
    if (mode != LightingMode::Unlit) {
        renderSpheres.push_back({QStringLiteral("黄色地面"),
                                 {0.0f, -5001.0f, 0.0f},
                                 5000.0f,
                                 QColor(255, 255, 0),
                                 1000.0f,
                                 0.5f});
    }

    const std::vector<Light> lights = lightsForMode(mode);
    const bool includeSpecular = mode == LightingMode::SpecularCombined
                                 || mode == LightingMode::Shadows
                                 || mode == LightingMode::ReflectionOneBounce
                                 || mode == LightingMode::ReflectionRecursive;
    const bool includeShadows = mode == LightingMode::Shadows
                                || mode == LightingMode::ReflectionOneBounce
                                || mode == LightingMode::ReflectionRecursive;

    // 一层模式用于观察最直接的“物体映在另一个物体上”；最终模式按原书建议追踪
    // 三层，已经足以表现反射的反射，同时保证递归一定终止。
    int recursionDepth = 0;
    if (mode == LightingMode::ReflectionOneBounce) {
        recursionDepth = 1;
    } else if (mode == LightingMode::ReflectionRecursive) {
        recursionDepth = 3;
    }

    // 第 2～4 章均固定相机位置和朝向。方向由视口点本身确定，因此无需相机矩阵。
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
                                    includeSpecular,
                                    includeShadows,
                                    recursionDepth)
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

    // 输入和最终 t 保持 float，以便与 QVector3D、Qt Quick 数据直接配合；二次方程
    // 的中间量使用 double。尤其是半径 5000 的地面球会计算两个约 25,000,000 的
    // 数之差，若全程使用 float，掠射角附近容易因有效位不足产生条纹或自相交噪点。
    const double dx = static_cast<double>(direction.x());
    const double dy = static_cast<double>(direction.y());
    const double dz = static_cast<double>(direction.z());
    const double ocx = static_cast<double>(cameraToCenter.x());
    const double ocy = static_cast<double>(cameraToCenter.y());
    const double ocz = static_cast<double>(cameraToCenter.z());
    const double radius = static_cast<double>(sphere.radius);
    const double a = dx * dx + dy * dy + dz * dz;
    const double b = 2.0 * (ocx * dx + ocy * dy + ocz * dz);
    const double c = ocx * ocx + ocy * ocy + ocz * ocz - radius * radius;
    const double discriminant = b * b - 4.0 * a * c;

    // 判别式小于零表示直线未穿过球面。用正无穷作为统一的“无交点”哨兵值，
    // 后续范围检查会自然排除它，无需额外的可选类型或状态标志。
    if (discriminant < 0.0f) {
        const float infinity = std::numeric_limits<float>::infinity();
        return {infinity, infinity};
    }

    const double squareRoot = std::sqrt(discriminant);
    const double denominator = 2.0 * a;

    // 两个根分别对应射线进入和离开球体的位置。这里不假设返回顺序，
    // traceRay 会分别检查两个根并选择最小的有效值。
    return {
        static_cast<float>((-b + squareRoot) / denominator),
        static_cast<float>((-b - squareRoot) / denominator)
    };
}

RayTracer::Intersection RayTracer::closestIntersection(const QVector3D &origin,
                                                        const QVector3D &direction,
                                                        float tMin,
                                                        float tMax,
                                                        const std::vector<Sphere> &spheres)
{
    Intersection closest;

    for (const Sphere &sphere : spheres) {
        const auto [t1, t2] = intersectRaySphere(origin, direction, sphere);

        // 两个根都可能有效，例如射线原点位于球体内部时，通常只有离开球体的根
        // 落在正方向。区间测试同时适用于主射线、有限点光阴影射线和无限反射射线。
        if (t1 >= tMin && t1 <= tMax && t1 < closest.t) {
            closest.t = t1;
            closest.sphere = &sphere;
        }
        if (t2 >= tMin && t2 <= tMax && t2 < closest.t) {
            closest.t = t2;
            closest.sphere = &sphere;
        }
    }

    return closest;
}

QColor RayTracer::traceRay(const QVector3D &origin,
                           const QVector3D &direction,
                           float tMin,
                           float tMax,
                           const std::vector<Sphere> &spheres,
                           const std::vector<Light> &lights,
                           bool includeSpecular,
                           bool includeShadows,
                           int recursionDepth)
{
    const Intersection closest = closestIntersection(origin, direction, tMin, tMax, spheres);
    if (!closest.sphere) {
        return BackgroundColor;
    }

    // 空光源集合只出现在第 2 章基线模式，此时保留原先的纯色输出行为。
    if (lights.empty()) {
        return closest.sphere->color;
    }

    const QVector3D point = origin + closest.t * direction;
    const QVector3D normal = (point - closest.sphere->center).normalized();
    const QVector3D view = -direction;
    const float intensity = computeLighting(point,
                                            normal,
                                            view,
                                            closest.sphere->specularExponent,
                                            lights,
                                            spheres,
                                            includeSpecular,
                                            includeShadows);
    const QColor localColor = applyIntensity(closest.sphere->color, intensity);

    // `recursionDepth == 0` 是递归的强制出口；反射率为零则是材质本身提供的
    // 自然出口。两者都能避免不必要的次级射线，也防止相对镜面导致无限递归。
    const float reflectivity = std::clamp(closest.sphere->reflectivity, 0.0f, 1.0f);
    if (recursionDepth <= 0 || reflectivity <= 0.0f) {
        return localColor;
    }

    // 入射射线方向 D 指向交点，因此从交点指回相机的 -D（即 view）才是
    // ReflectRay 需要的“向外向量”。反射射线从 P 出发并使用 epsilon 下界，
    // 避免浮点误差让它在 t≈0 处再次命中当前表面。
    const QVector3D reflectedDirection = reflectRay(view, normal);
    const QVector3D reflectedOrigin = offsetRayOrigin(point, normal, reflectedDirection);
    const QColor reflectedColor = traceRay(reflectedOrigin,
                                           reflectedDirection,
                                           SecondaryRayEpsilon,
                                           std::numeric_limits<float>::infinity(),
                                           spheres,
                                           lights,
                                           includeSpecular,
                                           includeShadows,
                                           recursionDepth - 1);

    return blendColors(localColor, reflectedColor, reflectivity);
}

float RayTracer::computeLighting(const QVector3D &point,
                                 const QVector3D &normal,
                                 const QVector3D &view,
                                 float specularExponent,
                                 const std::vector<Light> &lights,
                                 const std::vector<Sphere> &spheres,
                                 bool includeSpecular,
                                 bool includeShadows)
{
    float intensity = 0.0f;

    for (const Light &light : lights) {
        if (light.type == LightType::Ambient) {
            // 环境光是对多次间接反射的常量近似，与表面方向和观察方向都无关。
            intensity += light.intensity;
            continue;
        }

        // 点光的向量随交点变化，且参数 t=1 正好到达光源；方向光在场景中处处
        // 使用同一个向量并被视为无限远，因此它的阴影射线没有有限上界。
        const QVector3D lightVector = light.type == LightType::Point
                                         ? light.vector - point
                                         : light.vector;
        const float shadowTMax = light.type == LightType::Point
                                     ? 1.0f
                                     : std::numeric_limits<float>::infinity();
        const float normalLength = normal.length();
        const float lightLength = lightVector.length();
        if (normalLength <= 0.0f || lightLength <= 0.0f) {
            continue;
        }

        if (includeShadows) {
            // 只要 P 到光源的有效射线段内命中任意球体，这盏灯就不可见。本次循环
            // 直接跳过它的漫反射和镜面高光；环境光不表示具体方向，因此不做遮挡检测。
            const QVector3D shadowOrigin = offsetRayOrigin(point, normal, lightVector);
            const QVector3D shadowDirection = light.type == LightType::Point
                                                  ? light.vector - shadowOrigin
                                                  : lightVector;
            const Intersection blocker = closestIntersection(shadowOrigin,
                                                               shadowDirection,
                                                               SecondaryRayEpsilon,
                                                               shadowTMax,
                                                               spheres);
            if (blocker.sphere) {
                continue;
            }
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
        const QVector3D reflection = reflectRay(lightVector, normal);
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

QVector3D RayTracer::reflectRay(const QVector3D &ray, const QVector3D &normal)
{
    // `normal` 是单位向量时，dot(N,R) * N 就是 R 的法向投影。将法向分量
    // 保留、切向分量取反，得到关于法线对称的向量；该形式同时服务高光和递归反射。
    return 2.0f * normal * QVector3D::dotProduct(normal, ray) - ray;
}

QVector3D RayTracer::offsetRayOrigin(const QVector3D &point,
                                     const QVector3D &normal,
                                     const QVector3D &direction)
{
    // 射线向表面外侧发射时沿 +N 偏移，向内侧发射时沿 -N 偏移。符号不能固定，
    // 否则将来加入折射射线后，原点可能反而被推到错误的一侧。
    const float side = QVector3D::dotProduct(direction, normal) >= 0.0f ? 1.0f : -1.0f;
    return point + side * SecondaryRayEpsilon * normal;
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

QColor RayTracer::blendColors(const QColor &localColor,
                              const QColor &reflectedColor,
                              float reflectivity)
{
    const float reflectedWeight = std::clamp(reflectivity, 0.0f, 1.0f);
    const float localWeight = 1.0f - reflectedWeight;

    // 原书按每个 RGB 通道执行 C=(1-r)Clocal+rCreflected。这里显式四舍五入并
    // 限幅，使浮点混合结果稳定地落回 QColor 的 8 位通道范围。
    const auto mixedChannel = [localWeight, reflectedWeight](int local, int reflected) {
        const float mixed = localWeight * static_cast<float>(local)
                            + reflectedWeight * static_cast<float>(reflected);
        return std::clamp(static_cast<int>(std::lround(mixed)), 0, 255);
    };

    return QColor(mixedChannel(localColor.red(), reflectedColor.red()),
                  mixedChannel(localColor.green(), reflectedColor.green()),
                  mixedChannel(localColor.blue(), reflectedColor.blue()));
}

