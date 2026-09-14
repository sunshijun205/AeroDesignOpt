#ifndef AIRFOILSTORE_H
#define AIRFOILSTORE_H

#include "airfoiltypes.h"

#include <QString>

// 翼型分析输入的本地持久化（JSON）。
// 路径：%AppData%/AeroDesignOpt/气动设计优化平台/airfoil_analysis.json
class AirfoilStore
{
public:
    AirfoilStore();

    bool save(const AirfoilInput &input, QString *errorMessage = nullptr);
    AirfoilInput load(bool *ok = nullptr);

    QString filePath() const;

private:
    QString m_dir;
};

#endif
