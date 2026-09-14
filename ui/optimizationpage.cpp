#include "optimizationpage.h"

#include "uihelpers.h"

#include <QLabel>
#include <QProgressBar>
#include <QStackedWidget>
#include <QVBoxLayout>

OptimizationPage::OptimizationPage(QWidget *parent)
    : QWidget(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(22, 18, 22, 18);
    outer->setSpacing(14);

    outer->addWidget(makeHeading(
        QString::fromUtf8("优化"),
        QString::fromUtf8("在遮挡、接口与空间约束限定的可行域内，以总压恢复为主目标搜索“隐身 ↔ 气动”折中方案。原型页：内容为静态示意。")));

    auto *tabs = new SubTabBar({
        {QStringLiteral("variables"), QString::fromUtf8("设计变量")},
        {QStringLiteral("objective"), QString::fromUtf8("目标与约束")},
        {QStringLiteral("run"), QString::fromUtf8("运行监控")},
    }, QStringLiteral("variables"));
    outer->addWidget(tabs);

    m_inner = new QStackedWidget;
    m_inner->addWidget(wrapScroll(buildVariablesPage()));
    m_inner->addWidget(wrapScroll(buildObjectivePage()));
    m_inner->addWidget(wrapScroll(buildRunPage()));
    outer->addWidget(m_inner, 1);

    connect(tabs, &SubTabBar::currentChanged, this, [this](const QString &id) {
        if (id == QLatin1String("variables"))
            m_inner->setCurrentIndex(0);
        else if (id == QLatin1String("objective"))
            m_inner->setCurrentIndex(1);
        else
            m_inner->setCurrentIndex(2);
    });
}

QWidget *OptimizationPage::buildVariablesPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("设计变量与范围"), QString::fromUtf8("共 27 维"),
                                 QString::fromUtf8("搜索范围结合代理模型经过验证的适用范围确定，超范围区间将被限制。")));
    pl->addWidget(makeTable(
        {QString::fromUtf8("变量"), QString::fromUtf8("下限"), QString::fromUtf8("上限"), QString::fromUtf8("基准值"), QString::fromUtf8("状态")},
        {
            {QString::fromUtf8("入口俯仰角"), QStringLiteral("-8°"), QStringLiteral("0°"), QStringLiteral("-4°"), QString::fromUtf8("自由")},
            {QString::fromUtf8("出口俯仰角"), QStringLiteral("2°"), QStringLiteral("10°"), QStringLiteral("6.5°"), QString::fromUtf8("自由")},
            {QString::fromUtf8("Rib-2 下沉"), QStringLiteral("-0.24 m"), QStringLiteral("-0.06 m"), QStringLiteral("-0.14 m"), QString::fromUtf8("自由")},
            {QString::fromUtf8("Rib-2 缩放"), QStringLiteral("0.80"), QStringLiteral("1.05"), QStringLiteral("0.90"), QString::fromUtf8("自由")},
            {QString::fromUtf8("Rib-1 位置"), QStringLiteral("0.20"), QStringLiteral("0.35"), QStringLiteral("0.25"), QString::fromUtf8("自由")},
            {QString::fromUtf8("入口面积"), QStringLiteral("—"), QStringLiteral("—"), QStringLiteral("0.42 m²"), QString::fromUtf8("固定")},
        },
        TableOptions{{5}, {}}));
    lay->addWidget(panel);

    lay->addWidget(makeStatusText(
        QString::fromUtf8("提示：共 22 个自由变量、5 个固定；此处仅列出代表性变量。")));
    lay->addStretch();
    return content;
}

QWidget *OptimizationPage::buildObjectivePage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *obj = makePanel();
    auto *ol = qobject_cast<QVBoxLayout *>(obj->layout());
    ol->addWidget(makePanelTitle(QString::fromUtf8("优化目标"), QString::fromUtf8("多目标")));
    ol->addWidget(makeTable(
        {QString::fromUtf8("目标"), QString::fromUtf8("方向"), QString::fromUtf8("优先级"), QString::fromUtf8("说明")},
        {
            {QString::fromUtf8("总压恢复系数 σ"), QString::fromUtf8("最大化"), QString::fromUtf8("主目标"), QString::fromUtf8("气动效率")},
            {QString::fromUtf8("可见度 V"), QString::fromUtf8("最小化"), QString::fromUtf8("主目标"), QString::fromUtf8("隐身/遮挡")},
        },
        TableOptions{}));
    lay->addWidget(obj);

    auto *con = makePanel();
    auto *cl = qobject_cast<QVBoxLayout *>(con->layout());
    cl->addWidget(makePanelTitle(QString::fromUtf8("约束"), QString::fromUtf8("硬/软")));
    cl->addWidget(makeTable(
        {QString::fromUtf8("约束"), QString::fromUtf8("限值"), QString::fromUtf8("类型"), QString::fromUtf8("处理")},
        {
            {QString::fromUtf8("质量流量"), QStringLiteral("= 42 kg/s"), QString::fromUtf8("硬约束"), QString::fromUtf8("作为边界条件")},
            {QStringLiteral("DC60"), QStringLiteral("≤ 20%"), QString::fromUtf8("硬约束"), QString::fromUtf8("越界剔除")},
            {QStringLiteral("SC60"), QStringLiteral("≤ 20%"), QString::fromUtf8("硬约束"), QString::fromUtf8("越界剔除")},
            {QString::fromUtf8("安装空间"), QString::fromUtf8("包络内"), QString::fromUtf8("硬约束"), QString::fromUtf8("几何预筛")},
            {QString::fromUtf8("全遮挡"), QString::fromUtf8("尽量小 V"), QString::fromUtf8("目标化"), QString::fromUtf8("Pareto 权衡")},
        },
        TableOptions{}));
    lay->addWidget(con);

    auto *stop = makePanel();
    auto *st = qobject_cast<QVBoxLayout *>(stop->layout());
    st->addWidget(makePanelTitle(QString::fromUtf8("算法与停止条件"), QString::fromUtf8("代理辅助进化")));
    QList<QWidget *> fields = {
        makeSelectField(QString::fromUtf8("优化算法"), QString::fromUtf8("代理辅助进化（EVG/VG）"),
                        {QString::fromUtf8("多目标遗传算法"), QString::fromUtf8("差分进化")}),
        makeField(QString::fromUtf8("最大评价次数"), QStringLiteral("2500")),
        makeField(QString::fromUtf8("DOE 初始样本"), QStringLiteral("120")),
        makeField(QString::fromUtf8("最小改进幅度"), QStringLiteral("0.1"), QStringLiteral("%")),
    };
    st->addWidget(makeFieldGrid(fields, 2));
    lay->addWidget(stop);
    lay->addStretch();
    return content;
}

QWidget *OptimizationPage::buildRunPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    lay->addWidget(makeKpis({
        {QString::fromUtf8("已评价"), QStringLiteral("1860"), QString::fromUtf8("/ 2500")},
        {QString::fromUtf8("Pareto 成员"), QStringLiteral("37"), QString::fromUtf8("个")},
        {QString::fromUtf8("当前最优 σ"), QStringLiteral("0.975"), {}},
        {QString::fromUtf8("失败/无效"), QStringLiteral("112"), QString::fromUtf8("个")},
    }));

    auto *prog = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(prog->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("运行进度"), QString::fromUtf8("74%")));
    auto *bar = new QProgressBar;
    bar->setRange(0, 100);
    bar->setValue(74);
    bar->setTextVisible(false);
    pl->addWidget(bar);
    pl->addWidget(makeCanvas(
        QString::fromUtf8("收敛曲线（占位）\n横轴：评价次数　纵轴：最优目标值 / 超体积\n后续接入自定义绘制控件按迭代增量刷新"), 220));
    lay->addWidget(prog);

    auto *log = makePanel();
    auto *ll = qobject_cast<QVBoxLayout *>(log->layout());
    ll->addWidget(makePanelTitle(QString::fromUtf8("运行状态"), QString::fromUtf8("最近记录")));
    ll->addWidget(makeTable(
        {QString::fromUtf8("评价#"), QString::fromUtf8("σ"), QString::fromUtf8("可见度 V"), QStringLiteral("DC60"), QString::fromUtf8("状态")},
        {
            {QStringLiteral("5969"), QStringLiteral("0.962"), QStringLiteral("16.3%"), QStringLiteral("48.5%"), QString::fromUtf8("约束越界")},
            {QStringLiteral("5763"), QStringLiteral("0.971"), QStringLiteral("31.4%"), QStringLiteral("14.1%"), QString::fromUtf8("入 Pareto")},
            {QStringLiteral("5454"), QStringLiteral("0.975"), QStringLiteral("56.7%"), QStringLiteral("7.5%"), QString::fromUtf8("入 Pareto")},
            {QStringLiteral("5981"), QStringLiteral("—"), QStringLiteral("—"), QStringLiteral("—"), QString::fromUtf8("几何生成失败")},
        },
        TableOptions{{0, 3}, {}}));
    lay->addWidget(log);

    lay->addWidget(makeStatusText(
        QString::fromUtf8("优化进行中：无效外形与越界候选已标识并按策略处理，评价对象保持几何/气动一致。")));
    lay->addStretch();
    return content;
}
