#include "analysispresenter.h"

#include "aeroanalysisservice.h"
#include "airfoilstore.h"
#include "analysispage.h"

AnalysisPresenter::AnalysisPresenter(AnalysisPage *view,
                                     AeroAnalysisService *service,
                                     AirfoilStore *store,
                                     QObject *parent)
    : QObject(parent), m_view(view), m_service(service), m_store(store)
{
    connect(m_view, &AnalysisPage::runRequested, this, &AnalysisPresenter::onRunRequested);
    connect(m_view, &AnalysisPage::saveRequested, this, &AnalysisPresenter::onSaveRequested);
}

void AnalysisPresenter::initialize()
{
    bool ok = false;
    const AirfoilInput input = m_store->load(&ok);
    if (ok)
        m_view->setInput(input);
}

void AnalysisPresenter::onRunRequested(const AirfoilInput &input)
{
    m_view->setBusy(true);

    QString err;
    const AeroResult result = m_service->run(input, &err);
    if (!result.valid) {
        m_view->showError(err.isEmpty() ? QString::fromUtf8("计算失败。") : err);
        m_view->setBusy(false);
        return;
    }

    QString saveErr;
    if (!m_store->save(input, &saveErr)) {
        m_view->setResult(result);
        m_view->setStatus(saveErr, true);
        m_view->setBusy(false);
        return;
    }

    m_view->setResult(result);
    m_view->setStatus(QString::fromUtf8("计算完成，已保存到 %1").arg(m_store->filePath()));
    m_view->setBusy(false);
}

void AnalysisPresenter::onSaveRequested()
{
    QString err;
    if (!m_store->save(m_view->snapshotInput(), &err)) {
        m_view->showError(err);
        return;
    }
    m_view->setStatus(QString::fromUtf8("已保存到 %1").arg(m_store->filePath()));
}
