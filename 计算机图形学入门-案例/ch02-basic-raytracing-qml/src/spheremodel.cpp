#include "spheremodel.h"

#include <QColor>

SphereModel::SphereModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // 让模型从构造完成起就始终包含一个可渲染、可选择的有效场景。
    resetToDefaults();
}

int SphereModel::rowCount(const QModelIndex &parent) const
{
    // 这是扁平列表模型，不允许任何行再拥有子项。
    return parent.isValid() ? 0 : static_cast<int>(m_spheres.size());
}

QVariant SphereModel::data(const QModelIndex &index, int role) const
{
    // QML 在委托销毁或模型重置期间可能短暂请求失效索引，返回空值即可。
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount()) {
        return {};
    }

    const Sphere &sphere = m_spheres.at(static_cast<std::size_t>(index.row()));
    switch (role) {
    case Qt::DisplayRole:
    case NameRole:
        return sphere.name;
    case CenterXRole:
        return sphere.center.x();
    case CenterYRole:
        return sphere.center.y();
    case CenterZRole:
        return sphere.center.z();
    case RadiusRole:
        return sphere.radius;
    case SphereColorRole:
        return sphere.color;
    case SphereIndexRole:
        return index.row();
    default:
        return {};
    }
}

QHash<int, QByteArray> SphereModel::roleNames() const
{
    // 这些名称会成为 QML Repeater3D 委托中的 required property 名称。
    return {
        {NameRole, "name"},
        {CenterXRole, "centerX"},
        {CenterYRole, "centerY"},
        {CenterZRole, "centerZ"},
        {RadiusRole, "radius"},
        {SphereColorRole, "sphereColor"},
        {SphereIndexRole, "sphereIndex"}
    };
}

const std::vector<Sphere> &SphereModel::spheres() const
{
    // 返回 const 引用，避免每次渲染复制场景，同时防止渲染器绕过模型通知修改数据。
    return m_spheres;
}

const Sphere &SphereModel::sphereAt(int index) const
{
    return m_spheres.at(static_cast<std::size_t>(index));
}

bool SphereModel::setCenter(int index, const QVector3D &center)
{
    if (index < 0 || index >= rowCount()) {
        return false;
    }

    Sphere &sphere = m_spheres.at(static_cast<std::size_t>(index));
    if (sphere.center == center) {
        return false;
    }

    sphere.center = center;
    const QModelIndex changedIndex = this->index(index);

    // 只通知三个坐标角色，QML 可保留已有 Model 和材质，仅更新 position 绑定。
    emit dataChanged(changedIndex, changedIndex, {CenterXRole, CenterYRole, CenterZRole});
    return true;
}

void SphereModel::resetToDefaults()
{
    // begin/endResetModel 保证 ComboBox 和 Repeater3D 不会在容器替换期间读取旧行。
    beginResetModel();
    m_spheres = {
        // 坐标、半径和颜色对应书中第 2 章的第一个演示场景。
        {QStringLiteral("红色球体"), {0.0f, -1.0f, 3.0f}, 1.0f, QColor(225, 55, 55)},
        {QStringLiteral("蓝色球体"), {2.0f, 0.0f, 4.0f}, 1.0f, QColor(45, 105, 210)},
        {QStringLiteral("绿色球体"), {-2.0f, 0.0f, 4.0f}, 1.0f, QColor(40, 170, 95)}
    };
    endResetModel();
}

