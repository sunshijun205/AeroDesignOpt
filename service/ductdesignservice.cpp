#include "ductdesignservice.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
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

QStringList DuctDesignService::validate(const DuctParams &params, const ParamConstraints &c) const
{
    QStringList issues;
    auto checkRange = [&](const QString &name, double v, const RangeD &r, const QString &unit) {
        if (v < r.lo || v > r.hi)
            issues << QString::fromUtf8("%1 = %2%3 超出建议范围 [%4, %5]")
                          .arg(name).arg(v).arg(unit).arg(r.lo).arg(r.hi);
    };

    if (params.axialLength <= 0.0)
        issues << QString::fromUtf8("轴向长度必须为正。");
    if (params.inletArea <= 0.0)
        issues << QString::fromUtf8("入口面积必须为正。");
    if (params.outletArea <= 0.0)
        issues << QString::fromUtf8("出口面积必须为正。");

    checkRange(QString::fromUtf8("入口俯仰角"), params.inletPitchDeg, c.inletPitchDeg, QStringLiteral("°"));
    checkRange(QString::fromUtf8("入口延伸系数"), params.inletExtend, c.inletExtend, QString());
    checkRange(QString::fromUtf8("出口俯仰角"), params.outletPitchDeg, c.outletPitchDeg, QStringLiteral("°"));
    checkRange(QString::fromUtf8("出口延伸系数"), params.outletExtend, c.outletExtend, QString());

    if (params.ribs.size() != c.ribCount)
        issues << QString::fromUtf8("中间 rib 数应为 %1，当前 %2。").arg(c.ribCount).arg(params.ribs.size());

    for (const DuctRib &rib : params.ribs) {
        const QString tag = rib.id.isEmpty() ? QString::fromUtf8("rib") : rib.id;
        checkRange(tag + QString::fromUtf8(" 相对位置"), rib.spinePos, c.ribSpinePos, QString());
        checkRange(tag + QString::fromUtf8(" 缩放"), rib.scale, c.ribScale, QString());
        checkRange(tag + QString::fromUtf8(" 下沉"), rib.zShift, c.ribZShift, QStringLiteral("m"));
        checkRange(tag + QString::fromUtf8(" y 控制点"), rib.controlPointY, c.ribCpY, QStringLiteral("m"));
        if (rib.controlPointsZ.size() != c.cpPerRib) {
            issues << QString::fromUtf8("%1 的 z 控制点应为 %2 个，当前 %3。")
                          .arg(tag).arg(c.cpPerRib).arg(rib.controlPointsZ.size());
            continue;
        }
        if (c.enforceZSymmetry && rib.controlPointsZ.size() == 5) {
            const double tol = c.symmetryTol;
            const bool symmetric =
                qAbs(rib.controlPointsZ.at(0) + rib.controlPointsZ.at(4)) < tol &&
                qAbs(rib.controlPointsZ.at(1) + rib.controlPointsZ.at(3)) < tol &&
                qAbs(rib.controlPointsZ.at(2)) < tol;
            if (!symmetric)
                issues << QString::fromUtf8("%1 的 z 控制点未满足 z 轴对称（要求首尾相反、中点为 0）。").arg(tag);
        }
    }
    return issues;
}

ParamConstraints DuctDesignService::defaultConstraints() const
{
    ParamConstraints c;
    c.source = QString::fromUtf8("内置默认值（结构约束取自论文 DuctGen 参数化；数值范围为经验值，客户取值规则待确认）");
    return c;
}

bool DuctDesignService::loadConstraints(const QString &path, ParamConstraints &out, QString &error) const
{
    out = defaultConstraints();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = QString::fromUtf8("无法读取约束配置：%1").arg(path);
        return false;
    }
    const QByteArray raw = file.readAll();
    file.close();

    QJsonParseError perr{};
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &perr);
    if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
        error = QString::fromUtf8("param_ranges.json 解析失败：%1").arg(perr.errorString());
        return false;
    }
    const QJsonObject obj = doc.object();

    const QJsonObject st = obj.value(QStringLiteral("structural")).toObject();
    if (st.contains(QStringLiteral("rib_count")))
        out.ribCount = st.value(QStringLiteral("rib_count")).toInt(out.ribCount);
    if (st.contains(QStringLiteral("control_points_per_rib")))
        out.cpPerRib = st.value(QStringLiteral("control_points_per_rib")).toInt(out.cpPerRib);
    if (st.contains(QStringLiteral("enforce_z_symmetry")))
        out.enforceZSymmetry = st.value(QStringLiteral("enforce_z_symmetry")).toBool(out.enforceZSymmetry);
    if (st.contains(QStringLiteral("symmetry_tolerance")))
        out.symmetryTol = st.value(QStringLiteral("symmetry_tolerance")).toDouble(out.symmetryTol);

    const QJsonObject rg = obj.value(QStringLiteral("ranges")).toObject();
    auto readRange = [&](const QString &key, RangeD &r) {
        if (!rg.contains(key))
            return;
        const QJsonArray a = rg.value(key).toArray();
        if (a.size() >= 2) {
            r.lo = a.at(0).toDouble();
            r.hi = a.at(1).toDouble();
        }
    };
    readRange(QStringLiteral("inlet_pitch_deg"), out.inletPitchDeg);
    readRange(QStringLiteral("inlet_extend"), out.inletExtend);
    readRange(QStringLiteral("outlet_pitch_deg"), out.outletPitchDeg);
    readRange(QStringLiteral("outlet_extend"), out.outletExtend);
    readRange(QStringLiteral("rib_spine_pos"), out.ribSpinePos);
    readRange(QStringLiteral("rib_scale"), out.ribScale);
    readRange(QStringLiteral("rib_z_shift"), out.ribZShift);
    readRange(QStringLiteral("rib_control_point_y"), out.ribCpY);
    return true;
}

ParamConstraints DuctDesignService::resolveConstraints(const QString &designPath) const
{
    if (!designPath.isEmpty()) {
        QDir d(QFileInfo(designPath).absolutePath());
        for (int up = 0; up < 3; ++up) {
            const QString p = d.filePath(QStringLiteral("param_ranges.json"));
            if (QFile::exists(p)) {
                ParamConstraints c;
                QString err;
                if (loadConstraints(p, c, err)) {
                    c.source = QString::fromUtf8("数据集配置文件：%1").arg(QDir::cleanPath(p));
                    return c;
                }
            }
            if (!d.cdUp())
                break;
        }
    }
    return defaultConstraints();
}
