#include "aeroanalysisservice.h"

#include <QtGlobal>
#include <QtMath>

void AeroAnalysisService::parseNaca(const QString &naca, double *camberPct, double *thicknessPct)
{
    double camber = 2.0;
    double thickness = 12.0;
    const QString digits = naca.trimmed();
    if (digits.size() == 4) {
        bool ok1 = false, ok2 = false;
        const int m = digits.mid(0, 1).toInt(&ok1);   // 最大弯度（%弦长）
        const int tt = digits.mid(2, 2).toInt(&ok2);  // 最大厚度（%弦长）
        if (ok1)
            camber = static_cast<double>(m);
        if (ok2)
            thickness = static_cast<double>(tt);
    }
    if (camberPct)
        *camberPct = camber;
    if (thicknessPct)
        *thicknessPct = thickness;
}

AeroResult AeroAnalysisService::run(const AirfoilInput &input, QString *errorMessage) const
{
    AeroResult result;

    if (input.aspectRatio <= 0.0) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("计算失败：展弦比必须大于 0。");
        return result;
    }
    if (input.mach < 0.0 || input.mach >= 1.0) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("计算失败：马赫数须在 [0, 1) 区间（本模型仅适用亚声速）。");
        return result;
    }

    double camberPct = 0.0;
    double thicknessPct = 12.0;
    parseNaca(input.naca, &camberPct, &thicknessPct);

    // 零升迎角：正弯度产生负零升迎角（示意经验式）。
    const double alpha0Deg = -1.1 * camberPct;
    const double alphaRad = qDegreesToRadians(input.alphaDeg - alpha0Deg);

    // 有限翼升力线斜率修正：a = a0 / (1 + a0/(pi e AR))。
    const double a0 = 2.0 * M_PI;
    const double e = 0.9;
    double a = a0 / (1.0 + a0 / (M_PI * e * input.aspectRatio));

    // 亚声速可压缩性 Prandtl-Glauert 修正。
    const double beta = qSqrt(qMax(1.0 - input.mach * input.mach, 1.0e-3));
    a /= beta;

    double cl = a * alphaRad;

    // 失速夹断。
    const double clMax = 1.5;
    if (cl > clMax) {
        cl = clMax;
        result.stalled = true;
    } else if (cl < -clMax) {
        cl = -clMax;
        result.stalled = true;
    }

    // 阻力极曲线：Cd = Cd0 + Cl^2 / (pi e AR)。
    const double cd0 = 0.008 + 0.01 * (thicknessPct / 12.0);
    const double cd = cd0 + (cl * cl) / (M_PI * e * input.aspectRatio);

    result.valid = true;
    result.cl = cl;
    result.cd = cd;
    result.cm = -0.05 - 0.01 * camberPct;
    result.lOverD = (cd > 1.0e-6) ? cl / cd : 0.0;
    result.clAlpha = a;
    return result;
}
