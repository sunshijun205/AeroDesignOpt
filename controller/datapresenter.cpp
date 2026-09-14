#include "datapresenter.h"

#include "datapage.h"
#include "datasetservice.h"
#include "ductdataset.h"

DataPresenter::DataPresenter(DataPage *view, DatasetService *service, QObject *parent)
    : QObject(parent), m_view(view), m_service(service)
{
}

void DataPresenter::onImportRequested(const QString &rootDir)
{
    importFrom(rootDir, true);
}

void DataPresenter::onExportRequested(const QString &outDir)
{
    if (!m_view || !m_service)
        return;
    QStringList written;
    QString error;
    if (m_service->exportReports(m_view->currentSummary(), outDir, written, error))
        m_view->showExportResult(written);
    else
        m_view->showError(error);
}

void DataPresenter::importFrom(const QString &rootDir, bool interactive)
{
    if (!m_view || !m_service)
        return;
    DatasetSummary summary;
    QString error;
    if (m_service->load(rootDir, summary, error)) {
        m_view->setSummary(summary);
    } else if (interactive) {
        m_view->showError(error);
    } else {
        m_view->setPlaceholder(error);
    }
}
