#pragma once

#include "sphere.h"

#include <QColor>
#include <QImage>
#include <QVector3D>

#include <limits>
#include <utility>
#include <vector>

/**
 * @brief 实现第 2 章的无光照、纯色球体光线追踪器。
 *
 * 该类无内部可变状态，所有算法均为纯静态函数，因此相同输入必然产生相同输出。
 * 场景使用书中的坐标约定：相机在原点，朝 +Z 方向观察；画布坐标以中心为原点，
 * +X 向右、+Y 向上。渲染复杂度为 O(width * height * sphereCount)。
 */
class RayTracer
{
public:
    static constexpr float ViewportWidth = 1.0f;       ///< 视口的场景空间宽度。
    static constexpr float ViewportHeight = 1.0f;      ///< 视口的场景空间高度。
    static constexpr float ProjectionDistance = 1.0f; ///< 相机原点到视口平面的距离。

    /**
     * @brief 将整个球体场景渲染为一张 RGB 图像。
     * @param width 输出图像宽度，单位为像素，必须大于零。
     * @param height 输出图像高度，单位为像素，必须大于零。
     * @param spheres 当前场景内的全部球体。
     * @return 按屏幕行顺序存储的 `QImage::Format_RGB32` 图像。
     *
     * 每个像素发射一条射线，并使用 `tMin = 1` 忽略视口平面之前的交点。
     */
    static QImage render(int width, int height, const std::vector<Sphere> &spheres);

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
     * @brief 沿一条射线查找有效范围内距离最近的球体颜色。
     * @param origin 射线原点。
     * @param direction 射线方向。
     * @param tMin 可接受交点参数的下界，包含边界。
     * @param tMax 可接受交点参数的上界，包含边界。
     * @param spheres 当前场景内的全部球体。
     * @return 最近球体的纯色；没有有效交点时返回背景色。
     */
    static QColor traceRay(const QVector3D &origin,
                           const QVector3D &direction,
                           float tMin,
                           float tMax,
                           const std::vector<Sphere> &spheres);

private:
    static const QColor BackgroundColor; ///< 射线未命中任何物体时使用的画布颜色。
};

