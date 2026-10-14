#pragma once

#include "sphere.h"

#include <QAbstractListModel>

#include <vector>

/**
 * @brief 把 C++ 球体集合适配为 QML 可直接绑定的只增量更新列表模型。
 *
 * 每一行对应一个 `Sphere`。自定义角色把球心三个分量、半径和颜色分别暴露给
 * `Repeater3D`，从而在球体移动时只通知相关角色变化，无需重建整个三维场景。
 * 模型对象由 `SceneController` 持有，所有访问都发生在 GUI 线程。
 */
class SphereModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    /** @brief QML 委托可通过名称访问的数据角色。 */
    enum Role {
        NameRole = Qt::UserRole + 1, ///< 球体显示名称。
        CenterXRole,                ///< 球心 X 坐标。
        CenterYRole,                ///< 球心 Y 坐标。
        CenterZRole,                ///< 球心 Z 坐标。
        RadiusRole,                 ///< 球体半径。
        SphereColorRole,            ///< 球体基础颜色。
        SphereIndexRole             ///< 稳定的模型行索引，用于拾取后反查球体。
    };

    /** @brief 创建模型并载入书中第 2 章的三个默认球体。 */
    explicit SphereModel(QObject *parent = nullptr);

    /** @copydoc QAbstractItemModel::rowCount */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /** @copydoc QAbstractItemModel::data */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /** @return 自定义角色编号到 QML 属性名的映射。 */
    QHash<int, QByteArray> roleNames() const override;

    /** @return 全部球体的只读连续集合，供 CPU 渲染器遍历。 */
    const std::vector<Sphere> &spheres() const;

    /**
     * @brief 返回指定行的球体。
     * @param index 有效范围为 `[0, rowCount())`；调用方负责保证索引有效。
     */
    const Sphere &sphereAt(int index) const;

    /**
     * @brief 修改一个球体的球心并发送精确的 `dataChanged` 通知。
     * @param index 要修改的模型行。
     * @param center 新球心。
     * @return 数据实际发生变化时返回 `true`；索引无效或值相同时返回 `false`。
     */
    bool setCenter(int index, const QVector3D &center);

    /** @brief 恢复书中红、蓝、绿三个球体的初始位置、半径和颜色。 */
    void resetToDefaults();

private:
    std::vector<Sphere> m_spheres; ///< 模型行与渲染器共享的唯一场景数据源。
};

