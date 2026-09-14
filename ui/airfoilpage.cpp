#include "airfoilpage.h"

#include "uihelpers.h"

#include <QFrame>
#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>

AirfoilPage::AirfoilPage(QWidget *parent)
    : QWidget(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(22, 18, 22, 18);
    outer->setSpacing(14);

    outer->addWidget(makeHeading(
        QString::fromUtf8("翼型设计"),
        QString::fromUtf8("定义 NACA 翼型参数与几何，预览翼型剖面。原型页：编辑不写回模型。")));

    auto *tabs = new SubTabBar({
        {QStringLiteral("naca"), QString::fromUtf8("NACA 参数")},
        {QStringLiteral("geometry"), QString::fromUtf8("几何")},
        {QStringLiteral("preview"), QString::fromUtf8("预览")},
    }, QStringLiteral("naca"));
    outer->addWidget(tabs);

    m_inner = new QStackedWidget;
    m_inner->addWidget(wrapScroll(buildNacaPage()));
    m_inner->addWidget(wrapScroll(buildGeometryPage()));
    m_inner->addWidget(wrapScroll(buildPreviewPage()));
    outer->addWidget(m_inner, 1);

    connect(tabs, &SubTabBar::currentChanged, this, [this](const QString &id) {
        if (id == QLatin1String("naca"))
            m_inner->setCurrentIndex(0);
        else if (id == QLatin1String("geometry"))
            m_inner->setCurrentIndex(1);
        else
            m_inner->setCurrentIndex(2);
    });
}

QWidget *AirfoilPage::buildNacaPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("4 位 NACA 参数"), QString::fromUtf8("示例：2412")));
    QList<QWidget *> fields = {
        makeField(QString::fromUtf8("翼型编号"), QStringLiteral("2412")),
        makeField(QString::fromUtf8("最大弯度"), QStringLiteral("2"), QStringLiteral("%c")),
        makeField(QString::fromUtf8("最大弯度位置"), QStringLiteral("40"), QStringLiteral("%c")),
        makeField(QString::fromUtf8("最大厚度"), QStringLiteral("12"), QStringLiteral("%c")),
        makeField(QString::fromUtf8("弦长"), QStringLiteral("1.0"), QStringLiteral("m")),
        makeField(QString::fromUtf8("采样点数"), QStringLiteral("120")),
    };
    pl->addWidget(makeFieldGrid(fields, 2));
    lay->addWidget(panel);
    lay->addStretch();
    return content;
}

QWidget *AirfoilPage::buildGeometryPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("几何参数"), QString::fromUtf8("原型")));
    QList<QWidget *> fields = {
        makeField(QString::fromUtf8("前缘半径"), QStringLiteral("1.58"), QStringLiteral("%c")),
        makeField(QString::fromUtf8("后缘角"), QStringLiteral("14"), QStringLiteral("deg")),
        makeSelectField(QString::fromUtf8("后缘形式"), QString::fromUtf8("尖后缘"),
                        {QString::fromUtf8("钝后缘")}),
        makeSelectField(QString::fromUtf8("坐标分布"), QString::fromUtf8("余弦加密"),
                        {QString::fromUtf8("均匀")}),
    };
    pl->addWidget(makeFieldGrid(fields, 2));
    lay->addWidget(panel);
    lay->addStretch();
    return content;
}

QWidget *AirfoilPage::buildPreviewPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("翼型剖面预览"), QString::fromUtf8("示意")));

    auto *canvas = new QFrame;
    canvas->setObjectName(QStringLiteral("PreviewCanvas"));
    canvas->setMinimumHeight(240);
    auto *cl = new QVBoxLayout(canvas);
    auto *hint = new QLabel(QString::fromUtf8("NACA 2412 剖面（占位）\n后续接入自定义绘制控件按坐标增量绘制"));
    hint->setObjectName(QStringLiteral("NoteLabel"));
    hint->setAlignment(Qt::AlignCenter);
    cl->addWidget(hint);
    pl->addWidget(canvas);

    lay->addWidget(panel);
    lay->addStretch();
    return content;
}
