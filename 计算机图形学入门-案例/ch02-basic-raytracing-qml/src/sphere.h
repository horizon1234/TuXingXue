#pragma once

#include <QColor>
#include <QString>
#include <QVector3D>

struct Sphere
{
    QString name;
    QVector3D center;
    float radius = 1.0f;
    QColor color;
};

