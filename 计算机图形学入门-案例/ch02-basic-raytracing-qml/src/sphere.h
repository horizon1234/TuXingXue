#pragma once

#include <QColor>
#include <QString>
#include <QVector3D>

/**
 * @brief 描述光线追踪场景中的一个球体。
 *
 * 本章采用书中的右手坐标约定：相机位于原点并朝 +Z 方向观察，+X 向右，
 * +Y 向上。第 3 章增加镜面反射指数，用于区分哑光与闪亮表面；第 4 章再增加
 * 反射率，用于混合物体自身的局部光照颜色与递归射线看到的反射颜色。
 */
struct Sphere
{
    QString name;          ///< 用于 QML 列表显示的可读名称。
    QVector3D center;      ///< 球心在书本场景坐标系中的位置。
    float radius = 1.0f;  ///< 球体半径，单位与场景坐标一致且应大于零。
    QColor color;          ///< 物体的固有颜色；最终颜色由它与光照强度逐通道相乘得到。
    float specularExponent = -1.0f; ///< 镜面反射指数；负值表示完全不计算镜面反射。
    float reflectivity = 0.0f;      ///< 反射颜色权重，`0` 表示不反射，`1` 表示理想镜面。
};

