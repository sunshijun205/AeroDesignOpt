#ifndef DATAPAGE_H
#define DATAPAGE_H

#include "ductdataset.h"

#include <QWidget>
#include <memory>

class QStackedWidget;
class QVBoxLayout;
class QLabel;
class QLineEdit;
class DatasetService;
class DataPresenter;

// 数据管理页（View）。
// 对应模块 M7 数据集管理：数据源导入、一致性审查、联合空间覆盖度。
// “数据源”子页已接入真实导入数据流（View → Presenter → Service → model）；
// 一致性审查/覆盖度子页暂为静态示意。
class DataPage : public QWidget
{
    Q_OBJECT
public:
    explicit DataPage(QWidget *parent = nullptr);
    ~DataPage() override;

    // 顶栏“导入数据”按钮转发：弹目录选择框，选定后发意图信号。
    void requestImport();
    // 导出报告与训练清单：弹目录选择框，选定后发意图信号。
    void requestExport();
    // 供 Presenter 读取当前已导入的数据集摘要。
    const DatasetSummary &currentSummary() const { return m_summary; }

signals:
    void importRequested(const QString &rootDir);
    void exportRequested(const QString &outDir);

public slots:
    void setSummary(const DatasetSummary &summary);
    void showError(const QString &message);
    void setPlaceholder(const QString &message);
    void showExportResult(const QStringList &files);

private:
    QWidget *buildSourcePage();
    QWidget *buildConsistencyPage();
    QWidget *buildCoveragePage();
    void tryAutoLoad();

    QStackedWidget *m_inner = nullptr;
    QVBoxLayout *m_kpiHost = nullptr;
    QVBoxLayout *m_tableHost = nullptr;
    QLabel *m_sourceStatus = nullptr;
    QLineEdit *m_rootDir = nullptr;
    QVBoxLayout *m_consistencyHost = nullptr;
    QLabel *m_consistencyStatus = nullptr;
    QVBoxLayout *m_coverageKpiHost = nullptr;
    QVBoxLayout *m_coverageDimHost = nullptr;
    QLabel *m_coverageStatus = nullptr;

    std::unique_ptr<DatasetService> m_service;
    DataPresenter *m_presenter = nullptr;
    DatasetSummary m_summary;
    bool m_hasData = false;
};

#endif
