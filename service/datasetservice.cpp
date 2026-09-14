#include "datasetservice.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QSet>
#include <QTextStream>

bool DatasetService::load(const QString &rootDir, DatasetSummary &out, QString &error) const
{
    out = DatasetSummary{};
    out.rootDir = rootDir;

    QDir root(rootDir);
    if (!root.exists()) {
        error = QString::fromUtf8("目录不存在：%1").arg(rootDir);
        return false;
    }

    const QString manifestPath = root.filePath(QStringLiteral("dataset.json"));
    if (!QFile::exists(manifestPath)) {
        error = QString::fromUtf8("未找到数据集清单 dataset.json：%1").arg(manifestPath);
        return false;
    }
    if (!parseManifest(manifestPath, out, error))
        return false;

    const QString indexPath = root.filePath(QStringLiteral("cases_index.csv"));
    if (!QFile::exists(indexPath)) {
        out.warnings << QString::fromUtf8("未找到算例索引 cases_index.csv，仅载入清单信息。");
    } else if (!parseIndex(indexPath, out, error)) {
        return false;
    }

    // 计算汇总量。
    out.totalCases = out.records.size();
    out.fieldCases = 0;
    QSet<QString> families;
    QSet<QString> conditions;
    for (const DuctCaseRecord &r : out.records) {
        if (r.hasField)
            ++out.fieldCases;
        if (!r.shapeFamily.isEmpty())
            families.insert(r.shapeFamily);
        conditions.insert(QString::number(r.mach, 'f', 3) + QLatin1Char('|')
                          + QString::number(r.aoaDeg, 'f', 2) + QLatin1Char('|')
                          + QString::number(r.massFlow, 'f', 2));
    }
    out.conditionPoints = conditions.size();
    if (out.shapeFamilies.isEmpty())
        out.shapeFamilies = QStringList(families.values());

    // 一致性/状态告警。
    for (const DatasetBatch &b : out.batches) {
        if (b.status != QLatin1String("verified") && b.status != QLatin1String("ok"))
            out.warnings << QString::fromUtf8("批次 %1 状态为“%2”，需在一致性审查中确认后再用于训练。")
                                .arg(b.id, b.status);
    }
    int violated = 0;
    for (const DuctCaseRecord &r : out.records)
        if (r.status != QLatin1String("ok"))
            ++violated;
    if (violated > 0)
        out.warnings << QString::fromUtf8("有 %1 个算例状态非 ok（如约束越界），已标注供筛选。").arg(violated);

    // 读取各算例计算口径元数据与关键设计变量。
    loadCaseMetas(rootDir, out);
    // 一致性审查（需求 4.2）。
    buildConsistency(out);
    // 覆盖度评估（需求 4.3 / 4.4）。
    buildCoverage(out);
    // 审定与训练/验证划分（需求 3.5 / 4.5）。
    buildCuration(out);

    return true;
}

bool DatasetService::parseManifest(const QString &path, DatasetSummary &out, QString &error) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = QString::fromUtf8("无法读取 dataset.json：%1").arg(path);
        return false;
    }
    const QByteArray raw = file.readAll();
    file.close();

    QJsonParseError perr{};
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &perr);
    if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
        error = QString::fromUtf8("dataset.json 解析失败：%1").arg(perr.errorString());
        return false;
    }
    const QJsonObject obj = doc.object();
    out.name = obj.value(QStringLiteral("dataset_name")).toString();
    out.createdDate = obj.value(QStringLiteral("created")).toString();
    out.paramDimension = obj.value(QStringLiteral("param_dimension")).toInt();

    const QJsonObject conv = obj.value(QStringLiteral("global_conventions")).toObject();
    out.refTotalPressureConvention = conv.value(QStringLiteral("reference_total_pressure_location")).toString();
    out.outletAveragingConvention = conv.value(QStringLiteral("outlet_averaging_method")).toString();

    const QJsonArray families = obj.value(QStringLiteral("shape_families")).toArray();
    for (const QJsonValue &v : families)
        out.shapeFamilies << v.toString();

    const QJsonArray batches = obj.value(QStringLiteral("batches")).toArray();
    for (const QJsonValue &v : batches) {
        const QJsonObject bo = v.toObject();
        DatasetBatch b;
        b.id = bo.value(QStringLiteral("id")).toString();
        b.cases = bo.value(QStringLiteral("cases")).toInt();
        b.hasField = bo.value(QStringLiteral("has_field")).toBool();
        b.status = bo.value(QStringLiteral("status")).toString();
        out.batches << b;
    }
    return true;
}

bool DatasetService::parseIndex(const QString &path, DatasetSummary &out, QString &error) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = QString::fromUtf8("无法读取 cases_index.csv：%1").arg(path);
        return false;
    }
    QTextStream in(&file);
    QStringList headers;
    bool firstLine = true;
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;
        const QStringList cols = line.split(QLatin1Char(','));
        if (firstLine) {
            for (const QString &h : cols)
                headers << h.trimmed();
            firstLine = false;
            continue;
        }
        auto valueOf = [&](const QString &key) -> QString {
            const int idx = headers.indexOf(key);
            if (idx < 0 || idx >= cols.size())
                return QString();
            return cols.at(idx).trimmed();
        };
        DuctCaseRecord r;
        r.caseId = valueOf(QStringLiteral("case_id"));
        r.batch = valueOf(QStringLiteral("batch"));
        r.shapeFamily = valueOf(QStringLiteral("shape_family"));
        r.mach = valueOf(QStringLiteral("mach")).toDouble();
        r.aoaDeg = valueOf(QStringLiteral("aoa_deg")).toDouble();
        r.massFlow = valueOf(QStringLiteral("mass_flow_kgs")).toDouble();
        r.sigma = valueOf(QStringLiteral("sigma")).toDouble();
        r.omega = valueOf(QStringLiteral("omega")).toDouble();
        r.pi = valueOf(QStringLiteral("pi")).toDouble();
        r.dc60 = valueOf(QStringLiteral("DC60")).toDouble();
        r.sc60 = valueOf(QStringLiteral("SC60")).toDouble();
        r.visibility = valueOf(QStringLiteral("visibility")).toDouble();
        r.hasField = valueOf(QStringLiteral("has_field")).compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;
        r.status = valueOf(QStringLiteral("status"));
        if (!r.caseId.isEmpty())
            out.records << r;
    }
    file.close();

    if (out.records.isEmpty())
        out.warnings << QString::fromUtf8("cases_index.csv 未解析到任何算例记录。");
    return true;
}

void DatasetService::loadCaseMetas(const QString &rootDir, DatasetSummary &out) const
{
    QDir root(rootDir);
    for (const DuctCaseRecord &r : out.records) {
        CaseMeta meta;
        meta.caseId = r.caseId;
        meta.batch = r.batch;
        const QString metaPath = root.filePath(QStringLiteral("cases/case_%1/solver_meta.json").arg(r.caseId));
        if (QFile::exists(metaPath))
            parseSolverMeta(metaPath, meta);
        const QString designPath = root.filePath(QStringLiteral("cases/case_%1/design.json").arg(r.caseId));
        if (QFile::exists(designPath))
            parseDesign(designPath, meta);
        out.caseMetas << meta;
    }
}

bool DatasetService::parseSolverMeta(const QString &path, CaseMeta &meta) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    const QByteArray raw = file.readAll();
    file.close();

    QJsonParseError perr{};
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &perr);
    if (perr.error != QJsonParseError::NoError || !doc.isObject())
        return false;

    const QJsonObject obj = doc.object();
    meta.solver = obj.value(QStringLiteral("solver")).toString();
    meta.solverVersion = obj.value(QStringLiteral("solver_version")).toString();
    meta.turbulenceModel = obj.value(QStringLiteral("turbulence_model")).toString();
    meta.referenceTotalPressure = obj.value(QStringLiteral("reference_total_pressure_location")).toString();
    meta.outletAveraging = obj.value(QStringLiteral("outlet_averaging_method")).toString();
    meta.outletBc = obj.value(QStringLiteral("outlet_bc_type")).toString();

    if (obj.contains(QStringLiteral("convergence"))) {
        const QJsonObject cvg = obj.value(QStringLiteral("convergence")).toObject();
        meta.convergence = cvg.value(QStringLiteral("converged")).toBool()
                               ? QString::fromUtf8("已收敛")
                               : QString::fromUtf8("未收敛");
    } else {
        meta.convergence = QString::fromUtf8("未知");
    }
    meta.metaFound = true;
    return true;
}

void DatasetService::buildConsistency(DatasetSummary &out) const
{
    ConsistencyReport rep;

    QStringList caseIds;
    QStringList vRef, vAvg, vTurb, vBc, vSolver, vConv;
    for (const CaseMeta &m : out.caseMetas) {
        if (!m.metaFound) {
            ++rep.missingMeta;
            rep.deviations << QString::fromUtf8("算例 %1 缺少 solver_meta.json，无法核查计算口径。").arg(m.caseId);
            continue;
        }
        caseIds << m.caseId;
        vRef << m.referenceTotalPressure;
        vAvg << m.outletAveraging;
        vTurb << m.turbulenceModel;
        vBc << m.outletBc;
        vSolver << QString(m.solver + QLatin1Char(' ') + m.solverVersion).trimmed();
        vConv << m.convergence;
    }
    rep.checkedCases = caseIds.size();

    auto makeRow = [](const QString &item, const QString &expected,
                      const QStringList &caseIds, const QStringList &values) -> ConsistencyRow {
        ConsistencyRow row;
        row.item = item;
        QMap<QString, int> counts;
        for (const QString &v : values)
            counts[v.isEmpty() ? QString::fromUtf8("(空)") : v] += 1;
        QStringList parts;
        for (auto it = counts.constBegin(); it != counts.constEnd(); ++it)
            parts << QString::fromUtf8("%1×%2").arg(it.key()).arg(it.value());
        row.found = parts.join(QString::fromUtf8("、"));

        QStringList deviate;
        if (!expected.isEmpty()) {
            row.expected = expected;
            for (int i = 0; i < values.size(); ++i)
                if (values.at(i) != expected)
                    deviate << caseIds.at(i);
            row.consistent = deviate.isEmpty();
        } else {
            row.expected = counts.size() == 1 ? counts.firstKey() : QString::fromUtf8("（无全局约定）");
            row.consistent = counts.size() <= 1;
        }
        if (row.consistent)
            row.conclusion = QString::fromUtf8("一致");
        else if (!deviate.isEmpty())
            row.conclusion = QString::fromUtf8("不一致（算例 %1）").arg(deviate.join(QString::fromUtf8("、")));
        else
            row.conclusion = QString::fromUtf8("不一致（多值）");
        return row;
    };

    if (rep.checkedCases > 0) {
        rep.rows << makeRow(QString::fromUtf8("参考总压位置"), out.refTotalPressureConvention, caseIds, vRef);
        rep.rows << makeRow(QString::fromUtf8("出口平均方式"), out.outletAveragingConvention, caseIds, vAvg);
        rep.rows << makeRow(QString::fromUtf8("湍流模型"), QString(), caseIds, vTurb);
        rep.rows << makeRow(QString::fromUtf8("出口边界"), QString(), caseIds, vBc);
        rep.rows << makeRow(QString::fromUtf8("求解器/版本"), QString(), caseIds, vSolver);
        rep.rows << makeRow(QString::fromUtf8("收敛状态"), QString::fromUtf8("已收敛"), caseIds, vConv);

        for (const ConsistencyRow &r : rep.rows)
            if (!r.consistent)
                rep.deviations << QString::fromUtf8("%1：%2").arg(r.item, r.conclusion);
    }

    out.consistency = rep;
}

bool DatasetService::parseDesign(const QString &path, CaseMeta &meta) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    const QByteArray raw = file.readAll();
    file.close();

    QJsonParseError perr{};
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &perr);
    if (perr.error != QJsonParseError::NoError || !doc.isObject())
        return false;

    const QJsonObject fv = doc.object().value(QStringLiteral("free_variables")).toObject();
    meta.inletPitchDeg = fv.value(QStringLiteral("inlet_dir")).toObject()
                             .value(QStringLiteral("pitch_deg")).toDouble();
    meta.outletPitchDeg = fv.value(QStringLiteral("outlet_dir")).toObject()
                              .value(QStringLiteral("pitch_deg")).toDouble();
    const QJsonArray ribs = fv.value(QStringLiteral("ribs")).toArray();
    for (const QJsonValue &v : ribs) {
        const QJsonObject rib = v.toObject();
        if (rib.value(QStringLiteral("id")).toString() == QLatin1String("rib2")) {
            meta.rib2ZShift = rib.value(QStringLiteral("z_shift")).toDouble();
            break;
        }
    }
    meta.designFound = true;
    return true;
}

void DatasetService::buildCoverage(DatasetSummary &out) const
{
    CoverageReport cov;
    cov.cases = out.records.size();
    cov.conditionPoints = out.conditionPoints;

    // 不同外形数（按算例几何计，样例中每个算例为一套外形）。
    QSet<QString> geoms;
    for (const DuctCaseRecord &r : out.records)
        geoms.insert(r.caseId);
    cov.geometries = geoms.size();

    // 联合“外形 × 工况”覆盖。
    cov.jointCovered = out.records.size();
    cov.jointTarget = cov.geometries * cov.conditionPoints;
    cov.jointRatio = cov.jointTarget > 0
                         ? static_cast<double>(cov.jointCovered) / cov.jointTarget
                         : 0.0;

    // 维度覆盖：工况变量（来自 records）+ 设计变量（来自 design.json）。
    auto dimFromValues = [](const QString &name, const QString &kind, const QString &unit,
                            const QVector<double> &vals) -> CoverageDim {
        CoverageDim d;
        d.name = name;
        d.kind = kind;
        d.unit = unit;
        QSet<QString> distinct;
        double mn = 0.0, mx = 0.0;
        for (int i = 0; i < vals.size(); ++i) {
            distinct.insert(QString::number(vals.at(i), 'f', 4));
            if (i == 0 || vals.at(i) < mn) mn = vals.at(i);
            if (i == 0 || vals.at(i) > mx) mx = vals.at(i);
        }
        d.distinctCount = distinct.size();
        d.minValue = mn;
        d.maxValue = mx;
        return d;
    };

    QVector<double> mach, aoa, mdot;
    for (const DuctCaseRecord &r : out.records) {
        mach << r.mach;
        aoa << r.aoaDeg;
        mdot << r.massFlow;
    }
    if (!out.records.isEmpty()) {
        cov.dims << dimFromValues(QString::fromUtf8("来流马赫数"), QString::fromUtf8("工况"), QString(), mach);
        cov.dims << dimFromValues(QString::fromUtf8("攻角"), QString::fromUtf8("工况"), QStringLiteral("deg"), aoa);
        cov.dims << dimFromValues(QString::fromUtf8("质量流量"), QString::fromUtf8("工况"), QStringLiteral("kg/s"), mdot);
    }

    QVector<double> inPitch, outPitch, rib2z;
    int designCases = 0;
    for (const CaseMeta &m : out.caseMetas) {
        if (!m.designFound)
            continue;
        ++designCases;
        inPitch << m.inletPitchDeg;
        outPitch << m.outletPitchDeg;
        rib2z << m.rib2ZShift;
    }
    if (designCases > 0) {
        cov.dims << dimFromValues(QString::fromUtf8("入口俯仰角"), QString::fromUtf8("设计"), QStringLiteral("deg"), inPitch);
        cov.dims << dimFromValues(QString::fromUtf8("出口俯仰角"), QString::fromUtf8("设计"), QStringLiteral("deg"), outPitch);
        cov.dims << dimFromValues(QString::fromUtf8("Rib-2 下沉"), QString::fromUtf8("设计"), QStringLiteral("m"), rib2z);
    } else {
        cov.notes << QString::fromUtf8("未找到 design.json，设计变量维度未纳入覆盖评估。");
    }

    // 联合空间说明与加密建议（数据驱动，需求 4.3 / 4.4）。
    if (cov.conditionPoints <= 1)
        cov.notes << QString::fromUtf8("所有算例集中在单一工况点，无法评估跨工况泛化，建议在其他工况补算。");

    // DC60 跨度（非线性信号）。
    if (!out.records.isEmpty()) {
        double dcMin = out.records.first().dc60;
        double dcMax = dcMin;
        for (const DuctCaseRecord &r : out.records) {
            dcMin = qMin(dcMin, r.dc60);
            dcMax = qMax(dcMax, r.dc60);
        }
        if (dcMax - dcMin > 0.20)
            cov.suggestions << QString::fromUtf8("DC60 跨度大（%1%%~%2%%），响应可能强非线性，建议在高畸变区加密。")
                                   .arg(dcMin * 100.0, 0, 'f', 1)
                                   .arg(dcMax * 100.0, 0, 'f', 1);
    }

    // 约束边界样本。
    for (const DuctCaseRecord &r : out.records) {
        if (r.status != QLatin1String("ok"))
            cov.suggestions << QString::fromUtf8("算例 %1 处于约束边界（DC60=%2%%、SC60=%3%%），该区域样本不足，建议就近加密。")
                                   .arg(r.caseId)
                                   .arg(r.dc60 * 100.0, 0, 'f', 1)
                                   .arg(r.sc60 * 100.0, 0, 'f', 1);
    }

    if (cov.jointTarget > 0 && cov.jointCovered < cov.jointTarget)
        cov.suggestions << QString::fromUtf8("联合空间存在空缺：目标组合 %1，已算 %2，建议补齐缺失的外形×工况组合。")
                               .arg(cov.jointTarget)
                               .arg(cov.jointCovered);

    out.coverage = cov;
}

void DatasetService::buildCuration(DatasetSummary &out) const
{
    CurationReport cur;
    cur.splitRule = QString::fromUtf8(
        "可用性：solver_meta 存在、已收敛、参考总压位置与出口平均方式符合全局约定；"
        "划分：可用算例按导入顺序每 5 个取 1 个为验证集（单个可用算例则全部作训练集）。");

    const int n = out.records.size();
    QVector<CaseCuration> items;
    items.reserve(n);
    for (int i = 0; i < n; ++i) {
        const DuctCaseRecord &r = out.records.at(i);
        const CaseMeta meta = (i < out.caseMetas.size()) ? out.caseMetas.at(i) : CaseMeta{};
        CaseCuration c;
        c.caseId = r.caseId;
        c.batch = r.batch;
        c.usable = true;
        if (!meta.metaFound) {
            c.usable = false;
            c.reason = QString::fromUtf8("缺少 solver_meta.json");
        } else if (meta.convergence != QString::fromUtf8("已收敛")) {
            c.usable = false;
            c.reason = QString::fromUtf8("未收敛");
        } else if (!out.refTotalPressureConvention.isEmpty()
                   && meta.referenceTotalPressure != out.refTotalPressureConvention) {
            c.usable = false;
            c.reason = QString::fromUtf8("参考总压位置与全局约定不一致");
        } else if (!out.outletAveragingConvention.isEmpty()
                   && meta.outletAveraging != out.outletAveragingConvention) {
            c.usable = false;
            c.reason = QString::fromUtf8("出口平均方式与全局约定不一致");
        }
        items << c;
    }

    int usableTotal = 0;
    for (const CaseCuration &c : items)
        if (c.usable)
            ++usableTotal;

    const int stride = 5;
    int usableIdx = 0;
    for (int i = 0; i < items.size(); ++i) {
        CaseCuration &c = items[i];
        if (!c.usable) {
            c.role = QStringLiteral("excluded");
            continue;
        }
        if (usableTotal <= 1)
            c.role = QStringLiteral("train");
        else
            c.role = (usableIdx % stride == 0) ? QStringLiteral("validation") : QStringLiteral("train");
        ++usableIdx;
    }

    for (const CaseCuration &c : items) {
        if (!c.usable) {
            ++cur.excludedCount;
        } else {
            ++cur.usableCount;
            if (c.role == QLatin1String("validation"))
                ++cur.validationCount;
            else
                ++cur.trainCount;
        }
    }
    cur.cases = items;
    out.curation = cur;
}

bool DatasetService::writeJson(const QString &path, const QJsonObject &obj, QString &error) const
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        error = QString::fromUtf8("无法写入文件：%1").arg(path);
        return false;
    }
    file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool DatasetService::exportReports(const DatasetSummary &summary, const QString &outDir,
                                   QStringList &written, QString &error) const
{
    QDir dir(outDir);
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        error = QString::fromUtf8("无法创建导出目录：%1").arg(outDir);
        return false;
    }
    const QString ts = QDateTime::currentDateTime().toString(Qt::ISODate);

    // ---------- dataset_review.json（一致性 + 覆盖度报告）----------
    QJsonObject review;
    review.insert(QStringLiteral("dataset_name"), summary.name);
    review.insert(QStringLiteral("generated"), ts);
    review.insert(QStringLiteral("root_dir"), summary.rootDir);

    QJsonObject cons;
    cons.insert(QStringLiteral("checked_cases"), summary.consistency.checkedCases);
    cons.insert(QStringLiteral("missing_meta"), summary.consistency.missingMeta);
    QJsonArray consRows;
    for (const ConsistencyRow &r : summary.consistency.rows) {
        QJsonObject ro;
        ro.insert(QStringLiteral("item"), r.item);
        ro.insert(QStringLiteral("expected"), r.expected);
        ro.insert(QStringLiteral("found"), r.found);
        ro.insert(QStringLiteral("consistent"), r.consistent);
        ro.insert(QStringLiteral("conclusion"), r.conclusion);
        consRows.append(ro);
    }
    cons.insert(QStringLiteral("rows"), consRows);
    QJsonArray consDev;
    for (const QString &d : summary.consistency.deviations)
        consDev.append(d);
    cons.insert(QStringLiteral("deviations"), consDev);
    review.insert(QStringLiteral("consistency"), cons);

    QJsonObject cov;
    cov.insert(QStringLiteral("cases"), summary.coverage.cases);
    cov.insert(QStringLiteral("geometries"), summary.coverage.geometries);
    cov.insert(QStringLiteral("condition_points"), summary.coverage.conditionPoints);
    cov.insert(QStringLiteral("joint_covered"), summary.coverage.jointCovered);
    cov.insert(QStringLiteral("joint_target"), summary.coverage.jointTarget);
    cov.insert(QStringLiteral("joint_ratio"), summary.coverage.jointRatio);
    QJsonArray dims;
    for (const CoverageDim &d : summary.coverage.dims) {
        QJsonObject dobj;
        dobj.insert(QStringLiteral("name"), d.name);
        dobj.insert(QStringLiteral("kind"), d.kind);
        dobj.insert(QStringLiteral("unit"), d.unit);
        dobj.insert(QStringLiteral("distinct"), d.distinctCount);
        dobj.insert(QStringLiteral("min"), d.minValue);
        dobj.insert(QStringLiteral("max"), d.maxValue);
        dims.append(dobj);
    }
    cov.insert(QStringLiteral("dims"), dims);
    QJsonArray sug;
    for (const QString &x : summary.coverage.suggestions)
        sug.append(x);
    cov.insert(QStringLiteral("suggestions"), sug);
    QJsonArray notes;
    for (const QString &x : summary.coverage.notes)
        notes.append(x);
    cov.insert(QStringLiteral("notes"), notes);
    review.insert(QStringLiteral("coverage"), cov);

    const QString reviewPath = dir.filePath(QStringLiteral("dataset_review.json"));
    if (!writeJson(reviewPath, review, error))
        return false;
    written << reviewPath;

    // ---------- training_manifest.json（审定清单 + 训练/验证标记）----------
    QJsonObject man;
    man.insert(QStringLiteral("dataset_name"), summary.name);
    man.insert(QStringLiteral("generated"), ts);
    man.insert(QStringLiteral("root_dir"), summary.rootDir);
    man.insert(QStringLiteral("split_rule"), summary.curation.splitRule);

    QJsonObject counts;
    counts.insert(QStringLiteral("total"), summary.records.size());
    counts.insert(QStringLiteral("usable"), summary.curation.usableCount);
    counts.insert(QStringLiteral("train"), summary.curation.trainCount);
    counts.insert(QStringLiteral("validation"), summary.curation.validationCount);
    counts.insert(QStringLiteral("excluded"), summary.curation.excludedCount);
    man.insert(QStringLiteral("counts"), counts);

    QJsonObject conv;
    conv.insert(QStringLiteral("reference_total_pressure_location"), summary.refTotalPressureConvention);
    conv.insert(QStringLiteral("outlet_averaging_method"), summary.outletAveragingConvention);
    man.insert(QStringLiteral("global_conventions"), conv);

    QJsonArray cases;
    for (int i = 0; i < summary.records.size(); ++i) {
        const DuctCaseRecord &r = summary.records.at(i);
        const CaseMeta meta = (i < summary.caseMetas.size()) ? summary.caseMetas.at(i) : CaseMeta{};
        const CaseCuration c = (i < summary.curation.cases.size()) ? summary.curation.cases.at(i) : CaseCuration{};
        QJsonObject co;
        co.insert(QStringLiteral("case_id"), r.caseId);
        co.insert(QStringLiteral("batch"), r.batch);
        co.insert(QStringLiteral("role"), c.role);
        co.insert(QStringLiteral("usable"), c.usable);
        co.insert(QStringLiteral("reason"), c.reason);

        QJsonObject in;
        in.insert(QStringLiteral("mach"), r.mach);
        in.insert(QStringLiteral("aoa_deg"), r.aoaDeg);
        in.insert(QStringLiteral("mass_flow_kgs"), r.massFlow);
        if (meta.designFound) {
            in.insert(QStringLiteral("inlet_pitch_deg"), meta.inletPitchDeg);
            in.insert(QStringLiteral("outlet_pitch_deg"), meta.outletPitchDeg);
            in.insert(QStringLiteral("rib2_z_shift"), meta.rib2ZShift);
        }
        co.insert(QStringLiteral("inputs"), in);

        QJsonObject tg;
        tg.insert(QStringLiteral("sigma"), r.sigma);
        tg.insert(QStringLiteral("omega"), r.omega);
        tg.insert(QStringLiteral("pi"), r.pi);
        tg.insert(QStringLiteral("DC60"), r.dc60);
        tg.insert(QStringLiteral("SC60"), r.sc60);
        tg.insert(QStringLiteral("visibility"), r.visibility);
        co.insert(QStringLiteral("targets"), tg);

        co.insert(QStringLiteral("has_field"), r.hasField);

        QJsonObject files;
        files.insert(QStringLiteral("design"), QStringLiteral("cases/case_%1/design.json").arg(r.caseId));
        files.insert(QStringLiteral("condition"), QStringLiteral("cases/case_%1/condition.json").arg(r.caseId));
        files.insert(QStringLiteral("result"), QStringLiteral("cases/case_%1/result.json").arg(r.caseId));
        files.insert(QStringLiteral("solver_meta"), QStringLiteral("cases/case_%1/solver_meta.json").arg(r.caseId));
        co.insert(QStringLiteral("files"), files);

        cases.append(co);
    }
    man.insert(QStringLiteral("cases"), cases);

    const QString manPath = dir.filePath(QStringLiteral("training_manifest.json"));
    if (!writeJson(manPath, man, error))
        return false;
    written << manPath;

    return true;
}
