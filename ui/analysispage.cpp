#include "analysispage.h"

#include "aeroanalysisservice.h"
#include "airfoilstore.h"
#include "analysispresenter.h"
#include "uihelpers.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QStackedWidget>
#include <QStyle>
#include <QVBoxLayout>

namespace {
QString num(double v, int prec)
{
    return QString::number(v, 'f', prec);
}
}

AnalysisPage::AnalysisPage(QWidget *parent)
    : QWidget(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(22, 18, 22, 18);
    outer->setSpacing(14);

    outer->addWidget(makeHeading(
        QString::fromUtf8("气动分析"),
        QString::fromUtf8("翼型单点气动分析（工程估算：薄翼理论 + 阻力极曲线）。修改工况后点击「运行气动分析」。")));

    auto *tabs = new SubTabBar({
        {QStringLiteral("condition"), QString::fromUtf8("工况")},
        {QStringLiteral("solver"), QString::fromUtf8("求解设置")},
        {QStringLiteral("result"), QString::fromUtf8("结果")},
    }, QStringLiteral("condition"));
    outer->addWidget(tabs);

    m_inner = new QStackedWidget;
    m_inner->addWidget(wrapScroll(buildConditionPage()));
    m_inner->addWidget(wrapScroll(buildSolverPage()));
    m_inner->addWidget(wrapScroll(buildResultPage()));
    outer->addWidget(m_inner, 1);

    connect(tabs, &SubTabBar::currentChanged, this, [this](const QString &id) {
        if (id == QLatin1String("condition"))
            m_inner->setCurrentIndex(0);
        else if (id == QLatin1String("solver"))
            m_inner->setCurrentIndex(1);
        else
            m_inner->setCurrentIndex(2);
    });

    m_service = std::make_unique<AeroAnalysisService>();
    m_store = std::make_unique<AirfoilStore>();
    m_presenter = new AnalysisPresenter(this, m_service.get(), m_store.get(), this);
    m_presenter->initialize();
}

AnalysisPage::~AnalysisPage() = default;

QWidget *AnalysisPage::buildConditionPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("翼型与工况"),
                                 QString::fromUtf8("4 位 NACA"),
                                 QString::fromUtf8("展弦比用于有限翼升力线斜率修正；马赫数用于亚声速可压缩性修正。")));

    m_naca = makeInput(QStringLiteral("2412"));
    m_alpha = makeInput(QStringLiteral("4"));
    m_reynolds = makeInput(QStringLiteral("3000000"));
    m_mach = makeInput(QStringLiteral("0.2"));
    m_aspect = makeInput(QStringLiteral("8"));

    QList<QWidget *> fields = {
        makeLabeled(QString::fromUtf8("NACA 翼型"), m_naca),
        makeLabeled(QString::fromUtf8("迎角 α"), m_alpha, QStringLiteral("deg")),
        makeLabeled(QString::fromUtf8("雷诺数 Re"), m_reynolds),
        makeLabeled(QString::fromUtf8("马赫数 Ma"), m_mach),
        makeLabeled(QString::fromUtf8("展弦比 AR"), m_aspect),
    };
    pl->addWidget(makeFieldGrid(fields, 2));

    m_status = qobject_cast<QLabel *>(makeStatusText(QString::fromUtf8("尚未计算。")));
    pl->addWidget(m_status);

    lay->addWidget(panel);
    lay->addStretch();
    return content;
}

QWidget *AnalysisPage::buildSolverPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("求解设置"),
                                 QString::fromUtf8("原型"),
                                 QString::fromUtf8("当前使用内置工程估算模型，以下为后续接入面元法 / CFD 的占位设置。")));

    QList<QWidget *> fields = {
        makeSelectField(QString::fromUtf8("求解方法"), QString::fromUtf8("薄翼理论（内置）"),
                        {QString::fromUtf8("面元法（待接入）"), QString::fromUtf8("RANS CFD（待接入）")}),
        makeSelectField(QString::fromUtf8("湍流模型"), QString::fromUtf8("无（无粘）"),
                        {QStringLiteral("SA"), QStringLiteral("k-omega SST")}),
        makeField(QString::fromUtf8("最大迭代步"), QStringLiteral("500")),
        makeField(QString::fromUtf8("收敛残差"), QStringLiteral("1e-6")),
    };
    pl->addWidget(makeFieldGrid(fields, 2));

    lay->addWidget(panel);
    lay->addStretch();
    return content;
}

QWidget *AnalysisPage::buildResultPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *kpiPanel = makePanel();
    auto *kl = qobject_cast<QVBoxLayout *>(kpiPanel->layout());
    kl->addWidget(makePanelTitle(QString::fromUtf8("气动系数"),
                                 QString::fromUtf8("工程估算")));

    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(12);
    auto addKpi = [&](int r, int c, const QString &cap, QLabel **valueOut, const QString &unit) {
        auto *card = new QFrame;
        card->setObjectName(QStringLiteral("KpiCard"));
        auto *cl = new QVBoxLayout(card);
        cl->setContentsMargins(14, 12, 14, 12);
        cl->setSpacing(4);
        auto *capLbl = new QLabel(cap);
        capLbl->setObjectName(QStringLiteral("KpiCaption"));
        auto *row = new QHBoxLayout;
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(4);
        auto *val = new QLabel(QStringLiteral("—"));
        val->setObjectName(QStringLiteral("KpiValue"));
        row->addWidget(val);
        if (!unit.isEmpty()) {
            auto *u = new QLabel(unit);
            u->setObjectName(QStringLiteral("KpiUnit"));
            row->addWidget(u);
        }
        row->addStretch();
        cl->addWidget(capLbl);
        cl->addLayout(row);
        grid->addWidget(card, r, c);
        *valueOut = val;
    };
    addKpi(0, 0, QString::fromUtf8("升力系数 Cl"), &m_clValue, {});
    addKpi(0, 1, QString::fromUtf8("阻力系数 Cd"), &m_cdValue, {});
    addKpi(0, 2, QString::fromUtf8("升阻比 L/D"), &m_ldValue, {});
    addKpi(1, 0, QString::fromUtf8("力矩系数 Cm"), &m_cmValue, {});
    addKpi(1, 1, QString::fromUtf8("升力线斜率"), &m_clAlphaValue, QStringLiteral("1/rad"));
    addKpi(1, 2, QString::fromUtf8("失速状态"), &m_stallValue, {});
    kl->addLayout(grid);

    lay->addWidget(kpiPanel);

    auto *note = makePanel();
    auto *nl = qobject_cast<QVBoxLayout *>(note->layout());
    nl->addWidget(makePanelTitle(QString::fromUtf8("说明")));
    auto *text = new QLabel(QString::fromUtf8(
        "结果由内置工程估算模型给出（薄翼理论升力 + 阻力极曲线），用于概念阶段快速评估，"
        "非高保真结果。运行后自动保存本次工况到本地。"));
    text->setObjectName(QStringLiteral("NoteLabel"));
    text->setWordWrap(true);
    nl->addWidget(text);
    lay->addWidget(note);

    lay->addStretch();
    return content;
}

AirfoilInput AnalysisPage::snapshotInput() const
{
    AirfoilInput input;
    input.naca = m_naca->text().trimmed();
    input.alphaDeg = m_alpha->text().toDouble();
    input.reynolds = m_reynolds->text().toDouble();
    input.mach = m_mach->text().toDouble();
    input.aspectRatio = m_aspect->text().toDouble();
    input.nacaKnown = input.alphaKnown = input.reynoldsKnown = true;
    input.machKnown = input.aspectRatioKnown = true;
    return input;
}

void AnalysisPage::requestRun()
{
    emit runRequested(snapshotInput());
}

void AnalysisPage::requestSave()
{
    emit saveRequested();
}

void AnalysisPage::setInput(const AirfoilInput &input)
{
    if (input.nacaKnown)
        m_naca->setText(input.naca);
    if (input.alphaKnown)
        m_alpha->setText(QString::number(input.alphaDeg));
    if (input.reynoldsKnown)
        m_reynolds->setText(QString::number(input.reynolds, 'g', 10));
    if (input.machKnown)
        m_mach->setText(QString::number(input.mach));
    if (input.aspectRatioKnown)
        m_aspect->setText(QString::number(input.aspectRatio));
}

void AnalysisPage::setResult(const AeroResult &result)
{
    if (!result.valid)
        return;
    m_clValue->setText(num(result.cl, 3));
    m_cdValue->setText(num(result.cd, 4));
    m_ldValue->setText(num(result.lOverD, 1));
    m_cmValue->setText(num(result.cm, 3));
    m_clAlphaValue->setText(num(result.clAlpha, 2));
    m_stallValue->setText(result.stalled ? QString::fromUtf8("已失速")
                                         : QString::fromUtf8("未失速"));
    m_inner->setCurrentIndex(2);
}

void AnalysisPage::setStatus(const QString &text, bool warn)
{
    if (!m_status)
        return;
    m_status->setText(text);
    m_status->setObjectName(warn ? QStringLiteral("StatusWarn") : QStringLiteral("StatusGood"));
    m_status->style()->unpolish(m_status);
    m_status->style()->polish(m_status);
}

void AnalysisPage::showError(const QString &message)
{
    QMessageBox::warning(this, QString::fromUtf8("气动设计优化平台"), message);
    setStatus(message, true);
}

void AnalysisPage::setBusy(bool busy)
{
    setEnabled(!busy);
}
