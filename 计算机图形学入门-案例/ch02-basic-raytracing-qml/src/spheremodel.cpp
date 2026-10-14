#include "spheremodel.h"

#include <QColor>

SphereModel::SphereModel(QObject *parent)
    : QAbstractListModel(parent)
{
    resetToDefaults();
}

int SphereModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_spheres.size());
}

QVariant SphereModel::data(const QModelIndex &index, int role) const
{
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
    emit dataChanged(changedIndex, changedIndex, {CenterXRole, CenterYRole, CenterZRole});
    return true;
}

void SphereModel::resetToDefaults()
{
    beginResetModel();
    m_spheres = {
        {QStringLiteral("红色球体"), {0.0f, -1.0f, 3.0f}, 1.0f, QColor(225, 55, 55)},
        {QStringLiteral("蓝色球体"), {2.0f, 0.0f, 4.0f}, 1.0f, QColor(45, 105, 210)},
        {QStringLiteral("绿色球体"), {-2.0f, 0.0f, 4.0f}, 1.0f, QColor(40, 170, 95)}
    };
    endResetModel();
}

