#include "ductdesignpresenter.h"

#include "ductdesignpage.h"
#include "ductdesignservice.h"
#include "ductparams.h"

DuctDesignPresenter::DuctDesignPresenter(DuctDesignPage *view, DuctDesignService *service, QObject *parent)
    : QObject(parent), m_view(view), m_service(service)
{
    // 初始展示内置默认约束（在任何载入之前）。
    m_constraints = m_service->defaultConstraints();
    m_view->setConstraints(m_constraints);
}

void DuctDesignPresenter::loadFrom(const QString &path, bool interactive)
{
    if (!m_view || !m_service)
        return;
    DuctParams params;
    QString error;
    if (m_service->load(path, params, error)) {
        // 依据 design.json 位置解析约束来源（配置文件优先，否则默认）。
        m_constraints = m_service->resolveConstraints(path);
        m_view->setConstraints(m_constraints);
        m_view->setParams(params);
        m_view->setValidation(m_service->validate(params, m_constraints));
    } else if (interactive) {
        m_view->showError(error);
    } else {
        m_view->setParamStatus(error, true);
    }
}

void DuctDesignPresenter::onLoadRequested(const QString &path)
{
    loadFrom(path, true);
}

void DuctDesignPresenter::onValidateRequested()
{
    if (!m_view || !m_service)
        return;
    m_view->setValidation(m_service->validate(m_view->snapshotParams(), m_constraints));
}

void DuctDesignPresenter::onExportRequested(const QString &path)
{
    if (!m_view || !m_service)
        return;
    const DuctParams params = m_view->snapshotParams();
    const QStringList issues = m_service->validate(params, m_constraints);
    if (!issues.isEmpty()) {
        m_view->setValidation(issues);
        m_view->showError(QString::fromUtf8("参数未通过校验，已阻止导出：\n%1")
                              .arg(issues.join(QLatin1Char('\n'))));
        return;
    }
    QString error;
    if (m_service->exportSnapshot(params, path, error))
        m_view->showExportOk(path);
    else
        m_view->showError(error);
}
