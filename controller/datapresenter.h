#ifndef DATAPRESENTER_H
#define DATAPRESENTER_H

#include <QObject>
#include <QString>

class DataPage;
class DatasetService;

// 数据管理页协调层（Presenter）。
// 唯一负责在 View 意图与 DatasetService 之间协调，View 不直接调用 Service。
class DataPresenter : public QObject
{
    Q_OBJECT
public:
    DataPresenter(DataPage *view, DatasetService *service, QObject *parent = nullptr);

    // interactive=true：失败弹错误框；false：静默（用于启动自动加载）。
    void importFrom(const QString &rootDir, bool interactive);

public slots:
    void onImportRequested(const QString &rootDir);
    void onExportRequested(const QString &outDir);

private:
    DataPage *m_view = nullptr;
    DatasetService *m_service = nullptr;
};

#endif
