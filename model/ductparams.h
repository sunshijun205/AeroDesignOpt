#ifndef DUCTPARAMS_H
#define DUCTPARAMS_H

#include <QString>
#include <QVector>

// 单条截面（rib）参数（与 design.json 的 free_variables.ribs 同构）。
struct DuctRib {
    QString id;
    double spinePos = 0.0;          // 沿中弧线相对位置 0~1
    double scale = 1.0;             // 截面整体缩放
    double zShift = 0.0;            // z 向下沉
    QVector<double> controlPointsZ; // 5 个 z 向控制点（强制 z 对称）
    double controlPointY = 0.0;     // y 向控制点
};

// 统一设计参数（对应需求 M1 几何生成的输入，与 design.json 同构）。
// 入口/出口方向矢量 + 3 条 rib + 固定项，共 27 个自由变量的载体。
struct DuctParams {
    QString caseId;
    QString shapeFamily;
    QString paramVersion;
    QString lengthUnit = QStringLiteral("m");
    QString angleUnit = QStringLiteral("deg");

    // 固定项。
    double inletArea = 0.0;
    double outletArea = 0.0;
    double axialLength = 0.0;
    bool inletRibLocked = true;
    bool outletRibLocked = true;

    // 端部方向矢量（自由变量）。
    double inletPitchDeg = 0.0;
    double inletExtend = 0.0;
    double outletPitchDeg = 0.0;
    double outletExtend = 0.0;

    // 中间 rib（自由变量）。
    QVector<DuctRib> ribs;

    QString catiaReference;
    bool loaded = false;
};

#endif
