#ifndef ANALYSISPRESENTER_H
#define ANALYSISPRESENTER_H

#include "airfoiltypes.h"

#include <QObject>

class AnalysisPage;
class AeroAnalysisService;
class AirfoilStore;

// 编排气动分析用例：连接 View 信号 → Service 计算 / Store 落盘 → 刷新 View。
// View 不直接调用 Service / Store。
class AnalysisPresenter : public QObject
{
    Q_OBJECT
public:
    AnalysisPresenter(AnalysisPage *view,
                      AeroAnalysisService *service,
                      AirfoilStore *store,
                      QObject *parent = nullptr);

    // 构造完成后调用：载入上次输入并回填 View。
    void initialize();

private slots:
    void onRunRequested(const AirfoilInput &input);
    void onSaveRequested();

private:
    AnalysisPage *m_view = nullptr;
    AeroAnalysisService *m_service = nullptr;
    AirfoilStore *m_store = nullptr;
};

#endif
