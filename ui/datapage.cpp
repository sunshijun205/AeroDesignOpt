#include "datapage.h"

#include "uihelpers.h"
#include "datasetservice.h"
#include "datapresenter.h"
#include "ductdataset.h"

#include <QCoreApplication>
#include <QDir>
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

DataPage::DataPage(QWidget *parent)
    : QWidget(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(22, 18, 22, 18);
    outer->setSpacing(14);

    outer->addWidget(makeHeading(
        QString::fromUtf8("数据管理"),
        QString::fromUtf8("导入并审查 CFD 算例，建立几何/工况/性能的对应关系，评估联合设计空间的覆盖度。“数据源”子页已接入真实导入。")));

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

    m_service = std::make_unique<DatasetService>();
    m_presenter = new DataPresenter(this, m_service.get(), this);
    connect(this, &DataPage::importRequested,
            m_presenter, &DataPresenter::onImportRequested);
    connect(this, &DataPage::exportRequested,
            m_presenter, &DataPresenter::onExportRequested);

    tryAutoLoad();
}

DataPage::~DataPage() = default;

QWidget *DataPage::buildSourcePage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *about = makePanel();
    auto *al = qobject_cast<QVBoxLayout *>(about->layout());
    al->addWidget(makePanelTitle(QString::fromUtf8("关于本页"), QString::fromUtf8("数据来源与处理逻辑")));
    auto *aboutText = new QLabel(QString::fromUtf8(
        "数据来源：选择（或启动时自动加载）数据集根目录后，读取根目录下的 dataset.json（数据集清单）"
        "与 cases_index.csv（算例索引，一行一个算例）。\n"
        "顶部指标：可用算例=索引行数；外形族=不同 shape_family 数；工况点=不同(马赫,攻角,质量流量)组合数；"
        "保留流场=has_field 为 true 的算例数——均由 cases_index.csv 实时统计，非写死。\n"
        "“已导入算例”表：逐行来自 cases_index.csv（算例号、批次、马赫、攻角、质量流量、可见度V、σ、含流场、状态）；"
        "状态非 ok 的行标黄。\n"
        "输出：点“导出报告与训练清单”生成 dataset_review.json（一致性/覆盖度报告）与 "
        "training_manifest.json（审定后的可用算例 + 训练/验证标记），供代理训练模块直接消费。\n"
        "依据：需求 2.1 / 4.1（数据整理与对应关系）、3.5（形成交付数据集）、4.5（独立验证划分）。"));
    aboutText->setObjectName(QStringLiteral("NoteLabel"));
    aboutText->setWordWrap(true);
    al->addWidget(aboutText);
    lay->addWidget(about);

    auto *kpiWrap = new QWidget;
    m_kpiHost = new QVBoxLayout(kpiWrap);
    m_kpiHost->setContentsMargins(0, 0, 0, 0);
    m_kpiHost->addWidget(makeKpis({
        {QString::fromUtf8("可用算例"), QStringLiteral("—"), {}},
        {QString::fromUtf8("外形族"), QStringLiteral("—"), {}},
        {QString::fromUtf8("工况点"), QStringLiteral("—"), {}},
        {QString::fromUtf8("保留流场"), QStringLiteral("—"), {}},
    }));
    lay->addWidget(kpiWrap);

    auto *cfg = makePanel();
    auto *cl = qobject_cast<QVBoxLayout *>(cfg->layout());
    cl->addWidget(makePanelTitle(QString::fromUtf8("导入配置"), QString::fromUtf8("数据源")));
    m_rootDir = makeInput(QString());
    QList<QWidget *> fields = {
        makeSelectField(QString::fromUtf8("数据来源"), QString::fromUtf8("客户 CFD 数据库"),
                        {QString::fromUtf8("公开/自有数据")}),
        makeLabeled(QString::fromUtf8("数据集根目录"), m_rootDir),
        makeSelectField(QString::fromUtf8("几何来源"), QString::fromUtf8("CATIA 参数记录"),
                        {QString::fromUtf8("PicoGK 生成参数")}),
        makeSelectField(QString::fromUtf8("字段映射模板"), QString::fromUtf8("默认模板 v1"),
                        {QString::fromUtf8("自定义")}),
    };
    cl->addWidget(makeFieldGrid(fields, 2));
    auto *btnRow = new QHBoxLayout;
    btnRow->setContentsMargins(0, 0, 0, 0);
    auto *importBtn = makeButton(QString::fromUtf8("选择目录并导入"), true);
    connect(importBtn, &QPushButton::clicked, this, &DataPage::requestImport);
    btnRow->addWidget(importBtn);
    auto *exportBtn = makeButton(QString::fromUtf8("导出报告与训练清单"));
    connect(exportBtn, &QPushButton::clicked, this, &DataPage::requestExport);
    btnRow->addWidget(exportBtn);
    btnRow->addStretch();
    cl->addLayout(btnRow);
    lay->addWidget(cfg);

    auto *tablePanel = makePanel();
    auto *tpl = qobject_cast<QVBoxLayout *>(tablePanel->layout());
    tpl->addWidget(makePanelTitle(QString::fromUtf8("已导入算例"), QString::fromUtf8("来自 cases_index.csv")));
    auto *tableWrap = new QWidget;
    m_tableHost = new QVBoxLayout(tableWrap);
    m_tableHost->setContentsMargins(0, 0, 0, 0);
    tpl->addWidget(tableWrap);
    lay->addWidget(tablePanel);

    m_sourceStatus = qobject_cast<QLabel *>(
        makeStatusText(QString::fromUtf8("尚未导入数据。"), false));
    lay->addWidget(m_sourceStatus);
    lay->addStretch();
    return content;
}

void DataPage::setSummary(const DatasetSummary &summary)
{
    m_summary = summary;
    m_hasData = true;

    if (m_kpiHost) {
        clearLayout(m_kpiHost);
        m_kpiHost->addWidget(makeKpis({
            {QString::fromUtf8("可用算例"), QString::number(summary.totalCases), QString::fromUtf8("个")},
            {QString::fromUtf8("外形族"), QString::number(summary.shapeFamilies.size()), QString::fromUtf8("族")},
            {QString::fromUtf8("工况点"), QString::number(summary.conditionPoints), QString::fromUtf8("个")},
            {QString::fromUtf8("保留流场"), QString::number(summary.fieldCases), QString::fromUtf8("个")},
        }));
    }

    if (m_tableHost) {
        clearLayout(m_tableHost);
        QVector<QStringList> rows;
        QVector<int> warnRows;
        for (int i = 0; i < summary.records.size(); ++i) {
            const DuctCaseRecord &r = summary.records.at(i);
            rows.append({
                r.caseId,
                r.batch,
                QString::number(r.mach, 'f', 2),
                QString::number(r.aoaDeg, 'f', 1) + QString::fromUtf8("°"),
                QString::number(r.massFlow, 'f', 1),
                QString::number(r.visibility * 100.0, 'f', 2) + QLatin1Char('%'),
                QString::number(r.sigma, 'f', 4),
                r.hasField ? QString::fromUtf8("是") : QString::fromUtf8("否"),
                r.status,
            });
            if (r.status != QLatin1String("ok"))
                warnRows.append(i);
        }
        m_tableHost->addWidget(makeTable(
            {QString::fromUtf8("算例"), QString::fromUtf8("批次"), QString::fromUtf8("马赫"),
             QString::fromUtf8("攻角"), QString::fromUtf8("质量流量"), QString::fromUtf8("可见度V"),
             QStringLiteral("σ"), QString::fromUtf8("含流场"), QString::fromUtf8("状态")},
            rows, TableOptions{warnRows, {}}));
    }

    if (m_consistencyHost) {
        clearLayout(m_consistencyHost);
        const ConsistencyReport &rep = summary.consistency;
        QVector<QStringList> rows;
        QVector<int> warnRows;
        for (int i = 0; i < rep.rows.size(); ++i) {
            const ConsistencyRow &r = rep.rows.at(i);
            rows.append({
                r.item,
                r.expected.isEmpty() ? QString::fromUtf8("—") : r.expected,
                r.found.isEmpty() ? QString::fromUtf8("—") : r.found,
                r.conclusion,
            });
            if (!r.consistent)
                warnRows.append(i);
        }
        if (rows.isEmpty())
            rows.append({QString::fromUtf8("（未找到 solver_meta.json）"),
                         QString::fromUtf8("—"), QString::fromUtf8("—"),
                         QString::fromUtf8("未核查")});
        m_consistencyHost->addWidget(makeTable(
            {QString::fromUtf8("核查项"), QString::fromUtf8("全局约定/期望"),
             QString::fromUtf8("数据中取值"), QString::fromUtf8("结论")},
            rows, TableOptions{warnRows, {}}));
    }

    if (m_consistencyStatus) {
        const ConsistencyReport &rep = summary.consistency;
        QString text;
        bool warn = false;
        if (rep.checkedCases == 0) {
            warn = true;
            text = QString::fromUtf8("未找到任何 solver_meta.json，无法核查计算口径。");
        } else if (rep.deviations.isEmpty()) {
            text = QString::fromUtf8("计算口径一致：%1 个算例、%2 项核查全部一致。")
                       .arg(rep.checkedCases)
                       .arg(rep.rows.size());
        } else {
            warn = true;
            text = QString::fromUtf8("发现 %1 项问题：").arg(rep.deviations.size())
                   + rep.deviations.join(QString::fromUtf8("　"));
        }
        m_consistencyStatus->setObjectName(warn ? QStringLiteral("StatusWarn")
                                                : QStringLiteral("StatusGood"));
        m_consistencyStatus->setText(text);
        m_consistencyStatus->style()->unpolish(m_consistencyStatus);
        m_consistencyStatus->style()->polish(m_consistencyStatus);
    }

    if (m_coverageKpiHost) {
        clearLayout(m_coverageKpiHost);
        const CoverageReport &cov = summary.coverage;
        m_coverageKpiHost->addWidget(makeKpis({
            {QString::fromUtf8("算例数"), QString::number(cov.cases), QString::fromUtf8("个")},
            {QString::fromUtf8("外形数"), QString::number(cov.geometries), QString::fromUtf8("个")},
            {QString::fromUtf8("工况点"), QString::number(cov.conditionPoints), QString::fromUtf8("个")},
            {QString::fromUtf8("联合覆盖"), QString::number(cov.jointRatio * 100.0, 'f', 0), QStringLiteral("%")},
        }));
    }

    if (m_coverageDimHost) {
        clearLayout(m_coverageDimHost);
        const CoverageReport &cov = summary.coverage;
        QVector<QStringList> rows;
        for (const CoverageDim &d : cov.dims) {
            const QString range = QString::fromUtf8("%1 ~ %2")
                                      .arg(d.minValue, 0, 'f', 3)
                                      .arg(d.maxValue, 0, 'f', 3)
                                  + (d.unit.isEmpty() ? QString() : QLatin1Char(' ') + d.unit);
            rows.append({d.name, d.kind, QString::number(d.distinctCount), range});
        }
        if (rows.isEmpty())
            rows.append({QString::fromUtf8("（无数据）"), QString::fromUtf8("—"),
                         QString::fromUtf8("—"), QString::fromUtf8("—")});
        m_coverageDimHost->addWidget(makeTable(
            {QString::fromUtf8("维度"), QString::fromUtf8("类型"),
             QString::fromUtf8("取值个数"), QString::fromUtf8("取值范围")},
            rows, TableOptions{}));
    }

    if (m_coverageStatus) {
        const CoverageReport &cov = summary.coverage;
        QStringList lines = cov.notes + cov.suggestions;
        bool warn = !lines.isEmpty();
        QString text = warn
                           ? QString::fromUtf8("覆盖分析：") + lines.join(QString::fromUtf8("　"))
                           : QString::fromUtf8("覆盖分析：联合空间在当前样本内无明显空缺。");
        m_coverageStatus->setObjectName(warn ? QStringLiteral("StatusWarn")
                                             : QStringLiteral("StatusGood"));
        m_coverageStatus->setText(text);
        m_coverageStatus->style()->unpolish(m_coverageStatus);
        m_coverageStatus->style()->polish(m_coverageStatus);
    }

    if (m_rootDir && !summary.rootDir.isEmpty())
        m_rootDir->setText(summary.rootDir);

    if (m_sourceStatus) {
        const QString curLine = QString::fromUtf8("　审定：可用 %1（训练 %2 / 验证 %3），排除 %4。")
                                    .arg(summary.curation.usableCount)
                                    .arg(summary.curation.trainCount)
                                    .arg(summary.curation.validationCount)
                                    .arg(summary.curation.excludedCount);
        if (!summary.warnings.isEmpty()) {
            m_sourceStatus->setObjectName(QStringLiteral("StatusWarn"));
            m_sourceStatus->setText(summary.warnings.join(QString::fromUtf8("　")) + curLine);
        } else {
            m_sourceStatus->setObjectName(QStringLiteral("StatusGood"));
            m_sourceStatus->setText(QString::fromUtf8("数据集已导入：%1(%2 个算例，参数维度 %3)。")
                                        .arg(summary.name)
                                        .arg(summary.totalCases)
                                        .arg(summary.paramDimension)
                                    + curLine);
        }
        m_sourceStatus->style()->unpolish(m_sourceStatus);
        m_sourceStatus->style()->polish(m_sourceStatus);
    }
}

void DataPage::showError(const QString &message)
{
    QMessageBox::warning(this, QString::fromUtf8("导入数据"), message);
    setPlaceholder(message);
}

void DataPage::setPlaceholder(const QString &message)
{
    if (m_sourceStatus) {
        m_sourceStatus->setObjectName(QStringLiteral("StatusWarn"));
        m_sourceStatus->setText(message);
        m_sourceStatus->style()->unpolish(m_sourceStatus);
        m_sourceStatus->style()->polish(m_sourceStatus);
    }
}

void DataPage::requestImport()
{
    const QString start = (m_rootDir && !m_rootDir->text().isEmpty())
                              ? m_rootDir->text()
                              : QDir::currentPath();
    const QString dir = QFileDialog::getExistingDirectory(
        this, QString::fromUtf8("选择数据集根目录（含 dataset.json）"), start);
    if (dir.isEmpty())
        return;
    if (m_rootDir)
        m_rootDir->setText(dir);
    emit importRequested(dir);
}

void DataPage::requestExport()
{
    if (!m_hasData) {
        QMessageBox::information(this, QString::fromUtf8("导出报告与训练清单"),
                                 QString::fromUtf8("请先导入数据集，再导出报告与训练清单。"));
        return;
    }
    const QString start = (m_rootDir && !m_rootDir->text().isEmpty())
                              ? m_rootDir->text()
                              : QDir::currentPath();
    const QString dir = QFileDialog::getExistingDirectory(
        this, QString::fromUtf8("选择导出目录"), start);
    if (dir.isEmpty())
        return;
    emit exportRequested(dir);
}

void DataPage::showExportResult(const QStringList &files)
{
    const CurationReport &c = m_summary.curation;
    const QString msg =
        QString::fromUtf8("已导出以下文件：\n%1\n\n审定：可用 %2（训练 %3 / 验证 %4），排除 %5。\n"
                          "training_manifest.json 可供代理训练模块直接消费。")
            .arg(files.join(QLatin1Char('\n')))
            .arg(c.usableCount)
            .arg(c.trainCount)
            .arg(c.validationCount)
            .arg(c.excludedCount);
    QMessageBox::information(this, QString::fromUtf8("导出报告与训练清单"), msg);
}

void DataPage::tryAutoLoad()
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        appDir + QStringLiteral("/sample_dataset"),
        appDir + QStringLiteral("/../sample_dataset"),
        appDir + QStringLiteral("/../../sample_dataset"),
        QDir::currentPath() + QStringLiteral("/sample_dataset"),
    };
    for (const QString &c : candidates) {
        if (QFile::exists(c + QStringLiteral("/dataset.json"))) {
            const QString resolved = QDir(c).absolutePath();
            if (m_rootDir)
                m_rootDir->setText(resolved);
            m_presenter->importFrom(resolved, false);
            return;
        }
    }
    setPlaceholder(QString::fromUtf8("尚未导入数据。点击“选择目录并导入”选择含 dataset.json 的数据集根目录。"));
}

QWidget *DataPage::buildConsistencyPage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *about = makePanel();
    auto *al = qobject_cast<QVBoxLayout *>(about->layout());
    al->addWidget(makePanelTitle(QString::fromUtf8("关于本页"), QString::fromUtf8("数据来源与处理逻辑")));
    auto *aboutText = new QLabel(QString::fromUtf8(
        "数据来源：读取数据集根目录下各算例的 cases/case_<id>/solver_meta.json（计算口径元数据），"
        "并与 dataset.json 的 global_conventions（全局约定）比对。\n"
        "处理逻辑：逐算例提取参考总压位置、出口平均方式、湍流模型、出口边界、求解器/版本与收敛状态；"
        "按全局约定或跨算例一致性判定，偏离项标黄并在下方汇总告警。\n"
        "依据：需求《进气道外形优化设计工具方案报告》4.2——检查物理定义、计算设置与性能指标处理方式的一致性，"
        "避免因数据生成方法差异引入与外形/工况无关的性能差异。"));
    aboutText->setObjectName(QStringLiteral("NoteLabel"));
    aboutText->setWordWrap(true);
    al->addWidget(aboutText);
    lay->addWidget(about);

    auto *panel = makePanel();
    auto *pl = qobject_cast<QVBoxLayout *>(panel->layout());
    pl->addWidget(makePanelTitle(QString::fromUtf8("计算口径一致性核查"),
                                 QString::fromUtf8("跨算例"),
                                 QString::fromUtf8("下表由导入数据实时生成：全局约定/期望列缺省时按“跨算例是否唯一”判定一致性。")));
    auto *tableWrap = new QWidget;
    m_consistencyHost = new QVBoxLayout(tableWrap);
    m_consistencyHost->setContentsMargins(0, 0, 0, 0);
    pl->addWidget(tableWrap);
    lay->addWidget(panel);

    m_consistencyStatus = qobject_cast<QLabel *>(
        makeStatusText(QString::fromUtf8("尚未导入数据。"), false));
    lay->addWidget(m_consistencyStatus);
    lay->addStretch();
    return content;
}

QWidget *DataPage::buildCoveragePage()
{
    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(0, 4, 0, 0);
    lay->setSpacing(14);

    auto *about = makePanel();
    auto *al = qobject_cast<QVBoxLayout *>(about->layout());
    al->addWidget(makePanelTitle(QString::fromUtf8("关于本页"), QString::fromUtf8("数据来源与处理逻辑")));
    auto *aboutText = new QLabel(QString::fromUtf8(
        "数据来源：工况变量取自 cases_index.csv（马赫数、攻角、质量流量）；"
        "设计变量取自各算例 cases/case_<id>/design.json（入口/出口俯仰角、Rib-2 下沉等代表性参数）。\n"
        "处理逻辑：统计各维度取值个数与范围；联合空间覆盖率 = 已算“外形×工况”组合 ÷ 目标组合（外形数×工况点）；"
        "加密建议由工况点数、约束越界算例与 DC60 跨度等真实信号推导。\n"
        "依据：需求 4.3（联合设计空间覆盖，不能只看单参数）/ 4.4（对非线性、边界与高误差区加密）。"));
    aboutText->setObjectName(QStringLiteral("NoteLabel"));
    aboutText->setWordWrap(true);
    al->addWidget(aboutText);
    lay->addWidget(about);

    auto *kpiWrap = new QWidget;
    m_coverageKpiHost = new QVBoxLayout(kpiWrap);
    m_coverageKpiHost->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(kpiWrap);

    auto *dimPanel = makePanel();
    auto *dpl = qobject_cast<QVBoxLayout *>(dimPanel->layout());
    dpl->addWidget(makePanelTitle(QString::fromUtf8("维度覆盖"), QString::fromUtf8("设计 × 工况"),
                                  QString::fromUtf8("下表由导入数据实时生成：单个维度的取值个数与范围。")));
    auto *dimWrap = new QWidget;
    m_coverageDimHost = new QVBoxLayout(dimWrap);
    m_coverageDimHost->setContentsMargins(0, 0, 0, 0);
    dpl->addWidget(dimWrap);
    lay->addWidget(dimPanel);

    m_coverageStatus = qobject_cast<QLabel *>(
        makeStatusText(QString::fromUtf8("尚未导入数据。"), false));
    m_coverageStatus->setWordWrap(true);
    lay->addWidget(m_coverageStatus);
    lay->addStretch();
    return content;
}
