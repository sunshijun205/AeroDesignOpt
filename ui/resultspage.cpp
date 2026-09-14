#include "resultspage.h"

#include "uihelpers.h"

#include <QHBoxLayout>
#include <QStackedWidget>
#include <QVBoxLayout>

ResultsPage::ResultsPage(QWidget *parent)
    : QWidget(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(22, 18, 22, 18);
    outer->setSpacing(14);

    outer->addWidget(makeHeading(
        QString::fromUtf8("结果对比"),
        QString::fromUtf8("查看 Pareto 候选、并排对比方案，并核对代理预测与独立 CFD 结果，形成可追溯的方案选择。原型页：内容为静态示意。")));

    auto *tabs = new SubTabBar({
        {QStringLiteral("pareto"), QString::fromUtf8("Pareto 前沿")},
        {QStringLiteral("compare"), QString::fromUtf8("方案对比")},
        {QStringLiteral("detail"), QString::fromUtf8("方案详情")},
    }, QStringLiteral("pareto"));
    outer->addWidget(tabs);

    m_inner = new QStackedWidget;
    m_inner->addWidget(wrapScroll(buildParetoPage()));
    m_inner->addWidget(wrapScroll(buildComparePage()));
    m_inner->addWidget(wrapScroll(buildDetailPage()));
    outer->addWidget(m_inner, 1);

    connect(tabs, &SubTabBar::currentChanged, this, [this](const QString &id) {
        if (id == QLatin1String("pareto"))
            m_inner->setCurrentIndex(0);
        else if (id == QLatin1String("compare"))
            m_inner->setCurrentIndex(1);
        else
            m_inner->setCurrentIndex(2);
    });
}

QWidget *ResultsPage::buildParetoPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("Pareto 前沿"), QString::fromUtf8("隐身 ↔ 气动"),
                                 QString::fromUtf8("横轴可见度 V，纵轴总压损失 ω；降低可见度会带来损失上升，需在其中选取折中方案。")));
    pl->addWidget(makeCanvas(
        QString::fromUtf8("Pareto 散点图（占位）\n横轴：可见度 V (%)　纵轴：总压损失 ω (%)\n标注 DC60/SC60 均 < 20% 的可行成员"), 280));
    lay->addWidget(panel);

    auto *tbl = makePanel();
    auto *tl = qobject_cast<QVBoxLayout *>(tbl->layout());
    tl->addWidget(makePanelTitle(QString::fromUtf8("候选成员"), QString::fromUtf8("Pareto 上代表点")));
    tl->addWidget(makeTable(
        {QString::fromUtf8("成员"), QString::fromUtf8("可见度 V"), QStringLiteral("ω"), QStringLiteral("π"), QStringLiteral("DC60"), QStringLiteral("SC60")},
        {
            {QStringLiteral("5454"), QStringLiteral("56.69%"), QStringLiteral("11.45%"), QStringLiteral("97.49%"), QStringLiteral("7.45%"), QStringLiteral("9.80%")},
            {QStringLiteral("5763"), QStringLiteral("31.44%"), QStringLiteral("13.37%"), QStringLiteral("97.09%"), QStringLiteral("14.12%"), QStringLiteral("17.28%")},
            {QStringLiteral("5969"), QStringLiteral("16.32%"), QStringLiteral("17.10%"), QStringLiteral("96.23%"), QStringLiteral("48.48%"), QStringLiteral("31.50%")},
        },
        TableOptions{{2}, {}}));
    lay->addWidget(tbl);

    lay->addWidget(makeStatusText(
        QString::fromUtf8("说明：成员 5969 可见度最低但畸变/旋流超限；成员 5763 为隐身与气动可接受的折中候选。")));
    lay->addStretch();
    return content;
}

QWidget *ResultsPage::buildComparePage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("候选方案并排对比"), QString::fromUtf8("相对基准")));
    pl->addWidget(makeTable(
        {QString::fromUtf8("指标"), QString::fromUtf8("基准 S-01"), QString::fromUtf8("成员 5454"), QString::fromUtf8("成员 5763"), QString::fromUtf8("成员 5969")},
        {
            {QString::fromUtf8("可见度 V"), QStringLiteral("68.0%"), QStringLiteral("56.69%"), QStringLiteral("31.44%"), QStringLiteral("16.32%")},
            {QString::fromUtf8("总压损失 ω"), QStringLiteral("12.9%"), QStringLiteral("11.45%"), QStringLiteral("13.37%"), QStringLiteral("17.10%")},
            {QString::fromUtf8("总压恢复 σ"), QStringLiteral("0.968"), QStringLiteral("0.975"), QStringLiteral("0.971"), QStringLiteral("0.962")},
            {QStringLiteral("DC60"), QStringLiteral("9.8%"), QStringLiteral("7.45%"), QStringLiteral("14.12%"), QStringLiteral("48.48%")},
            {QStringLiteral("SC60"), QStringLiteral("11.0%"), QStringLiteral("9.80%"), QStringLiteral("17.28%"), QStringLiteral("31.50%")},
            {QString::fromUtf8("可见倾角范围 Δγ"), QStringLiteral("—"), QStringLiteral("24.7°"), QStringLiteral("18.1°"), QStringLiteral("10.3°")},
            {QString::fromUtf8("综合评价"), QString::fromUtf8("基准"), QString::fromUtf8("气动优/隐身弱"), QString::fromUtf8("推荐折中"), QString::fromUtf8("隐身优/超限")},
        },
        TableOptions{{3, 4}, {}}));
    lay->addWidget(panel);
    lay->addStretch();
    return content;
}

QWidget *ResultsPage::buildDetailPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *head = makePanel();
    auto *hl = qobject_cast<QVBoxLayout *>(head->layout());
    hl->addWidget(makePanelTitle(QString::fromUtf8("方案详情 — 成员 5763"), QString::fromUtf8("推荐方案")));
    hl->addWidget(makeKpis({
        {QString::fromUtf8("可见度 V"), QStringLiteral("31.4"), QStringLiteral("%")},
        {QString::fromUtf8("总压恢复 σ"), QStringLiteral("0.971"), {}},
        {QStringLiteral("DC60"), QStringLiteral("14.1"), QStringLiteral("%")},
        {QStringLiteral("SC60"), QStringLiteral("17.3"), QStringLiteral("%")},
    }));
    lay->addWidget(head);

    auto *views = new QWidget;
    auto *vl = new QHBoxLayout(views);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(14);

    auto *geom = makePanel();
    auto *gl = qobject_cast<QVBoxLayout *>(geom->layout());
    gl->addWidget(makePanelTitle(QString::fromUtf8("外形与遮挡")));
    gl->addWidget(makeCanvas(QString::fromUtf8("三维外形 + 正视遮挡视图（占位）"), 220));
    vl->addWidget(geom, 1);

    auto *flow = makePanel();
    auto *fl = qobject_cast<QVBoxLayout *>(flow->layout());
    fl->addWidget(makePanelTitle(QString::fromUtf8("AIP 流场")));
    fl->addWidget(makeCanvas(QString::fromUtf8("AIP 总压分布云图（占位）"), 220));
    vl->addWidget(flow, 1);
    lay->addWidget(views);

    auto *verify = makePanel();
    auto *vfl = qobject_cast<QVBoxLayout *>(verify->layout());
    vfl->addWidget(makePanelTitle(QString::fromUtf8("代理预测 vs 独立 CFD 核验"), QString::fromUtf8("分别记录")));
    vfl->addWidget(makeTable(
        {QString::fromUtf8("指标"), QString::fromUtf8("代理预测"), QString::fromUtf8("CFD 核验"), QString::fromUtf8("偏差")},
        {
            {QString::fromUtf8("总压恢复 σ"), QStringLiteral("0.971"), QStringLiteral("0.970"), QStringLiteral("+0.1%")},
            {QString::fromUtf8("总压损失 ω"), QStringLiteral("13.37%"), QStringLiteral("13.62%"), QStringLiteral("-0.25%")},
            {QStringLiteral("DC60"), QStringLiteral("14.12%"), QStringLiteral("14.80%"), QStringLiteral("-0.68%")},
            {QStringLiteral("SC60"), QStringLiteral("17.28%"), QStringLiteral("18.05%"), QStringLiteral("-0.77%")},
        },
        TableOptions{}));
    lay->addWidget(verify);

    lay->addWidget(makeStatusText(
        QString::fromUtf8("代理预测与 CFD 核验偏差在客户确认精度内；该方案在容许损失下遮挡正视 AIP，作为最终交付候选。")));
    lay->addStretch();
    return content;
}
