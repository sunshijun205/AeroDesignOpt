#ifndef AEROANALYSISSERVICE_H
#define AEROANALYSISSERVICE_H

#include "airfoiltypes.h"

#include <QString>

// 翼型气动分析用例：工程级简化模型（薄翼理论 + 阻力极曲线）。
// 非高保真，可作为后续接入面元法 / CFD 的接口占位。
class AeroAnalysisService
{
public:
    // 单点计算。输入非法（如展弦比<=0）时返回 valid=false，并写 errorMessage。
    AeroResult run(const AirfoilInput &input, QString *errorMessage = nullptr) const;

private:
    // 从 4 位 NACA 编号提取百分弯度与百分厚度（示意）。
    static void parseNaca(const QString &naca, double *camberPct, double *thicknessPct);
};

#endif
