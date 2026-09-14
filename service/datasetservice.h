#ifndef DATASETSERVICE_H
#define DATASETSERVICE_H

#include "ductdataset.h"

#include <QString>

class QJsonObject;

// 数据集导入服务（M7）。
// 读取数据集根目录下的 dataset.json + cases_index.csv，
// 建立算例/参数/工况/结果对应关系并做基本一致性检查。不触碰 UI。
class DatasetService
{
public:
    // 成功返回 true 并填充 out；失败返回 false 并填充 error。禁止 silent failure。
    bool load(const QString &rootDir, DatasetSummary &out, QString &error) const;

    // 导出交付物：dataset_review.json（一致性/覆盖度报告）+ training_manifest.json
    //（审定后的可用算例 + 训练/验证标记，供代理训练模块直接消费）。
    // 成功返回 true 并把写出的文件路径填入 written；失败返回 false 并填充 error。
    bool exportReports(const DatasetSummary &summary, const QString &outDir,
                       QStringList &written, QString &error) const;

private:
    bool parseManifest(const QString &path, DatasetSummary &out, QString &error) const;
    bool parseIndex(const QString &path, DatasetSummary &out, QString &error) const;
    void loadCaseMetas(const QString &rootDir, DatasetSummary &out) const;
    bool parseSolverMeta(const QString &path, CaseMeta &meta) const;
    bool parseDesign(const QString &path, CaseMeta &meta) const;
    void buildConsistency(DatasetSummary &out) const;
    void buildCoverage(DatasetSummary &out) const;
    void buildCuration(DatasetSummary &out) const;
    bool writeJson(const QString &path, const QJsonObject &obj, QString &error) const;
};

#endif
