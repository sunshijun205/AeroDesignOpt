#include "visibilitypage.h"

#include "uihelpers.h"

#include <QStackedWidget>
#include <QVBoxLayout>

VisibilityPage::VisibilityPage(QWidget *parent)
    : QWidget(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(22, 18, 22, 18);
    outer->setSpacing(14);

    outer->addWidget(makeHeading(
        QString::fromUtf8("全遮挡评价"),
        QString::fromUtf8("在约定观察范围内判定进气口能否直视目标区域（AIP）。可见度 V = 可见点数 / 总点数。原型页：内容为静态示意。")));

    auto *tabs = new SubTabBar({
        {QStringLiteral("spec"), QString::fromUtf8("观察与目标")},
        {QStringLiteral("result"), QString::fromUtf8("遮挡判定")},
        {QStringLiteral("constraint"), QString::fromUtf8("几何约束")},
    }, QStringLiteral("spec"));
    outer->addWidget(tabs);

    m_inner = new QStackedWidget;
    m_inner->addWidget(wrapScroll(buildSpecPage()));
    m_inner->addWidget(wrapScroll(buildResultPage()));
    m_inner->addWidget(wrapScroll(buildConstraintPage()));
    outer->addWidget(m_inner, 1);

    connect(tabs, &SubTabBar::currentChanged, this, [this](const QString &id) {
        if (id == QLatin1String("spec"))
            m_inner->setCurrentIndex(0);
        else if (id == QLatin1String("result"))
            m_inner->setCurrentIndex(1);
        else
            m_inner->setCurrentIndex(2);
    });
}

QWidget *VisibilityPage::buildSpecPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *obs = makePanel();
    auto *ol = qobject_cast<QVBoxLayout *>(obs->layout());
    ol->addWidget(makePanelTitle(QString::fromUtf8("观察范围"), QString::fromUtf8("外部视线"),
                                 QString::fromUtf8("必须同时明确观察范围与目标范围；单一正视方向看不到不代表侧向也不可见。")));
    QList<QWidget *> obsFields = {
        makeField(QString::fromUtf8("俯仰角范围"), QStringLiteral("-25 ~ +5"), QStringLiteral("deg")),
        makeField(QString::fromUtf8("偏航角范围"), QStringLiteral("-20 ~ +20"), QStringLiteral("deg")),
        makeField(QString::fromUtf8("角度步长"), QStringLiteral("0.5"), QStringLiteral("deg")),
        makeSelectField(QString::fromUtf8("观察面"), QString::fromUtf8("入口面板"),
                        {QString::fromUtf8("自定义观察面")}),
    };
    ol->addWidget(makeFieldGrid(obsFields, 2));
    lay->addWidget(obs);

    auto *tgt = makePanel();
    auto *tl = qobject_cast<QVBoxLayout *>(tgt->layout());
    tl->addWidget(makePanelTitle(QString::fromUtf8("目标区域"), QString::fromUtf8("需客户确认")));
    QList<QWidget *> tgtFields = {
        makeSelectField(QString::fromUtf8("目标截面"), QString::fromUtf8("AIP（压气机前端面）"),
                        {QString::fromUtf8("自定义截面")}),
        makeField(QString::fromUtf8("采样点数"), QStringLiteral("2000")),
        makeField(QString::fromUtf8("几何分辨率"), QStringLiteral("1.0"), QStringLiteral("mm")),
        makeField(QString::fromUtf8("判定容差"), QStringLiteral("0.5"), QStringLiteral("mm")),
    };
    tl->addWidget(makeFieldGrid(tgtFields, 2));
    lay->addWidget(tgt);

    lay->addWidget(makeStatusText(
        QString::fromUtf8("说明：全遮挡属几何约束，不直接等同电磁性能结论；需记录分辨率与容差以支持临界外形核查。")));
    lay->addStretch();
    return content;
}

QWidget *VisibilityPage::buildResultPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    lay->addWidget(makeKpis({
        {QString::fromUtf8("可见度 V"), QStringLiteral("31.4"), QStringLiteral("%")},
        {QString::fromUtf8("正视遮挡"), QString::fromUtf8("满足"), {}},
        {QString::fromUtf8("全遮挡"), QString::fromUtf8("未满足"), {}},
        {QString::fromUtf8("可见倾角范围 Δγ"), QStringLiteral("18.1"), QStringLiteral("deg")},
    }));

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("直视遮挡剖面"), QString::fromUtf8("示意")));
    pl->addWidget(makeCanvas(
        QString::fromUtf8("直视遮挡剖面图（占位）\n绘制入口点到目标点的代表性视线，标出被壁面阻断/未阻断段\n可见点区域高亮"), 260));
    lay->addWidget(panel);

    auto *tbl = makePanel();
    auto *tl = qobject_cast<QVBoxLayout *>(tbl->layout());
    tl->addWidget(makePanelTitle(QString::fromUtf8("未遮挡方向"), QString::fromUtf8("按可见占比")));
    tl->addWidget(makeTable(
        {QString::fromUtf8("俯仰角"), QString::fromUtf8("偏航角"), QString::fromUtf8("可见点数"), QString::fromUtf8("可见占比"), QString::fromUtf8("结论")},
        {
            {QStringLiteral("-2.8°"), QStringLiteral("0.0°"), QStringLiteral("18"), QStringLiteral("0.9%"), QString::fromUtf8("临界可见")},
            {QStringLiteral("-8.0°"), QStringLiteral("0.0°"), QStringLiteral("142"), QStringLiteral("7.1%"), QString::fromUtf8("可见")},
            {QStringLiteral("-14.0°"), QStringLiteral("+6.0°"), QStringLiteral("206"), QStringLiteral("10.3%"), QString::fromUtf8("可见")},
            {QStringLiteral("-20.0°"), QStringLiteral("-12.0°"), QStringLiteral("88"), QStringLiteral("4.4%"), QString::fromUtf8("可见")},
        },
        TableOptions{{0}, {}}));
    lay->addWidget(tbl);

    lay->addWidget(makeStatusText(
        QString::fromUtf8("结论：正视方向已遮挡 AIP，但在 -2.8° 起出现临界可见，受安装空间限制无法在全部倾角下遮挡。"), true));
    lay->addStretch();
    return content;
}

QWidget *VisibilityPage::buildConstraintPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("几何约束检查"), QString::fromUtf8("硬约束"),
                                 QString::fromUtf8("接口、安装空间与外形有效性等约束的满足情况；违反硬约束的候选将被优化器标识处理。")));
    pl->addWidget(makeTable(
        {QString::fromUtf8("约束项"), QString::fromUtf8("要求"), QString::fromUtf8("当前值"), QString::fromUtf8("状态")},
        {
            {QString::fromUtf8("入口接口位置"), QString::fromUtf8("固定"), QString::fromUtf8("对齐"), QString::fromUtf8("满足")},
            {QString::fromUtf8("出口接口位置"), QString::fromUtf8("固定"), QString::fromUtf8("对齐"), QString::fromUtf8("满足")},
            {QString::fromUtf8("安装空间高度"), QStringLiteral("≤ 0.62 m"), QStringLiteral("0.58 m"), QString::fromUtf8("满足")},
            {QString::fromUtf8("最大下沉"), QStringLiteral("≤ 0.30 m"), QStringLiteral("0.24 m"), QString::fromUtf8("满足")},
            {QString::fromUtf8("外形自交检测"), QString::fromUtf8("无自交"), QString::fromUtf8("无"), QString::fromUtf8("满足")},
        },
        TableOptions{}));
    lay->addWidget(panel);
    lay->addWidget(makeStatusText(QString::fromUtf8("全部几何硬约束满足，外形有效，可进入气动评价。")));
    lay->addStretch();
    return content;
}
