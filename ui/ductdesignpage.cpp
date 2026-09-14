#include "ductdesignpage.h"

#include "uihelpers.h"
#include "ductdesignservice.h"
#include "ductdesignpresenter.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QStyle>
#include <QVBoxLayout>

namespace {
void clearLayout(QVBoxLayout *layout)
{
    if (!layout)
        return;
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *w = item->widget())
            w->deleteLater();
        delete item;
    }
}
}

DuctDesignPage::DuctDesignPage(QWidget *parent)
    : QWidget(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(22, 18, 22, 18);
    outer->setSpacing(14);

    outer->addWidget(makeHeading(
        QString::fromUtf8("参数化设计"),
        QString::fromUtf8("以统一设计参数描述 S 形进气道：入口、流道走向与截面变化。“设计参数”子页已接入真实数据流"
                          "（载入 design.json → 编辑 → 校验 → 导出参数快照）；三维生成(PicoGK)暂未接入。")));

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

    m_service = std::make_unique<DuctDesignService>();
    m_presenter = new DuctDesignPresenter(this, m_service.get(), this);
    connect(this, &DuctDesignPage::loadRequested, m_presenter, &DuctDesignPresenter::onLoadRequested);
    connect(this, &DuctDesignPage::validateRequested, m_presenter, &DuctDesignPresenter::onValidateRequested);
    connect(this, &DuctDesignPage::exportRequested, m_presenter, &DuctDesignPresenter::onExportRequested);

    tryAutoLoad();
}

DuctDesignPage::~DuctDesignPage() = default;

QWidget *DuctDesignPage::buildParamsPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *about = makePanel();
    auto *al = qobject_cast<QVBoxLayout *>(about->layout());
    al->addWidget(makePanelTitle(QString::fromUtf8("关于本页"), QString::fromUtf8("数据来源 / 处理 / 输出")));
    auto *aboutText = new QLabel(QString::fromUtf8(
        "数据来源：从算例 design.json（如 sample_dataset/cases/case_<id>/design.json）载入统一设计参数；也可手工编辑或新建。\n"
        "处理逻辑：字段绑定统一设计参数(DuctParams)，校验取值范围与 rib 的 z 轴对称；无 silent failure。\n"
        "输出：导出为 design.json 同构的参数快照 JSON，供几何生成/气动代理/优化共用（同定义、单位、param_version）。\n"
        "依据：需求 2.2（参数含义/单位/取值规则）、3.2（几何与气动同参数版本）、3.3（PicoGK 由统一参数生成外形，暂未接入）。"));
    aboutText->setObjectName(QStringLiteral("NoteLabel"));
    aboutText->setWordWrap(true);
    al->addWidget(aboutText);
    lay->addWidget(about);

    auto *ops = makePanel();
    auto *ol = qobject_cast<QVBoxLayout *>(ops->layout());
    ol->addWidget(makePanelTitle(QString::fromUtf8("参数来源与操作")));
    m_source = new QLabel(QString::fromUtf8("尚未载入设计参数。"));
    m_source->setObjectName(QStringLiteral("NoteLabel"));
    m_source->setWordWrap(true);
    ol->addWidget(m_source);
    m_shapeFamily = makeInput(QString());
    m_paramVersion = makeInput(QString());
    ol->addWidget(makeFieldGrid({
        makeLabeled(QString::fromUtf8("外形族"), m_shapeFamily),
        makeLabeled(QString::fromUtf8("参数版本"), m_paramVersion),
    }, 2));
    auto *btnRow = new QHBoxLayout;
    btnRow->setContentsMargins(0, 0, 0, 0);
    auto *loadBtn = makeButton(QString::fromUtf8("载入算例参数"), true);
    connect(loadBtn, &QPushButton::clicked, this, &DuctDesignPage::requestLoad);
    auto *valBtn = makeButton(QString::fromUtf8("校验"));
    connect(valBtn, &QPushButton::clicked, this, [this]() { emit validateRequested(); });
    auto *expBtn = makeButton(QString::fromUtf8("导出参数快照"));
    connect(expBtn, &QPushButton::clicked, this, &DuctDesignPage::requestExport);
    btnRow->addWidget(loadBtn);
    btnRow->addWidget(valBtn);
    btnRow->addWidget(expBtn);
    btnRow->addStretch();
    ol->addLayout(btnRow);
    lay->addWidget(ops);

    auto *cons = makePanel();
    auto *csl = qobject_cast<QVBoxLayout *>(cons->layout());
    csl->addWidget(makePanelTitle(QString::fromUtf8("参数约束"), QString::fromUtf8("校验依据")));
    m_constraintSource = new QLabel(QString::fromUtf8("约束来源：加载中…"));
    m_constraintSource->setObjectName(QStringLiteral("NoteLabel"));
    m_constraintSource->setWordWrap(true);
    csl->addWidget(m_constraintSource);
    auto *consWrap = new QWidget;
    m_constraintHost = new QVBoxLayout(consWrap);
    m_constraintHost->setContentsMargins(0, 0, 0, 0);
    csl->addWidget(consWrap);
    lay->addWidget(cons);

    auto *gen = makePanel();
    auto *gl = qobject_cast<QVBoxLayout *>(gen->layout());
    gl->addWidget(makePanelTitle(QString::fromUtf8("总体与固定项")));
    m_axialLength = makeInput(QString());
    m_inletArea = makeInput(QString());
    m_outletArea = makeInput(QString());
    gl->addWidget(makeFieldGrid({
        makeLabeled(QString::fromUtf8("轴向长度"), m_axialLength, QStringLiteral("m")),
        makeLabeled(QString::fromUtf8("入口面积"), m_inletArea, QString::fromUtf8("m²")),
        makeLabeled(QString::fromUtf8("出口(AIP)面积"), m_outletArea, QString::fromUtf8("m²")),
    }, 2));
    lay->addWidget(gen);

    auto *dir = makePanel();
    auto *dl = qobject_cast<QVBoxLayout *>(dir->layout());
    dl->addWidget(makePanelTitle(QString::fromUtf8("端部方向矢量"), QString::fromUtf8("自由变量")));
    m_inletPitch = makeInput(QString());
    m_inletExtend = makeInput(QString());
    m_outletPitch = makeInput(QString());
    m_outletExtend = makeInput(QString());
    dl->addWidget(makeFieldGrid({
        makeLabeled(QString::fromUtf8("入口俯仰角"), m_inletPitch, QStringLiteral("deg")),
        makeLabeled(QString::fromUtf8("入口延伸系数"), m_inletExtend),
        makeLabeled(QString::fromUtf8("出口俯仰角"), m_outletPitch, QStringLiteral("deg")),
        makeLabeled(QString::fromUtf8("出口延伸系数"), m_outletExtend),
    }, 2));
    lay->addWidget(dir);

    const QStringList ribIds = {QStringLiteral("rib1"), QStringLiteral("rib2"), QStringLiteral("rib3")};
    for (const QString &rid : ribIds) {
        RibEdits e;
        e.id = rid;
        e.spinePos = makeInput(QString());
        e.scale = makeInput(QString());
        e.zShift = makeInput(QString());
        e.cpY = makeInput(QString());
        e.cpZ = makeInput(QString());
        auto *rp = makePanel();
        auto *rl = qobject_cast<QVBoxLayout *>(rp->layout());
        rl->addWidget(makePanelTitle(QString::fromUtf8("截面 %1").arg(rid), QString::fromUtf8("自由变量")));
        rl->addWidget(makeFieldGrid({
            makeLabeled(QString::fromUtf8("相对位置"), e.spinePos),
            makeLabeled(QString::fromUtf8("缩放"), e.scale),
            makeLabeled(QString::fromUtf8("z 下沉"), e.zShift, QStringLiteral("m")),
            makeLabeled(QString::fromUtf8("y 控制点"), e.cpY, QStringLiteral("m")),
        }, 2));
        rl->addWidget(makeLabeled(QString::fromUtf8("z 控制点（5 个，逗号分隔，需 z 对称）"), e.cpZ));
        lay->addWidget(rp);
        m_ribEdits.append(e);
    }

    m_paramStatus = qobject_cast<QLabel *>(makeStatusText(QString::fromUtf8("尚未载入设计参数。"), false));
    m_paramStatus->setWordWrap(true);
    lay->addWidget(m_paramStatus);
    lay->addStretch();
    return content;
}

void DuctDesignPage::setParams(const DuctParams &params)
{
    m_params = params;

    m_shapeFamily->setText(params.shapeFamily);
    m_paramVersion->setText(params.paramVersion);
    m_axialLength->setText(QString::number(params.axialLength));
    m_inletArea->setText(QString::number(params.inletArea));
    m_outletArea->setText(QString::number(params.outletArea));
    m_inletPitch->setText(QString::number(params.inletPitchDeg));
    m_inletExtend->setText(QString::number(params.inletExtend));
    m_outletPitch->setText(QString::number(params.outletPitchDeg));
    m_outletExtend->setText(QString::number(params.outletExtend));

    for (int i = 0; i < m_ribEdits.size(); ++i) {
        RibEdits &e = m_ribEdits[i];
        if (i < params.ribs.size()) {
            const DuctRib &rib = params.ribs.at(i);
            if (!rib.id.isEmpty())
                e.id = rib.id;
            e.spinePos->setText(QString::number(rib.spinePos));
            e.scale->setText(QString::number(rib.scale));
            e.zShift->setText(QString::number(rib.zShift));
            e.cpY->setText(QString::number(rib.controlPointY));
            QStringList zs;
            for (double z : rib.controlPointsZ)
                zs << QString::number(z);
            e.cpZ->setText(zs.join(QStringLiteral(", ")));
        } else {
            e.spinePos->clear();
            e.scale->clear();
            e.zShift->clear();
            e.cpY->clear();
            e.cpZ->clear();
        }
    }

    if (m_source) {
        m_source->setText(params.caseId.isEmpty()
                              ? QString::fromUtf8("来源：手工/新建。")
                              : QString::fromUtf8("来源：算例 %1（外形族 %2，版本 %3）；CATIA 参考：%4")
                                    .arg(params.caseId, params.shapeFamily, params.paramVersion,
                                         params.catiaReference.isEmpty() ? QString::fromUtf8("无") : params.catiaReference));
    }
}

DuctParams DuctDesignPage::snapshotParams() const
{
    DuctParams p = m_params;  // 保留 caseId / 单位 / 锁定项 / CATIA 引用
    p.shapeFamily = m_shapeFamily->text().trimmed();
    p.paramVersion = m_paramVersion->text().trimmed();
    p.axialLength = m_axialLength->text().toDouble();
    p.inletArea = m_inletArea->text().toDouble();
    p.outletArea = m_outletArea->text().toDouble();
    p.inletPitchDeg = m_inletPitch->text().toDouble();
    p.inletExtend = m_inletExtend->text().toDouble();
    p.outletPitchDeg = m_outletPitch->text().toDouble();
    p.outletExtend = m_outletExtend->text().toDouble();

    QVector<DuctRib> ribs;
    for (int i = 0; i < m_ribEdits.size(); ++i) {
        const RibEdits &e = m_ribEdits.at(i);
        DuctRib rib;
        rib.id = e.id.isEmpty() ? QStringLiteral("rib%1").arg(i + 1) : e.id;
        rib.spinePos = e.spinePos->text().toDouble();
        rib.scale = e.scale->text().toDouble();
        rib.zShift = e.zShift->text().toDouble();
        rib.controlPointY = e.cpY->text().toDouble();
        const QStringList parts = e.cpZ->text().split(QLatin1Char(','), Qt::SkipEmptyParts);
        for (const QString &s : parts)
            rib.controlPointsZ << s.trimmed().toDouble();
        ribs << rib;
    }
    p.ribs = ribs;
    return p;
}

void DuctDesignPage::setValidation(const QStringList &issues)
{
    if (!m_paramStatus)
        return;
    if (issues.isEmpty()) {
        m_paramStatus->setObjectName(QStringLiteral("StatusGood"));
        m_paramStatus->setText(QString::fromUtf8("参数校验通过：取值范围与 rib 的 z 对称均满足。"));
    } else {
        m_paramStatus->setObjectName(QStringLiteral("StatusWarn"));
        m_paramStatus->setText(QString::fromUtf8("校验发现 %1 项问题：").arg(issues.size())
                               + issues.join(QString::fromUtf8("　")));
    }
    m_paramStatus->style()->unpolish(m_paramStatus);
    m_paramStatus->style()->polish(m_paramStatus);
}

void DuctDesignPage::setConstraints(const ParamConstraints &c)
{
    if (m_constraintSource)
        m_constraintSource->setText(QString::fromUtf8("约束来源：%1").arg(c.source));

    if (!m_constraintHost)
        return;
    clearLayout(m_constraintHost);

    auto range = [](const RangeD &r, const QString &unit) {
        return QString::fromUtf8("[%1, %2]%3").arg(r.lo).arg(r.hi)
               .arg(unit.isEmpty() ? QString() : QLatin1Char(' ') + unit);
    };
    QVector<QStringList> rows = {
        {QString::fromUtf8("入口俯仰角"), range(c.inletPitchDeg, QStringLiteral("°"))},
        {QString::fromUtf8("入口延伸系数"), range(c.inletExtend, QString())},
        {QString::fromUtf8("出口俯仰角"), range(c.outletPitchDeg, QStringLiteral("°"))},
        {QString::fromUtf8("出口延伸系数"), range(c.outletExtend, QString())},
        {QString::fromUtf8("rib 相对位置"), range(c.ribSpinePos, QString())},
        {QString::fromUtf8("rib 缩放"), range(c.ribScale, QString())},
        {QString::fromUtf8("rib z 下沉"), range(c.ribZShift, QStringLiteral("m"))},
        {QString::fromUtf8("rib y 控制点"), range(c.ribCpY, QStringLiteral("m"))},
        {QString::fromUtf8("rib 数"), QString::fromUtf8("= %1（论文规则）").arg(c.ribCount)},
        {QString::fromUtf8("每 rib z 控制点"),
         QString::fromUtf8("= %1；%2（容差 %3）")
             .arg(c.cpPerRib)
             .arg(c.enforceZSymmetry ? QString::fromUtf8("强制 z 对称") : QString::fromUtf8("不强制对称"))
             .arg(c.symmetryTol)},
    };
    m_constraintHost->addWidget(makeTable(
        {QString::fromUtf8("约束项"), QString::fromUtf8("取值范围 / 规则")}, rows, TableOptions{}));
}

void DuctDesignPage::showError(const QString &message)
{
    QMessageBox::warning(this, QString::fromUtf8("参数化设计"), message);
    setParamStatus(message, true);
}

void DuctDesignPage::setParamStatus(const QString &message, bool warn)
{
    if (!m_paramStatus)
        return;
    m_paramStatus->setObjectName(warn ? QStringLiteral("StatusWarn") : QStringLiteral("StatusGood"));
    m_paramStatus->setText(message);
    m_paramStatus->style()->unpolish(m_paramStatus);
    m_paramStatus->style()->polish(m_paramStatus);
}

void DuctDesignPage::showExportOk(const QString &path)
{
    QMessageBox::information(this, QString::fromUtf8("导出参数快照"),
                             QString::fromUtf8("已导出参数快照（design.json 同构）：\n%1\n\n可供几何生成/气动代理/优化共用。").arg(path));
}

void DuctDesignPage::requestLoad()
{
    const QString path = QFileDialog::getOpenFileName(
        this, QString::fromUtf8("选择设计参数文件 design.json"),
        QDir::currentPath(), QString::fromUtf8("设计参数 (*.json)"));
    if (path.isEmpty())
        return;
    emit loadRequested(path);
}

void DuctDesignPage::requestExport()
{
    const QString suggested = QDir(QDir::currentPath()).filePath(QStringLiteral("duct_params_snapshot.json"));
    const QString path = QFileDialog::getSaveFileName(
        this, QString::fromUtf8("导出参数快照"), suggested, QString::fromUtf8("设计参数 (*.json)"));
    if (path.isEmpty())
        return;
    emit exportRequested(path);
}

void DuctDesignPage::tryAutoLoad()
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        appDir + QStringLiteral("/../sample_dataset/cases/case_5763/design.json"),
        appDir + QStringLiteral("/sample_dataset/cases/case_5763/design.json"),
        appDir + QStringLiteral("/../../sample_dataset/cases/case_5763/design.json"),
        QDir::currentPath() + QStringLiteral("/sample_dataset/cases/case_5763/design.json"),
    };
    for (const QString &c : candidates) {
        if (QFile::exists(c)) {
            m_presenter->loadFrom(QDir::cleanPath(c), false);
            return;
        }
    }
    setParamStatus(QString::fromUtf8("未自动载入设计参数。点“载入算例参数”选择 design.json，或直接编辑后导出。"), false);
}

QWidget *DuctDesignPage::buildSpinePage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("中弧线控制点"), QString::fromUtf8("spine 曲线（示意）"),
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
    vl->addWidget(makePanelTitle(QString::fromUtf8("端部方向矢量"), QString::fromUtf8("弯管起止（示意）")));
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
    pl->addWidget(makePanelTitle(QString::fromUtf8("截面(rib)控制"), QString::fromUtf8("3 条自由截面（示意）"),
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
    al->addWidget(makePanelTitle(QString::fromUtf8("面积分布"), QString::fromUtf8("沿程（示意）")));
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
    pl->addWidget(makePanelTitle(QString::fromUtf8("三维外形预览"), QString::fromUtf8("PicoGK 生成（占位，暂未接入）")));
    pl->addWidget(makeCanvas(
        QString::fromUtf8("S 形进气道三维外形（占位）\n后续接入 PicoGK 生成结果 / 自定义三维视图\n展示、几何评价与气动评价共用同一外形"), 300));
    lay->addWidget(panel);

    lay->addWidget(makeStatusText(
        QString::fromUtf8("几何一致性：关键截面与接口位置需对照 CATIA 参考几何核对（PicoGK 接入后启用）。")));
    lay->addStretch();
    return content;
}
