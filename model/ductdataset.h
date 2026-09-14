#ifndef DUCTDATASET_H
#define DUCTDATASET_H

#include <QString>
#include <QStringList>
#include <QVector>

// 单个 CFD 算例的索引记录（来自 cases_index.csv）。
struct DuctCaseRecord {
    QString caseId;
    QString batch;
    QString shapeFamily;
    double mach = 0.0;
    double aoaDeg = 0.0;
    double massFlow = 0.0;
    double sigma = 0.0;
    double omega = 0.0;
    double pi = 0.0;
    double dc60 = 0.0;
    double sc60 = 0.0;
    double visibility = 0.0;
    bool hasField = false;
    QString status;
};

// 数据批次登记（来自 dataset.json 的 batches）。
struct DatasetBatch {
    QString id;
    int cases = 0;
    bool hasField = false;
    QString status;
};

// 单个算例的计算口径元数据（来自 cases/case_<id>/solver_meta.json）。
struct CaseMeta {
    QString caseId;
    QString batch;
    QString solver;
    QString solverVersion;
    QString turbulenceModel;
    QString referenceTotalPressure;  // 参考总压位置
    QString outletAveraging;         // 出口平均方式
    QString outletBc;                // 出口边界类型
    QString convergence;             // 已收敛 / 未收敛 / 未知
    bool metaFound = false;          // solver_meta.json 是否存在且解析成功

    // 代表性设计变量（来自 design.json，用于联合空间覆盖评估）。
    double inletPitchDeg = 0.0;
    double outletPitchDeg = 0.0;
    double rib2ZShift = 0.0;
    bool designFound = false;
};

// 一致性核查的单行结果。
struct ConsistencyRow {
    QString item;         // 核查项
    QString expected;     // 全局约定/期望值
    QString found;        // 数据中出现的取值（含计数）
    bool consistent = true;
    QString conclusion;   // 结论文案
};

// 一次一致性审查报告（对应需求 4.2）。
struct ConsistencyReport {
    int checkedCases = 0;             // 参与核查的算例数
    int missingMeta = 0;             // 缺少 solver_meta.json 的算例数
    QVector<ConsistencyRow> rows;
    QStringList deviations;          // 偏离/告警明细
};

// 单个维度（设计变量或工况变量）的覆盖情况。
struct CoverageDim {
    QString name;
    QString kind;        // 设计 / 工况
    QString unit;
    int distinctCount = 0;
    double minValue = 0.0;
    double maxValue = 0.0;
};

// 单个算例的审定结论（是否可用 + 训练/验证角色）。
struct CaseCuration {
    QString caseId;
    QString batch;
    bool usable = true;
    QString role;    // train / validation / excluded（机器可消费）
    QString reason;  // 排除原因（不可用时）
};

// 审定与训练/验证划分报告（对应需求 3.5 交付数据集 / 4.5 独立验证）。
struct CurationReport {
    int usableCount = 0;
    int trainCount = 0;
    int validationCount = 0;
    int excludedCount = 0;
    QString splitRule;
    QVector<CaseCuration> cases;
};

// 覆盖度报告（对应需求 4.3 联合空间覆盖 / 4.4 重点区域加密）。
struct CoverageReport {
    int cases = 0;
    int geometries = 0;      // 不同外形数
    int conditionPoints = 0; // 不同工况点数
    int jointCovered = 0;    // 已计算的“外形 × 工况”组合数
    int jointTarget = 0;     // 目标组合数 = 外形数 × 工况点数
    double jointRatio = 0.0; // 覆盖率
    QVector<CoverageDim> dims;
    QStringList suggestions; // 加密建议（数据驱动）
    QStringList notes;       // 覆盖说明/告警
};

// 一次导入得到的数据集摘要。
struct DatasetSummary {
    QString name;
    QString createdDate;
    int paramDimension = 0;
    int totalCases = 0;
    int fieldCases = 0;
    int conditionPoints = 0;
    QStringList shapeFamilies;
    QVector<DatasetBatch> batches;
    QVector<DuctCaseRecord> records;
    QStringList warnings;
    QString rootDir;

    // 全局计算口径约定（来自 dataset.json 的 global_conventions）。
    QString refTotalPressureConvention;
    QString outletAveragingConvention;

    // 一致性审查（读取各算例 solver_meta.json 后计算）。
    QVector<CaseMeta> caseMetas;
    ConsistencyReport consistency;

    // 覆盖度评估（读取 records + 各算例 design.json 后计算）。
    CoverageReport coverage;

    // 审定与训练/验证划分（导出交付物的依据）。
    CurationReport curation;
};

#endif
