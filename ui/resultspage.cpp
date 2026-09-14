#include "resultspage.h"

#include "uihelpers.h"

#include <QFrame>
#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>

ResultsPage::ResultsPage(QWidget *parent)
    : QWidget(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(22, 18, 22, 18);
    outer->setSpacing(14);

    outer->addWidget(makeHeading(
        QString::fromUtf8("结果"),
        QString::fromUtf8("查看极曲线、收敛历史与方案对比。原型页：图表为示意占位。")));

    auto *tabs = new SubTabBar({
        {QStringLiteral("polar"), QString::fromUtf8("极曲线")},
        {QStringLiteral("convergence"), QString::fromUtf8("收敛历史")},
        {QStringLiteral("compare"), QString::fromUtf8("方案对比")},
    }, QStringLiteral("polar"));
    outer->addWidget(tabs);

    m_inner = new QStackedWidget;
    m_inner->addWidget(wrapScroll(buildPolarPage()));
    m_inner->addWidget(wrapScroll(buildConvergencePage()));
    m_inner->addWidget(wrapScroll(buildComparePage()));
    outer->addWidget(m_inner, 1);

    connect(tabs, &SubTabBar::currentChanged, this, [this](const QString &id) {
        if (id == QLatin1String("polar"))
            m_inner->setCurrentIndex(0);
        else if (id == QLatin1String("convergence"))
            m_inner->setCurrentIndex(1);
        else
            m_inner->setCurrentIndex(2);
    });
}

QWidget *ResultsPage::buildPolarPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("阻力极曲线 Cl-Cd"), QString::fromUtf8("示意")));
    auto *canvas = new QFrame;
    canvas->setObjectName(QStringLiteral("PreviewCanvas"));
    canvas->setMinimumHeight(260);
    auto *cl = new QVBoxLayout(canvas);
    auto *hint = new QLabel(QString::fromUtf8("Cl-Cd 极曲线（占位）"));
    hint->setObjectName(QStringLiteral("NoteLabel"));
    hint->setAlignment(Qt::AlignCenter);
    cl->addWidget(hint);
    pl->addWidget(canvas);
    lay->addWidget(panel);
    lay->addStretch();
    return content;
}

QWidget *ResultsPage::buildConvergencePage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("优化收敛历史"), QString::fromUtf8("示意")));
    auto *canvas = new QFrame;
    canvas->setObjectName(QStringLiteral("PreviewCanvas"));
    canvas->setMinimumHeight(260);
    auto *cl = new QVBoxLayout(canvas);
    auto *hint = new QLabel(QString::fromUtf8("目标值随代数变化（占位）"));
    hint->setObjectName(QStringLiteral("NoteLabel"));
    hint->setAlignment(Qt::AlignCenter);
    cl->addWidget(hint);
    pl->addWidget(canvas);
    lay->addWidget(panel);
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
    pl->addWidget(makePanelTitle(QString::fromUtf8("方案对比"), QString::fromUtf8("示意数据")));
    const QStringList headers = {
        QString::fromUtf8("方案"), QStringLiteral("Cl"), QStringLiteral("Cd"),
        QString::fromUtf8("L/D"), QStringLiteral("Cm")
    };
    const QVector<QStringList> rows = {
        {QString::fromUtf8("基线 NACA2412"), QStringLiteral("0.62"), QStringLiteral("0.0121"), QStringLiteral("51.2"), QStringLiteral("-0.07")},
        {QString::fromUtf8("优化 v1"), QStringLiteral("0.60"), QStringLiteral("0.0104"), QStringLiteral("57.7"), QStringLiteral("-0.06")},
        {QString::fromUtf8("优化 v2"), QStringLiteral("0.58"), QStringLiteral("0.0098"), QStringLiteral("59.2"), QStringLiteral("-0.05")},
    };
    pl->addWidget(makeTable(headers, rows));
    lay->addWidget(panel);
    lay->addStretch();
    return content;
}
