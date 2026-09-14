#include "ductdesignservice.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtGlobal>
#include <QtMath>

bool DuctDesignService::load(const QString &path, DuctParams &out, QString &error) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = QString::fromUtf8("无法读取设计参数文件：%1").arg(path);
        return false;
    }
    const QByteArray raw = file.readAll();
    file.close();

    QJsonParseError perr{};
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &perr);
    if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
        error = QString::fromUtf8("design.json 解析失败：%1").arg(perr.errorString());
        return false;
    }
    const QJsonObject obj = doc.object();

    out = DuctParams{};
    out.caseId = obj.value(QStringLiteral("case_id")).toString();
    out.shapeFamily = obj.value(QStringLiteral("shape_family")).toString();
    out.paramVersion = obj.value(QStringLiteral("param_version")).toString();

    const QJsonObject units = obj.value(QStringLiteral("units")).toObject();
    if (units.contains(QStringLiteral("length")))
        out.lengthUnit = units.value(QStringLiteral("length")).toString();
    if (units.contains(QStringLiteral("angle")))
        out.angleUnit = units.value(QStringLiteral("angle")).toString();

    const QJsonObject fixed = obj.value(QStringLiteral("fixed")).toObject();
    out.inletArea = fixed.value(QStringLiteral("inlet_area")).toDouble();
    out.outletArea = fixed.value(QStringLiteral("outlet_area")).toDouble();
    out.axialLength = fixed.value(QStringLiteral("axial_length")).toDouble();
    out.inletRibLocked = fixed.value(QStringLiteral("inlet_rib_locked")).toBool(true);
    out.outletRibLocked = fixed.value(QStringLiteral("outlet_rib_locked")).toBool(true);

    const QJsonObject fv = obj.value(QStringLiteral("free_variables")).toObject();
    const QJsonObject inDir = fv.value(QStringLiteral("inlet_dir")).toObject();
    out.inletPitchDeg = inDir.value(QStringLiteral("pitch_deg")).toDouble();
    out.inletExtend = inDir.value(QStringLiteral("extend")).toDouble();
    const QJsonObject outDir = fv.value(QStringLiteral("outlet_dir")).toObject();
    out.outletPitchDeg = outDir.value(QStringLiteral("pitch_deg")).toDouble();
    out.outletExtend = outDir.value(QStringLiteral("extend")).toDouble();

    const QJsonArray ribs = fv.value(QStringLiteral("ribs")).toArray();
    for (const QJsonValue &v : ribs) {
        const QJsonObject ro = v.toObject();
        DuctRib rib;
        rib.id = ro.value(QStringLiteral("id")).toString();
        rib.spinePos = ro.value(QStringLiteral("spine_pos")).toDouble();
        rib.scale = ro.value(QStringLiteral("scale")).toDouble();
        rib.zShift = ro.value(QStringLiteral("z_shift")).toDouble();
        rib.controlPointY = ro.value(QStringLiteral("control_point_y")).toDouble();
        for (const QJsonValue &z : ro.value(QStringLiteral("control_points_z")).toArray())
            rib.controlPointsZ << z.toDouble();
        out.ribs << rib;
    }

    out.catiaReference = obj.value(QStringLiteral("catia_reference")).toString();
    out.loaded = true;
    return true;
}

QJsonObject DuctDesignService::toJson(const DuctParams &params) const
{
    QJsonObject obj;
    obj.insert(QStringLiteral("case_id"), params.caseId);
    obj.insert(QStringLiteral("shape_family"), params.shapeFamily);
    obj.insert(QStringLiteral("param_version"), params.paramVersion);

    QJsonObject units;
    units.insert(QStringLiteral("length"), params.lengthUnit);
    units.insert(QStringLiteral("angle"), params.angleUnit);
    obj.insert(QStringLiteral("units"), units);

    QJsonObject fixed;
    fixed.insert(QStringLiteral("inlet_area"), params.inletArea);
    fixed.insert(QStringLiteral("outlet_area"), params.outletArea);
    fixed.insert(QStringLiteral("axial_length"), params.axialLength);
    fixed.insert(QStringLiteral("inlet_rib_locked"), params.inletRibLocked);
    fixed.insert(QStringLiteral("outlet_rib_locked"), params.outletRibLocked);
    obj.insert(QStringLiteral("fixed"), fixed);

    QJsonObject fv;
    QJsonObject inDir;
    inDir.insert(QStringLiteral("pitch_deg"), params.inletPitchDeg);
    inDir.insert(QStringLiteral("extend"), params.inletExtend);
    fv.insert(QStringLiteral("inlet_dir"), inDir);
    QJsonObject outDir;
    outDir.insert(QStringLiteral("pitch_deg"), params.outletPitchDeg);
    outDir.insert(QStringLiteral("extend"), params.outletExtend);
    fv.insert(QStringLiteral("outlet_dir"), outDir);

    QJsonArray ribs;
    for (const DuctRib &rib : params.ribs) {
        QJsonObject ro;
        ro.insert(QStringLiteral("id"), rib.id);
        ro.insert(QStringLiteral("spine_pos"), rib.spinePos);
        ro.insert(QStringLiteral("scale"), rib.scale);
        ro.insert(QStringLiteral("z_shift"), rib.zShift);
        QJsonArray cz;
        for (double z : rib.controlPointsZ)
            cz.append(z);
        ro.insert(QStringLiteral("control_points_z"), cz);
        ro.insert(QStringLiteral("control_point_y"), rib.controlPointY);
        ribs.append(ro);
    }
    fv.insert(QStringLiteral("ribs"), ribs);
    obj.insert(QStringLiteral("free_variables"), fv);

    obj.insert(QStringLiteral("catia_reference"), params.catiaReference);
    return obj;
}

bool DuctDesignService::exportSnapshot(const DuctParams &params, const QString &path, QString &error) const
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        error = QString::fromUtf8("无法写入参数快照：%1").arg(path);
        return false;
    }
    file.write(QJsonDocument(toJson(params)).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

QStringList DuctDesignService::validate(const DuctParams &params) const
{
    QStringList issues;
    auto checkRange = [&](const QString &name, double v, double lo, double hi, const QString &unit) {
        if (v < lo || v > hi)
            issues << QString::fromUtf8("%1 = %2%3 超出建议范围 [%4, %5]")
                          .arg(name).arg(v).arg(unit).arg(lo).arg(hi);
    };

    if (params.axialLength <= 0.0)
        issues << QString::fromUtf8("轴向长度必须为正。");
    if (params.inletArea <= 0.0)
        issues << QString::fromUtf8("入口面积必须为正。");
    if (params.outletArea <= 0.0)
        issues << QString::fromUtf8("出口面积必须为正。");

    checkRange(QString::fromUtf8("入口俯仰角"), params.inletPitchDeg, -15.0, 15.0, QStringLiteral("°"));
    checkRange(QString::fromUtf8("入口延伸系数"), params.inletExtend, 0.0, 1.0, QString());
    checkRange(QString::fromUtf8("出口俯仰角"), params.outletPitchDeg, -15.0, 15.0, QStringLiteral("°"));
    checkRange(QString::fromUtf8("出口延伸系数"), params.outletExtend, 0.0, 1.0, QString());

    if (params.ribs.size() != 3)
        issues << QString::fromUtf8("中间 rib 数应为 3，当前 %1。").arg(params.ribs.size());

    for (const DuctRib &rib : params.ribs) {
        const QString tag = rib.id.isEmpty() ? QString::fromUtf8("rib") : rib.id;
        checkRange(tag + QString::fromUtf8(" 相对位置"), rib.spinePos, 0.0, 1.0, QString());
        checkRange(tag + QString::fromUtf8(" 缩放"), rib.scale, 0.5, 1.5, QString());
        checkRange(tag + QString::fromUtf8(" 下沉"), rib.zShift, -0.5, 0.2, QStringLiteral("m"));
        checkRange(tag + QString::fromUtf8(" y 控制点"), rib.controlPointY, 0.0, 0.5, QStringLiteral("m"));
        if (rib.controlPointsZ.size() != 5) {
            issues << QString::fromUtf8("%1 的 z 控制点应为 5 个，当前 %2。")
                          .arg(tag).arg(rib.controlPointsZ.size());
            continue;
        }
        const double tol = 1e-3;
        const bool symmetric =
            qAbs(rib.controlPointsZ.at(0) + rib.controlPointsZ.at(4)) < tol &&
            qAbs(rib.controlPointsZ.at(1) + rib.controlPointsZ.at(3)) < tol &&
            qAbs(rib.controlPointsZ.at(2)) < tol;
        if (!symmetric)
            issues << QString::fromUtf8("%1 的 z 控制点未满足 z 轴对称（要求首尾相反、中点为 0）。").arg(tag);
    }
    return issues;
}
