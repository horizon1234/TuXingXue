#pragma once

#include <QVector3D>

/**
 * @brief 区分第 3 章使用的三类理想光源。
 *
 * 环境光没有方向；点光由空间位置定义；方向光由从被照点指向光源的固定方向定义。
 */
enum class LightType
{
    Ambient,
    Point,
    Directional
};

/**
 * @brief 描述光线追踪器使用的一盏白色光源。
 *
 * 第 3 章假设所有光均为白光，并忽略随传播距离产生的衰减，因此每盏灯只需要类型、
 * 强度以及一个位置或方向向量。对于环境光，`vector` 不参与计算。
 */
struct Light
{
    LightType type = LightType::Ambient; ///< 决定 `vector` 的解释方式。
    float intensity = 0.0f;              ///< 光源强度，通常位于 `[0, 1]`。
    QVector3D vector;                    ///< 点光位置或方向光的固定光向量。
};
