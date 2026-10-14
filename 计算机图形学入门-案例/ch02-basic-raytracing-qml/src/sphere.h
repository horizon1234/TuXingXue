#pragma once

#include <QColor>
#include <QString>
#include <QVector3D>

/**
 * @brief 描述光线追踪场景中的一个球体。
 *
 * 本章采用书中的右手坐标约定：相机位于原点并朝 +Z 方向观察，+X 向右，
 * +Y 向上。该结构只保存渲染算法需要的最小数据，不包含光照和材质参数。
 */
struct Sphere
{
    QString name;          ///< 用于 QML 列表显示的可读名称。
    QVector3D center;      ///< 球心在书本场景坐标系中的位置。
    float radius = 1.0f;  ///< 球体半径，单位与场景坐标一致且应大于零。
    QColor color;          ///< 未引入光照前，射线命中球体时直接返回的颜色。
};

