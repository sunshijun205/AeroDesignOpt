#ifndef DUCTDESIGNPRESENTER_H
#define DUCTDESIGNPRESENTER_H

#include <QObject>
#include <QString>

class DuctDesignPage;
class DuctDesignService;

// 参数化设计页协调层（Presenter）。
// 在 View 意图与 DuctDesignService 之间协调，View 不直接调用 Service。
class DuctDesignPresenter : public QObject
{
    Q_OBJECT
public:
    DuctDesignPresenter(DuctDesignPage *view, DuctDesignService *service, QObject *parent = nullptr);

    // interactive=true：失败弹错误；false：静默（启动自动载入）。
    void loadFrom(const QString &path, bool interactive);

public slots:
    void onLoadRequested(const QString &path);
    void onValidateRequested();
    void onExportRequested(const QString &path);

private:
    DuctDesignPage *m_view = nullptr;
    DuctDesignService *m_service = nullptr;
};

#endif
