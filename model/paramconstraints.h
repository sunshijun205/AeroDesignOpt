#ifndef PARAMCONSTRAINTS_H
#define PARAMCONSTRAINTS_H

#include <QString>

// 数值区间 [lo, hi]。
struct RangeD {
    double lo = 0.0;
    double hi = 0.0;
};

// 设计参数的校验约束（来自配置文件 param_ranges.json，或内置默认值）。
// 结构约束取自论文 DuctGen 参数化；数值范围为可配置的取值规则。
struct ParamConstraints {
    int ribCount = 3;
    int cpPerRib = 5;
    bool enforceZSymmetry = true;
    double symmetryTol = 1e-3;

    RangeD inletPitchDeg{-15.0, 15.0};
    RangeD inletExtend{0.0, 1.0};
    RangeD outletPitchDeg{-15.0, 15.0};
    RangeD outletExtend{0.0, 1.0};
    RangeD ribSpinePos{0.0, 1.0};
    RangeD ribScale{0.5, 1.5};
    RangeD ribZShift{-0.5, 0.2};
    RangeD ribCpY{0.0, 0.5};

    QString source;  // 约束来源描述（界面显示用）
};

#endif
