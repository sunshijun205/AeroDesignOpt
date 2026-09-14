#ifndef DUCTDESIGNSERVICE_H
#define DUCTDESIGNSERVICE_H

#include "ductparams.h"

#include <QString>
#include <QStringList>

class QJsonObject;

// 参数化设计服务（M1 输入端）。
// 载入/导出统一设计参数（design.json 同构），并做取值范围与对称性校验。不触碰 UI。
class DuctDesignService
{
public:
    // 从 design.json 载入设计参数。成功返回 true 并填充 out；失败返回 false 并填充 error。
    bool load(const QString &path, DuctParams &out, QString &error) const;

    // 导出参数快照（design.json 同构），供几何/气动/优化共用与回灌数据集。
    bool exportSnapshot(const DuctParams &params, const QString &path, QString &error) const;

    // 校验取值范围与 rib z 对称性，返回问题列表（空=通过）。
    QStringList validate(const DuctParams &params) const;

private:
    QJsonObject toJson(const DuctParams &params) const;
};

#endif
