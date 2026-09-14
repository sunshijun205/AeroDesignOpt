#include "datapage.h"

#include "uihelpers.h"

#include <QHBoxLayout>
#include <QStackedWidget>
#include <QVBoxLayout>

DataPage::DataPage(QWidget *parent)
    : QWidget(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(22, 18, 22, 18);
    outer->setSpacing(14);

    outer->addWidget(makeHeading(
        QString::fromUtf8("数据管理"),
        QString::fromUtf8("导入并审查 CFD 算例，建立几何/工况/性能的对应关系，评估联合设计空间的覆盖度。原型页：内容为静态示意。")));

    auto *tabs = new SubTabBar({
        {QStringLiteral("source"), QString::fromUtf8("数据源")},
        {QStringLiteral("consistency"), QString::fromUtf8("一致性审查")},
        {QStringLiteral("coverage"), QString::fromUtf8("覆盖度")},
    }, QStringLiteral("source"));
    outer->addWidget(tabs);

    m_inner = new QStackedWidget;
    m_inner->addWidget(wrapScroll(buildSourcePage()));
    m_inner->addWidget(wrapScroll(buildConsistencyPage()));
    m_inner->addWidget(wrapScroll(buildCoveragePage()));
    outer->addWidget(m_inner, 1);

    connect(tabs, &SubTabBar::currentChanged, this, [this](const QString &id) {
        if (id == QLatin1String("source"))
            m_inner->setCurrentIndex(0);
        else if (id == QLatin1String("consistency"))
            m_inner->setCurrentIndex(1);
        else
            m_inner->setCurrentIndex(2);
    });
}

QWidget *DataPage::buildSourcePage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    lay->addWidget(makeKpis({
        {QString::fromUtf8("可用算例"), QStringLiteral("312"), QString::fromUtf8("个")},
        {QString::fromUtf8("外形族"), QStringLiteral("1"), QString::fromUtf8("族")},
        {QString::fromUtf8("工况点"), QStringLiteral("18"), QString::fromUtf8("个")},
        {QString::fromUtf8("保留流场"), QStringLiteral("124"), QString::fromUtf8("个")},
    }));

    auto *cfg = makePanel();
    auto *cl = qobject_cast<QVBoxLayout *>(cfg->layout());
    cl->addWidget(makePanelTitle(QString::fromUtf8("导入配置"), QString::fromUtf8("数据源")));
    QList<QWidget *> fields = {
        makeSelectField(QString::fromUtf8("数据来源"), QString::fromUtf8("客户 CFD 数据库"),
                        {QString::fromUtf8("公开/自有数据")}),
        makeField(QString::fromUtf8("根目录"), QStringLiteral("D:/duct_cfd/cases")),
        makeSelectField(QString::fromUtf8("几何来源"), QString::fromUtf8("CATIA 参数记录"),
                        {QString::fromUtf8("PicoGK 生成参数")}),
        makeSelectField(QString::fromUtf8("字段映射模板"), QString::fromUtf8("默认模板 v1"),
                        {QString::fromUtf8("自定义")}),
    };
    cl->addWidget(makeFieldGrid(fields, 2));
    lay->addWidget(cfg);

    auto *table = makePanel();
    auto *tl = qobject_cast<QVBoxLayout *>(table->layout());
    tl->addWidget(makePanelTitle(QString::fromUtf8("已导入数据集"), QString::fromUtf8("共 4 批")));
    tl->addWidget(makeTable(
        {QString::fromUtf8("批次"), QString::fromUtf8("外形数"), QString::fromUtf8("工况数"),
         QString::fromUtf8("算例数"), QString::fromUtf8("含流场"), QString::fromUtf8("状态")},
        {
            {QStringLiteral("batch-A"), QStringLiteral("42"), QStringLiteral("6"), QStringLiteral("164"), QString::fromUtf8("是"), QString::fromUtf8("已校核")},
            {QStringLiteral("batch-B"), QStringLiteral("28"), QStringLiteral("4"), QStringLiteral("96"), QString::fromUtf8("否"), QString::fromUtf8("已校核")},
            {QStringLiteral("batch-C"), QStringLiteral("12"), QStringLiteral("3"), QStringLiteral("36"), QString::fromUtf8("是"), QString::fromUtf8("待核对")},
            {QStringLiteral("batch-D"), QStringLiteral("6"), QStringLiteral("2"), QStringLiteral("16"), QString::fromUtf8("否"), QString::fromUtf8("口径存疑")},
        },
        TableOptions{{3}, {}}));
    lay->addWidget(table);

    lay->addWidget(makeStatusText(
        QString::fromUtf8("提示：batch-D 的边界条件定义与其余批次不一致，需在“一致性审查”中确认后再用于训练。"), true));
    lay->addStretch();
    return content;
}

QWidget *DataPage::buildConsistencyPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("计算口径一致性核查"),
                                 QString::fromUtf8("跨批次"),
                                 QString::fromUtf8("检查物理定义、计算设置与性能指标处理方式是否一致，避免引入与外形/工况无关的性能差异。")));
    pl->addWidget(makeTable(
        {QString::fromUtf8("核查项"), QString::fromUtf8("A"), QString::fromUtf8("B"), QString::fromUtf8("C"), QString::fromUtf8("D"), QString::fromUtf8("结论")},
        {
            {QString::fromUtf8("参考总压位置"), QString::fromUtf8("入口"), QString::fromUtf8("入口"), QString::fromUtf8("入口"), QString::fromUtf8("入口"), QString::fromUtf8("一致")},
            {QString::fromUtf8("出口平均方式"), QString::fromUtf8("质量平均"), QString::fromUtf8("质量平均"), QString::fromUtf8("质量平均"), QString::fromUtf8("面积平均"), QString::fromUtf8("D 不一致")},
            {QString::fromUtf8("湍流模型"), QStringLiteral("k-omega"), QStringLiteral("k-omega"), QStringLiteral("k-omega"), QStringLiteral("SST"), QString::fromUtf8("D 不一致")},
            {QString::fromUtf8("出口边界"), QString::fromUtf8("定质量流量"), QString::fromUtf8("定质量流量"), QString::fromUtf8("定质量流量"), QString::fromUtf8("定背压"), QString::fromUtf8("D 存疑")},
            {QString::fromUtf8("收敛记录"), QString::fromUtf8("完整"), QString::fromUtf8("完整"), QString::fromUtf8("缺失"), QString::fromUtf8("完整"), QString::fromUtf8("C 待补")},
        },
        TableOptions{{1, 2, 3, 4}, {}}));
    lay->addWidget(panel);

    lay->addWidget(makeStatusText(
        QString::fromUtf8("建议：batch-D 采用不同平均方式与湍流模型，指标不可直接与 A/B/C 混用，需重算或单独标注。"), true));
    lay->addStretch();
    return content;
}

QWidget *DataPage::buildCoveragePage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    lay->addWidget(makeKpis({
        {QString::fromUtf8("参数维度"), QStringLiteral("27"), QString::fromUtf8("维")},
        {QString::fromUtf8("联合空间覆盖"), QStringLiteral("63"), QStringLiteral("%")},
        {QString::fromUtf8("空缺组合"), QStringLiteral("21"), QString::fromUtf8("处")},
        {QString::fromUtf8("边界样本"), QString::fromUtf8("偏少"), {}},
    }));

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("设计参数 × 工况 联合覆盖矩阵"),
                                 QString::fromUtf8("示意"),
                                 QString::fromUtf8("各参数单独看均覆盖目标范围，但部分外形只在少量工况下计算，联合空间仍存在空缺。")));
    pl->addWidget(makeCanvas(
        QString::fromUtf8("联合空间覆盖热力图（占位）\n横轴：设计参数分组　纵轴：工况点　颜色深浅表示样本密度\n后续接入自定义绘制控件按矩阵渲染"), 240));
    lay->addWidget(panel);

    auto *gap = makePanel();
    auto *gl = qobject_cast<QVBoxLayout *>(gap->layout());
    gl->addWidget(makePanelTitle(QString::fromUtf8("加密建议"), QString::fromUtf8("按优化关注度排序")));
    gl->addWidget(makeTable(
        {QString::fromUtf8("区域"), QString::fromUtf8("特征"), QString::fromUtf8("现有样本"), QString::fromUtf8("建议补充"), QString::fromUtf8("优先级")},
        {
            {QString::fromUtf8("大弯度 × 高马赫"), QString::fromUtf8("响应非线性强"), QStringLiteral("3"), QStringLiteral("18"), QString::fromUtf8("高")},
            {QString::fromUtf8("小截面 × 大攻角"), QString::fromUtf8("可能流动分离"), QStringLiteral("5"), QStringLiteral("12"), QString::fromUtf8("高")},
            {QString::fromUtf8("设计空间边界"), QString::fromUtf8("外推风险"), QStringLiteral("2"), QStringLiteral("10"), QString::fromUtf8("中")},
        },
        TableOptions{{0, 1}, {}}));
    lay->addWidget(gap);
    lay->addStretch();
    return content;
}
