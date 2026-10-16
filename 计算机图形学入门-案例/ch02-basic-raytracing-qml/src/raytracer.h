#pragma once

#include "light.h"
#include "sphere.h"

#include <QColor>
#include <QImage>
#include <QVector3D>

#include <limits>
#include <utility>
#include <vector>

/**
 * @brief 实现第 2 章基础求交以及第 3 章漫反射、镜面反射光照模型。
 *
 * 该类无内部可变状态，所有算法均为纯静态函数，因此相同输入必然产生相同输出。
 * 场景使用书中的坐标约定：相机在原点，朝 +Z 方向观察；画布坐标以中心为原点，
 * +X 向右、+Y 向上。渲染复杂度为 O(width * height * sphereCount)。
 */
class RayTracer
{
public:
    /** @brief 用于逐步对照第 2、3 章效果的渲染模式。 */
    enum class LightingMode {
        Unlit = 0,          ///< 第 2 章基线：命中后直接返回固有颜色。
        AmbientOnly,       ///< 仅使用强度 0.2 的环境光。
        PointOnly,         ///< 仅使用强度 0.6、位置 `(2,1,0)` 的点光。
        DirectionalOnly,   ///< 仅使用强度 0.2、方向 `(1,4,4)` 的方向光。
        DiffuseCombined,   ///< 组合三类光源，只计算漫反射。
        SpecularCombined   ///< 组合三类光源，同时计算漫反射和镜面反射。
    };

    static constexpr float ViewportWidth = 1.0f;       ///< 视口的场景空间宽度。
    static constexpr float ViewportHeight = 1.0f;      ///< 视口的场景空间高度。
    static constexpr float ProjectionDistance = 1.0f; ///< 相机原点到视口平面的距离。

    /**
     * @brief 将整个球体场景渲染为一张 RGB 图像。
     * @param width 输出图像宽度，单位为像素，必须大于零。
     * @param height 输出图像高度，单位为像素，必须大于零。
     * @param spheres 当前可交互的三个球体；第 3 章模式会自动追加书中的黄色地面球。
     * @param mode 决定启用哪些光源以及是否计算镜面反射。
     * @return 按屏幕行顺序存储的 `QImage::Format_RGB32` 图像。
     *
     * 每个像素发射一条射线，并使用 `tMin = 1` 忽略视口平面之前的交点。
     */
    static QImage render(int width,
                         int height,
                         const std::vector<Sphere> &spheres,
                         LightingMode mode = LightingMode::Unlit);

    /**
     * @brief 把以画布中心为原点的像素坐标映射到三维视口平面。
     * @param x 画布横坐标，向右为正。
     * @param y 画布纵坐标，向上为正。
     * @param canvasWidth 画布总宽度，单位为像素。
     * @param canvasHeight 画布总高度，单位为像素。
     * @return 视口平面 `z = ProjectionDistance` 上对应的三维点。
     */
    static QVector3D canvasToViewport(int x, int y, int canvasWidth, int canvasHeight);

    /**
     * @brief 求参数射线 `P(t) = origin + t * direction` 与球体的交点参数。
     * @param origin 射线原点。
     * @param direction 射线方向；无需归一化，但不能是零向量。
     * @param sphere 待检测的球体。
     * @return 二次方程的两个根；无交点时两个值均为正无穷。
     *
     * 返回值不负责过滤负数或裁剪区间，调用方应结合 `tMin/tMax` 判断交点
     * 是否位于当前有效射线段内。
     */
    static std::pair<float, float> intersectRaySphere(const QVector3D &origin,
                                                       const QVector3D &direction,
                                                       const Sphere &sphere);

    /**
     * @brief 沿一条射线查找最近球体，并根据当前模式计算其表面颜色。
     * @param origin 射线原点。
     * @param direction 射线方向。
     * @param tMin 可接受交点参数的下界，包含边界。
     * @param tMax 可接受交点参数的上界，包含边界。
     * @param spheres 当前场景内的全部球体。
     * @param lights 当前模式启用的光源集合。
     * @param includeSpecular 是否在漫反射之外叠加镜面反射项。
     * @return 最近球体经过光照后的颜色；没有有效交点时返回背景色。
     */
    static QColor traceRay(const QVector3D &origin,
                           const QVector3D &direction,
                           float tMin,
                           float tMax,
                           const std::vector<Sphere> &spheres,
                           const std::vector<Light> &lights,
                           bool includeSpecular);

    /**
     * @brief 计算一个表面点接收到的环境光、漫反射和可选镜面反射强度之和。
     * @param point 射线与物体表面的交点。
     * @param normal 交点处朝向物体外部的单位法线。
     * @param view 从交点指向相机的视线向量。
     * @param specularExponent 表面的镜面反射指数；负值代表哑光表面。
     * @param lights 参与当前场景计算的全部光源。
     * @param includeSpecular 是否启用镜面反射项。
     * @return 可与物体固有颜色相乘的总光照强度。
     */
    static float computeLighting(const QVector3D &point,
                                 const QVector3D &normal,
                                 const QVector3D &view,
                                 float specularExponent,
                                 const std::vector<Light> &lights,
                                 bool includeSpecular);

private:
    /** @brief 按光照强度缩放 RGB 通道，并将结果限制到 `[0,255]`。 */
    static QColor applyIntensity(const QColor &color, float intensity);

    static const QColor BackgroundColor; ///< 射线未命中任何物体时使用的画布颜色。
};

