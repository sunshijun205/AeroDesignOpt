#include "optimizationpage.h"

#include "uihelpers.h"

#include <QLabel>
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
        QString::fromUtf8("配置优化目标、设计变量与算法。原型页：优化后端待接入。")));

    auto *tabs = new SubTabBar({
        {QStringLiteral("objective"), QString::fromUtf8("目标与约束")},
        {QStringLiteral("variables"), QString::fromUtf8("设计变量")},
        {QStringLiteral("algorithm"), QString::fromUtf8("算法")},
    }, QStringLiteral("objective"));
    outer->addWidget(tabs);

    m_inner = new QStackedWidget;
    m_inner->addWidget(wrapScroll(buildObjectivePage()));
    m_inner->addWidget(wrapScroll(buildVariablesPage()));
    m_inner->addWidget(wrapScroll(buildAlgorithmPage()));
    outer->addWidget(m_inner, 1);

    connect(tabs, &SubTabBar::currentChanged, this, [this](const QString &id) {
        if (id == QLatin1String("objective"))
            m_inner->setCurrentIndex(0);
        else if (id == QLatin1String("variables"))
            m_inner->setCurrentIndex(1);
        else
            m_inner->setCurrentIndex(2);
    });
}

QWidget *OptimizationPage::buildObjectivePage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("优化目标与约束"), QString::fromUtf8("原型")));
    QList<QWidget *> fields = {
        makeSelectField(QString::fromUtf8("目标"), QString::fromUtf8("最大化 L/D"),
                        {QString::fromUtf8("最小化 Cd"), QString::fromUtf8("最大化 Cl")}),
        makeField(QString::fromUtf8("设计升力系数 Cl"), QStringLiteral("0.5")),
        makeField(QString::fromUtf8("约束：最小厚度"), QStringLiteral("10"), QStringLiteral("%c")),
        makeField(QString::fromUtf8("约束：最大 Cm"), QStringLiteral("-0.08")),
    };
    pl->addWidget(makeFieldGrid(fields, 2));
    lay->addWidget(panel);
    lay->addStretch();
    return content;
}

QWidget *OptimizationPage::buildVariablesPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("设计变量"), QString::fromUtf8("原型")));
    const QStringList headers = {
        QString::fromUtf8("变量"), QString::fromUtf8("下限"),
        QString::fromUtf8("上限"), QString::fromUtf8("初值")
    };
    const QVector<QStringList> rows = {
        {QString::fromUtf8("最大弯度 %c"), QStringLiteral("0"), QStringLiteral("6"), QStringLiteral("2")},
        {QString::fromUtf8("弯度位置 %c"), QStringLiteral("20"), QStringLiteral("60"), QStringLiteral("40")},
        {QString::fromUtf8("最大厚度 %c"), QStringLiteral("8"), QStringLiteral("18"), QStringLiteral("12")},
        {QString::fromUtf8("迎角 deg"), QStringLiteral("-2"), QStringLiteral("10"), QStringLiteral("4")},
    };
    pl->addWidget(makeTable(headers, rows));
    lay->addWidget(panel);
    lay->addStretch();
    return content;
}

QWidget *OptimizationPage::buildAlgorithmPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("优化算法"), QString::fromUtf8("待接入")));
    QList<QWidget *> fields = {
        makeSelectField(QString::fromUtf8("算法"), QString::fromUtf8("遗传算法 (GA)"),
                        {QString::fromUtf8("梯度法 (SLSQP)"), QString::fromUtf8("粒子群 (PSO)")}),
        makeField(QString::fromUtf8("种群规模"), QStringLiteral("40")),
        makeField(QString::fromUtf8("最大代数"), QStringLiteral("60")),
        makeField(QString::fromUtf8("收敛容差"), QStringLiteral("1e-4")),
    };
    pl->addWidget(makeFieldGrid(fields, 2));

    auto *note = new QLabel(QString::fromUtf8(
        "优化后端尚未接入，此处为参数占位。接入时按分层落地到 service/ 与 controller/。"));
    note->setObjectName(QStringLiteral("NoteLabel"));
    note->setWordWrap(true);
    pl->addWidget(note);

    lay->addWidget(panel);
    lay->addStretch();
    return content;
}
