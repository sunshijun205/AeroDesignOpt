#ifndef DUCTDESIGNPAGE_H
#define DUCTDESIGNPAGE_H

#include "ductparams.h"
#include "paramconstraints.h"

#include <QWidget>
#include <QVector>
#include <memory>

class QStackedWidget;
class QLineEdit;
class QLabel;
class QVBoxLayout;
class DuctDesignService;
class DuctDesignPresenter;

// 参数化设计页（View）。对应模块 M1 几何生成的输入端。
// “设计参数”子页已接入真实数据流：载入 design.json → 编辑 → 校验 → 导出参数快照；
// 三维外形生成（PicoGK）暂未接入，流道走向/截面/预览子页仍为静态示意。
class DuctDesignPage : public QWidget
{
    Q_OBJECT
public:
    explicit DuctDesignPage(QWidget *parent = nullptr);
    ~DuctDesignPage() override;

    void requestLoad();
    void requestExport();

    // 从当前表单读出设计参数（保留载入时的 caseId/单位/锁定项/CATIA 引用）。
    DuctParams snapshotParams() const;

signals:
    void loadRequested(const QString &path);
    void validateRequested();
    void exportRequested(const QString &path);

public slots:
    void setParams(const DuctParams &params);
    void setValidation(const QStringList &issues);
    void setConstraints(const ParamConstraints &constraints);
    void showError(const QString &message);
    void setParamStatus(const QString &message, bool warn);
    void showExportOk(const QString &path);

private:
    QWidget *buildParamsPage();
    QWidget *buildSpinePage();
    QWidget *buildSectionPage();
    QWidget *buildPreviewPage();
    void tryAutoLoad();

    struct RibEdits {
        QString id;
        QLineEdit *spinePos = nullptr;
        QLineEdit *scale = nullptr;
        QLineEdit *zShift = nullptr;
        QLineEdit *cpY = nullptr;
        QLineEdit *cpZ = nullptr; // 逗号分隔的 5 个 z 控制点
    };

    QStackedWidget *m_inner = nullptr;

    QLineEdit *m_shapeFamily = nullptr;
    QLineEdit *m_paramVersion = nullptr;
    QLineEdit *m_axialLength = nullptr;
    QLineEdit *m_inletArea = nullptr;
    QLineEdit *m_outletArea = nullptr;
    QLineEdit *m_inletPitch = nullptr;
    QLineEdit *m_inletExtend = nullptr;
    QLineEdit *m_outletPitch = nullptr;
    QLineEdit *m_outletExtend = nullptr;
    QVector<RibEdits> m_ribEdits;
    QLabel *m_source = nullptr;
    QLabel *m_paramStatus = nullptr;
    QLabel *m_constraintSource = nullptr;
    QVBoxLayout *m_constraintHost = nullptr;

    DuctParams m_params;
    std::unique_ptr<DuctDesignService> m_service;
    DuctDesignPresenter *m_presenter = nullptr;
};

#endif
