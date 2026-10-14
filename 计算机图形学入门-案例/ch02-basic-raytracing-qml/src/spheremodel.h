#pragma once

#include "sphere.h"

#include <QAbstractListModel>

#include <vector>

class SphereModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        NameRole = Qt::UserRole + 1,
        CenterXRole,
        CenterYRole,
        CenterZRole,
        RadiusRole,
        SphereColorRole,
        SphereIndexRole
    };

    explicit SphereModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    const std::vector<Sphere> &spheres() const;
    const Sphere &sphereAt(int index) const;
    bool setCenter(int index, const QVector3D &center);
    void resetToDefaults();

private:
    std::vector<Sphere> m_spheres;
};

