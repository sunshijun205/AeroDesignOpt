#ifndef ANALYSISPAGE_H
#define ANALYSISPAGE_H

#include "airfoiltypes.h"

#include <QWidget>
#include <memory>

class QLineEdit;
class QLabel;
class QStackedWidget;
class AeroAnalysisService;
class AirfoilStore;
class AnalysisPresenter;

// 气动分析页（分层样板 View）。
// 持有 Store/Service/Presenter；只发意图信号、只被动接受刷新，不直接取数/写业务。
class AnalysisPage : public QWidget
{
    Q_OBJECT
public:
    explicit AnalysisPage(QWidget *parent = nullptr);
    ~AnalysisPage() override;

    AirfoilInput snapshotInput() const;

    // 顶栏主/次按钮转发。
    void requestRun();
    void requestSave();

signals:
    void runRequested(const AirfoilInput &input);
    void saveRequested();

public slots:
    void setInput(const AirfoilInput &input);
    void setResult(const AeroResult &result);
    void setStatus(const QString &text, bool warn = false);
    void showError(const QString &message);
    void setBusy(bool busy);

private:
    QWidget *buildConditionPage();
    QWidget *buildSolverPage();
    QWidget *buildResultPage();

    QStackedWidget *m_inner = nullptr;

    QLineEdit *m_naca = nullptr;
    QLineEdit *m_alpha = nullptr;
    QLineEdit *m_reynolds = nullptr;
    QLineEdit *m_mach = nullptr;
    QLineEdit *m_aspect = nullptr;

    QLabel *m_clValue = nullptr;
    QLabel *m_cdValue = nullptr;
    QLabel *m_ldValue = nullptr;
    QLabel *m_cmValue = nullptr;
    QLabel *m_clAlphaValue = nullptr;
    QLabel *m_stallValue = nullptr;
    QLabel *m_status = nullptr;

    std::unique_ptr<AeroAnalysisService> m_service;
    std::unique_ptr<AirfoilStore> m_store;
    AnalysisPresenter *m_presenter = nullptr;
};

#endif
