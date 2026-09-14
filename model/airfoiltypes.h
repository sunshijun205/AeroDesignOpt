#ifndef AIRFOILTYPES_H
#define AIRFOILTYPES_H

#include <QString>

// 翼型气动分析输入（POD）。*Known 区分“未载入/未设置”与“取值 0”。
struct AirfoilInput {
    QString naca = QStringLiteral("2412");
    double alphaDeg = 4.0;       // 迎角，度
    double reynolds = 3.0e6;     // 雷诺数
    double mach = 0.2;           // 马赫数
    double aspectRatio = 8.0;    // 展弦比

    bool nacaKnown = false;
    bool alphaKnown = false;
    bool reynoldsKnown = false;
    bool machKnown = false;
    bool aspectRatioKnown = false;
};

// 气动分析结果（POD）。
struct AeroResult {
    bool valid = false;
    double cl = 0.0;
    double cd = 0.0;
    double cm = 0.0;
    double lOverD = 0.0;
    double clAlpha = 0.0;   // 升力线斜率，1/rad
    bool stalled = false;
};

#endif
