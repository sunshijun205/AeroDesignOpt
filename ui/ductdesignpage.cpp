#include "ductdesignpage.h"

#include "uihelpers.h"

#include <QStackedWidget>
#include <QVBoxLayout>

DuctDesignPage::DuctDesignPage(QWidget *parent)
    : QWidget(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(22, 18, 22, 18);
    outer->setSpacing(14);

    outer->addWidget(makeHeading(
        QString::fromUtf8("参数化设计"),
        QString::fromUtf8("以统一设计参数描述 S 形进气道：入口、流道走向（中弧线）与截面变化，由 PicoGK 生成完整三维外形。原型页：内容为静态示意。")));

    auto *tabs = new SubTabBar({
        {QStringLiteral("params"), QString::fromUtf8("设计参数")},
        {QStringLiteral("spine"), QString::fromUtf8("流道走向")},
        {QStringLiteral("section"), QString::fromUtf8("截面变化")},
        {QStringLiteral("preview"), QString::fromUtf8("外形预览")},
    }, QStringLiteral("params"));
    outer->addWidget(tabs);

    m_inner = new QStackedWidget;
    m_inner->addWidget(wrapScroll(buildParamsPage()));
    m_inner->addWidget(wrapScroll(buildSpinePage()));
    m_inner->addWidget(wrapScroll(buildSectionPage()));
    m_inner->addWidget(wrapScroll(buildPreviewPage()));
    outer->addWidget(m_inner, 1);

    connect(tabs, &SubTabBar::currentChanged, this, [this](const QString &id) {
        if (id == QLatin1String("params"))
            m_inner->setCurrentIndex(0);
        else if (id == QLatin1String("spine"))
            m_inner->setCurrentIndex(1);
        else if (id == QLatin1String("section"))
            m_inner->setCurrentIndex(2);
        else
            m_inner->setCurrentIndex(3);
    });
}

QWidget *DuctDesignPage::buildParamsPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *base = makePanel();
    auto *bl = qobject_cast<QVBoxLayout *>(base->layout());
    bl->addWidget(makePanelTitle(QString::fromUtf8("基准与总体"), QString::fromUtf8("S-01 进气道")));
    QList<QWidget *> baseFields = {
        makeSelectField(QString::fromUtf8("基准外形"), QString::fromUtf8("S-01（双 S 弯管）"),
                        {QString::fromUtf8("S-02"), QString::fromUtf8("自定义")}),
        makeField(QString::fromUtf8("进出口轴向长度"), QStringLiteral("1.85"), QStringLiteral("m")),
        makeField(QString::fromUtf8("入口面积"), QStringLiteral("0.42"), QString::fromUtf8("m²")),
        makeField(QString::fromUtf8("出口(AIP)面积"), QStringLiteral("0.58"), QString::fromUtf8("m²")),
    };
    bl->addWidget(makeFieldGrid(baseFields, 2));
    lay->addWidget(base);

    auto *space = makePanel();
    auto *sl = qobject_cast<QVBoxLayout *>(space->layout());
    sl->addWidget(makePanelTitle(QString::fromUtf8("安装空间包络"), QString::fromUtf8("硬约束")));
    QList<QWidget *> spaceFields = {
        makeField(QString::fromUtf8("最大高度"), QStringLiteral("0.62"), QStringLiteral("m")),
        makeField(QString::fromUtf8("最大宽度"), QStringLiteral("0.98"), QStringLiteral("m")),
        makeField(QString::fromUtf8("下沉上限"), QStringLiteral("0.30"), QStringLiteral("m")),
        makeSelectField(QString::fromUtf8("入口/出口接口"), QString::fromUtf8("固定（不可变）"),
                        {QString::fromUtf8("可微调")}),
    };
    sl->addWidget(makeFieldGrid(spaceFields, 2));
    lay->addWidget(space);

    auto *vars = makePanel();
    auto *vl = qobject_cast<QVBoxLayout *>(vars->layout());
    vl->addWidget(makePanelTitle(QString::fromUtf8("自由参数总览"), QString::fromUtf8("共 27 个")));
    vl->addWidget(makeTable(
        {QString::fromUtf8("参数组"), QString::fromUtf8("含义"), QString::fromUtf8("个数"), QString::fromUtf8("说明")},
        {
            {QString::fromUtf8("入口方向矢量"), QString::fromUtf8("入口处走向朝向"), QStringLiteral("2"), QString::fromUtf8("控制弯管起始角")},
            {QString::fromUtf8("出口方向矢量"), QString::fromUtf8("出口处走向朝向"), QStringLiteral("2"), QString::fromUtf8("控制 AIP 前弯度")},
            {QString::fromUtf8("中间 rib 控制点"), QString::fromUtf8("3 条截面各 5 点"), QStringLiteral("15"), QString::fromUtf8("z/y 方向可动")},
            {QString::fromUtf8("rib 缩放"), QString::fromUtf8("截面整体缩放"), QStringLiteral("3"), QString::fromUtf8("沿中弧线")},
            {QString::fromUtf8("rib 沿程平移"), QString::fromUtf8("截面位置"), QStringLiteral("3"), QString::fromUtf8("沿中弧线")},
            {QString::fromUtf8("rib z 向平移"), QString::fromUtf8("截面下沉"), QStringLiteral("2"), QString::fromUtf8("入口截面除外")},
        },
        TableOptions{}));
    lay->addWidget(vars);
    lay->addStretch();
    return content;
}

QWidget *DuctDesignPage::buildSpinePage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("中弧线控制点"), QString::fromUtf8("spine 曲线"),
                                 QString::fromUtf8("三维空间中插值的走向曲线，入口/出口方向矢量决定端部斜率，中间点决定 S 弯幅度。")));
    pl->addWidget(makeTable(
        {QString::fromUtf8("点"), QString::fromUtf8("相对弧长"), QStringLiteral("x (m)"), QStringLiteral("y (m)"), QStringLiteral("z (m)"), QString::fromUtf8("可变")},
        {
            {QStringLiteral("P0 入口"), QStringLiteral("0.00"), QStringLiteral("0.00"), QStringLiteral("0.00"), QStringLiteral("0.00"), QString::fromUtf8("否")},
            {QStringLiteral("P1"), QStringLiteral("0.25"), QStringLiteral("0.46"), QStringLiteral("0.00"), QStringLiteral("-0.12"), QString::fromUtf8("是")},
            {QStringLiteral("P2"), QStringLiteral("0.50"), QStringLiteral("0.92"), QStringLiteral("0.00"), QStringLiteral("-0.24"), QString::fromUtf8("是")},
            {QStringLiteral("P3"), QStringLiteral("0.75"), QStringLiteral("1.38"), QStringLiteral("0.00"), QStringLiteral("-0.08"), QString::fromUtf8("是")},
            {QStringLiteral("P4 出口"), QStringLiteral("1.00"), QStringLiteral("1.85"), QStringLiteral("0.00"), QStringLiteral("0.00"), QString::fromUtf8("否")},
        },
        TableOptions{}));
    lay->addWidget(panel);

    auto *vec = makePanel();
    auto *vl = qobject_cast<QVBoxLayout *>(vec->layout());
    vl->addWidget(makePanelTitle(QString::fromUtf8("端部方向矢量"), QString::fromUtf8("弯管起止")));
    QList<QWidget *> fields = {
        makeField(QString::fromUtf8("入口俯仰角"), QStringLiteral("-4.0"), QStringLiteral("deg")),
        makeField(QString::fromUtf8("入口延伸系数"), QStringLiteral("0.35")),
        makeField(QString::fromUtf8("出口俯仰角"), QStringLiteral("6.5"), QStringLiteral("deg")),
        makeField(QString::fromUtf8("出口延伸系数"), QStringLiteral("0.40")),
    };
    vl->addWidget(makeFieldGrid(fields, 2));
    vl->addWidget(makeCanvas(QString::fromUtf8("中弧线走向侧视图（占位）\n显示 x-z 平面 S 弯曲线与最高/最低点"), 200));
    lay->addWidget(vec);
    lay->addStretch();
    return content;
}

QWidget *DuctDesignPage::buildSectionPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("截面(rib)控制"), QString::fromUtf8("3 条自由截面"),
                                 QString::fromUtf8("每条 rib 由 5 个控制点定义，强制 z 轴对称；沿中弧线正交放置后蒙皮成型。")));
    pl->addWidget(makeTable(
        {QString::fromUtf8("截面"), QString::fromUtf8("相对位置"), QString::fromUtf8("面积比"), QString::fromUtf8("宽高比"), QString::fromUtf8("缩放"), QString::fromUtf8("z 平移")},
        {
            {QStringLiteral("Rib-1"), QStringLiteral("0.25"), QStringLiteral("1.18"), QStringLiteral("1.35"), QStringLiteral("0.96"), QStringLiteral("-0.06")},
            {QStringLiteral("Rib-2"), QStringLiteral("0.50"), QStringLiteral("1.05"), QStringLiteral("1.72"), QStringLiteral("0.90"), QStringLiteral("-0.14")},
            {QStringLiteral("Rib-3"), QStringLiteral("0.75"), QStringLiteral("1.28"), QStringLiteral("1.44"), QStringLiteral("1.02"), QStringLiteral("-0.05")},
        },
        TableOptions{}));
    lay->addWidget(panel);

    auto *area = makePanel();
    auto *al = qobject_cast<QVBoxLayout *>(area->layout());
    al->addWidget(makePanelTitle(QString::fromUtf8("面积分布"), QString::fromUtf8("沿程")));
    al->addWidget(makeCanvas(QString::fromUtf8("截面面积沿相对弧长分布曲线（占位）\n先扩张至局部最大，再局部收缩以降低直视可见度"), 200));
    lay->addWidget(area);
    lay->addStretch();
    return content;
}

QWidget *DuctDesignPage::buildPreviewPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    lay->addWidget(makeKpis({
        {QString::fromUtf8("流道长度"), QStringLiteral("1.85"), QStringLiteral("m")},
        {QString::fromUtf8("内部体积"), QStringLiteral("0.83"), QString::fromUtf8("m³")},
        {QString::fromUtf8("最大下沉"), QStringLiteral("0.24"), QStringLiteral("m")},
        {QString::fromUtf8("外形有效性"), QString::fromUtf8("有效"), {}},
    }));

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("三维外形预览"), QString::fromUtf8("PicoGK 生成（示意）")));
    pl->addWidget(makeCanvas(
        QString::fromUtf8("S 形进气道三维外形（占位）\n后续接入 PicoGK 生成结果 / 自定义三维视图\n展示、几何评价与气动评价共用同一外形"), 300));
    lay->addWidget(panel);

    lay->addWidget(makeStatusText(
        QString::fromUtf8("几何一致性：关键截面与接口位置已对照 CATIA 参考几何，偏差在容差内。")));
    lay->addStretch();
    return content;
}
