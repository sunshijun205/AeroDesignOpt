#include "surrogatepage.h"

#include "uihelpers.h"

#include <QStackedWidget>
#include <QVBoxLayout>

SurrogatePage::SurrogatePage(QWidget *parent)
    : QWidget(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(22, 18, 22, 18);
    outer->setSpacing(14);

    outer->addWidget(makeHeading(
        QString::fromUtf8("气动代理"),
        QString::fromUtf8("用机器学习代理模型替代逐次 CFD，快速预测设计参数与工况下的气动性能，并标注误差与适用范围。原型页：内容为静态示意。")));

    auto *tabs = new SubTabBar({
        {QStringLiteral("condition"), QString::fromUtf8("工况输入")},
        {QStringLiteral("model"), QString::fromUtf8("模型与适用范围")},
        {QStringLiteral("result"), QString::fromUtf8("性能预测")},
    }, QStringLiteral("condition"));
    outer->addWidget(tabs);

    m_inner = new QStackedWidget;
    m_inner->addWidget(wrapScroll(buildConditionPage()));
    m_inner->addWidget(wrapScroll(buildModelPage()));
    m_inner->addWidget(wrapScroll(buildResultPage()));
    outer->addWidget(m_inner, 1);

    connect(tabs, &SubTabBar::currentChanged, this, [this](const QString &id) {
        if (id == QLatin1String("condition"))
            m_inner->setCurrentIndex(0);
        else if (id == QLatin1String("model"))
            m_inner->setCurrentIndex(1);
        else
            m_inner->setCurrentIndex(2);
    });
}

QWidget *SurrogatePage::buildConditionPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("来流与边界条件"), QString::fromUtf8("巡航点")));
    QList<QWidget *> fields = {
        makeField(QString::fromUtf8("来流马赫数"), QStringLiteral("0.80")),
        makeField(QString::fromUtf8("飞行高度"), QStringLiteral("11000"), QStringLiteral("m")),
        makeField(QString::fromUtf8("攻角"), QStringLiteral("0.0"), QStringLiteral("deg")),
        makeField(QString::fromUtf8("侧滑角"), QStringLiteral("0.0"), QStringLiteral("deg")),
        makeSelectField(QString::fromUtf8("出口边界"), QString::fromUtf8("定质量流量"),
                        {QString::fromUtf8("定背压")}),
        makeField(QString::fromUtf8("质量流量"), QStringLiteral("42.0"), QString::fromUtf8("kg/s")),
    };
    pl->addWidget(makeFieldGrid(fields, 2));
    lay->addWidget(panel);

    auto *batch = makePanel();
    auto *bl = qobject_cast<QVBoxLayout *>(batch->layout());
    bl->addWidget(makePanelTitle(QString::fromUtf8("多工况批量"), QString::fromUtf8("可选")));
    bl->addWidget(makeTable(
        {QString::fromUtf8("工况"), QString::fromUtf8("马赫数"), QString::fromUtf8("攻角"), QString::fromUtf8("质量流量"), QString::fromUtf8("权重")},
        {
            {QString::fromUtf8("巡航"), QStringLiteral("0.80"), QStringLiteral("0.0°"), QStringLiteral("42.0"), QStringLiteral("0.6")},
            {QString::fromUtf8("爬升"), QStringLiteral("0.65"), QStringLiteral("4.0°"), QStringLiteral("38.0"), QStringLiteral("0.25")},
            {QString::fromUtf8("高速"), QStringLiteral("0.88"), QStringLiteral("-2.0°"), QStringLiteral("45.0"), QStringLiteral("0.15")},
        },
        TableOptions{}));
    lay->addWidget(batch);
    lay->addStretch();
    return content;
}

QWidget *SurrogatePage::buildModelPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *sel = makePanel();
    auto *sl = qobject_cast<QVBoxLayout *>(sel->layout());
    sl->addWidget(makePanelTitle(QString::fromUtf8("模型选择"), QString::fromUtf8("已训练")));
    QList<QWidget *> fields = {
        makeSelectField(QString::fromUtf8("代理模型"), QString::fromUtf8("Kriging（高斯过程）"),
                        {QString::fromUtf8("神经网络"), QString::fromUtf8("Co-Kriging 多精度")}),
        makeSelectField(QString::fromUtf8("模型版本"), QStringLiteral("v3-2026w32"),
                        {QStringLiteral("v2-2026w20")}),
        makeField(QString::fromUtf8("训练样本"), QStringLiteral("248")),
        makeField(QString::fromUtf8("独立验证样本"), QStringLiteral("64")),
    };
    sl->addWidget(makeFieldGrid(fields, 2));
    lay->addWidget(sel);

    auto *acc = makePanel();
    auto *al = qobject_cast<QVBoxLayout *>(acc->layout());
    al->addWidget(makePanelTitle(QString::fromUtf8("各指标验证误差"), QString::fromUtf8("独立样本 MAE")));
    al->addWidget(makeTable(
        {QString::fromUtf8("指标"), QString::fromUtf8("平均误差"), QString::fromUtf8("最大误差"), QString::fromUtf8("排序一致性")},
        {
            {QString::fromUtf8("总压恢复系数 σ"), QStringLiteral("0.4%"), QStringLiteral("1.2%"), QString::fromUtf8("高")},
            {QStringLiteral("总压损失 ω"), QStringLiteral("0.9%"), QStringLiteral("2.8%"), QString::fromUtf8("高")},
            {QStringLiteral("DC60"), QStringLiteral("1.6%"), QStringLiteral("5.4%"), QString::fromUtf8("中")},
            {QStringLiteral("SC60"), QStringLiteral("2.1%"), QStringLiteral("6.9%"), QString::fromUtf8("中")},
        },
        TableOptions{{2, 3}, {}}));
    lay->addWidget(acc);

    auto *rng = makePanel();
    auto *rl = qobject_cast<QVBoxLayout *>(rng->layout());
    rl->addWidget(makePanelTitle(QString::fromUtf8("已验证适用范围"), QString::fromUtf8("超出将提示")));
    rl->addWidget(makeTable(
        {QString::fromUtf8("变量"), QString::fromUtf8("下限"), QString::fromUtf8("上限"), QString::fromUtf8("说明")},
        {
            {QString::fromUtf8("来流马赫数"), QStringLiteral("0.60"), QStringLiteral("0.88"), QString::fromUtf8("巡航包线内")},
            {QString::fromUtf8("攻角"), QStringLiteral("-4°"), QStringLiteral("+6°"), QString::fromUtf8("含设计边界")},
            {QString::fromUtf8("中弧线下沉"), QStringLiteral("0.10 m"), QStringLiteral("0.28 m"), QString::fromUtf8("S 弯幅度")},
        },
        TableOptions{}));
    lay->addWidget(rng);
    lay->addStretch();
    return content;
}

QWidget *SurrogatePage::buildResultPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    lay->addWidget(makeKpis({
        {QString::fromUtf8("总压恢复 σ"), QStringLiteral("0.971"), {}},
        {QString::fromUtf8("总压损失 ω"), QStringLiteral("13.4"), QStringLiteral("%")},
        {QStringLiteral("DC60"), QStringLiteral("14.1"), QStringLiteral("%")},
        {QStringLiteral("SC60"), QStringLiteral("17.3"), QStringLiteral("%")},
    }));

    auto *dist = makePanel();
    auto *dl = qobject_cast<QVBoxLayout *>(dist->layout());
    dl->addWidget(makePanelTitle(QString::fromUtf8("出口(AIP)总压分布"), QString::fromUtf8("辅助输出")));
    dl->addWidget(makeCanvas(
        QString::fromUtf8("AIP 总压/速度分布云图（占位）\n上部低能流体区、周向不均匀性示意\n后续接入自定义绘制或流场读取结果"), 260));
    lay->addWidget(dist);

    auto *meta = makePanel();
    auto *ml = qobject_cast<QVBoxLayout *>(meta->layout());
    ml->addWidget(makePanelTitle(QString::fromUtf8("预测元信息"), QString::fromUtf8("可追溯")));
    ml->addWidget(makeTable(
        {QString::fromUtf8("项"), QString::fromUtf8("值")},
        {
            {QString::fromUtf8("模型版本"), QStringLiteral("v3-2026w32")},
            {QString::fromUtf8("输入是否在适用范围"), QString::fromUtf8("是")},
            {QString::fromUtf8("单次预测耗时"), QStringLiteral("8 ms")},
            {QString::fromUtf8("不确定度(σ 估计)"), QStringLiteral("±0.004")},
        },
        TableOptions{}));
    lay->addWidget(meta);

    lay->addWidget(makeStatusText(
        QString::fromUtf8("预测在已验证范围内，可用于优化排序；若输入超范围，将给出明确提示并需 CFD 核验后再用于设计判断。")));
    lay->addStretch();
    return content;
}
