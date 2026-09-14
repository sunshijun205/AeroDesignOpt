#include "mainwindow.h"

#include "theme.h"
#include "uihelpers.h"
#include "airfoilpage.h"
#include "analysispage.h"
#include "optimizationpage.h"
#include "resultspage.h"

#include <QButtonGroup>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QString::fromUtf8("气动设计优化平台"));
    resize(1360, 860);
    setMinimumSize(1080, 700);
    menuBar()->hide();
    statusBar()->hide();

    auto *root = new QWidget;
    root->setObjectName(QStringLiteral("RootShell"));
    auto *rootLay = new QVBoxLayout(root);
    rootLay->setContentsMargins(0, 0, 0, 0);
    rootLay->setSpacing(0);

    auto *top = new QFrame;
    top->setObjectName(QStringLiteral("TopBar"));
    top->setFixedHeight(54);
    auto *tl = new QHBoxLayout(top);
    tl->setContentsMargins(22, 0, 22, 0);
    tl->setSpacing(18);
    auto *brand = new QLabel(QString::fromUtf8("气动设计优化平台"));
    brand->setObjectName(QStringLiteral("BrandLabel"));
    auto *divider = new QFrame;
    divider->setFrameShape(QFrame::VLine);
    divider->setStyleSheet(QStringLiteral("color: #d6dde8;"));
    divider->setFixedHeight(18);
    auto *project = new QLabel(QString::fromUtf8("项目：Wing-01 　/　基准翼型 NACA2412"));
    project->setObjectName(QStringLiteral("ProjectLabel"));
    m_secondary = makeButton(QString::fromUtf8("保存分析"));
    m_primary = makeButton(QString::fromUtf8("运行气动分析"), true);
    connect(m_secondary, &QPushButton::clicked, this, &MainWindow::onSecondaryClicked);
    connect(m_primary, &QPushButton::clicked, this, &MainWindow::onPrimaryClicked);
    tl->addWidget(brand);
    tl->addWidget(divider);
    tl->addWidget(project);
    tl->addStretch();
    tl->addWidget(m_secondary);
    tl->addWidget(m_primary);

    auto *func = new QFrame;
    func->setObjectName(QStringLiteral("FunctionBar"));
    auto *fl = new QHBoxLayout(func);
    fl->setContentsMargins(22, 8, 22, 8);
    fl->setSpacing(8);
    const QStringList modes = {
        QString::fromUtf8("翼型设计"),
        QString::fromUtf8("气动分析"),
        QString::fromUtf8("优化"),
        QString::fromUtf8("结果")
    };
    auto *group = new QButtonGroup(this);
    group->setExclusive(true);
    for (int i = 0; i < modes.size(); ++i) {
        auto *btn = new QPushButton(modes[i]);
        btn->setObjectName(QStringLiteral("ModeButton"));
        btn->setCheckable(true);
        btn->setChecked(i == 0);
        btn->setCursor(Qt::PointingHandCursor);
        group->addButton(btn, i);
        fl->addWidget(btn, 1);
        m_modeButtons.append(btn);
        connect(btn, &QPushButton::clicked, this, [this, i]() { switchMode(i); });
    }

    m_pages = new QStackedWidget;
    m_analysisPage = new AnalysisPage;
    m_pages->addWidget(new AirfoilPage);
    m_pages->addWidget(m_analysisPage);
    m_pages->addWidget(new OptimizationPage);
    m_pages->addWidget(new ResultsPage);

    rootLay->addWidget(top);
    rootLay->addWidget(func);
    rootLay->addWidget(m_pages, 1);
    setCentralWidget(root);
    switchMode(0);
}

MainWindow::~MainWindow() = default;

void MainWindow::switchMode(int index)
{
    m_pages->setCurrentIndex(index);
    for (int i = 0; i < m_modeButtons.size(); ++i)
        m_modeButtons[i]->setChecked(i == index);
    updateActions(index);
}

void MainWindow::updateActions(int index)
{
    static const char *primary[] = {
        "生成翼型", "运行气动分析", "开始优化", "导出结果"
    };
    static const char *secondary[] = {
        "重置参数", "保存分析", "保存配置", "刷新"
    };
    m_primary->setText(QString::fromUtf8(primary[index]));
    m_secondary->setText(QString::fromUtf8(secondary[index]));
}

void MainWindow::onPrimaryClicked()
{
    if (m_pages->currentIndex() == 1 && m_analysisPage) {
        m_analysisPage->requestRun();
        return;
    }
    QMessageBox::information(this, QString::fromUtf8("气动设计优化平台"),
                             m_primary->text() + QString::fromUtf8(" — 原型交互已记录。"));
}

void MainWindow::onSecondaryClicked()
{
    if (m_pages->currentIndex() == 1 && m_analysisPage) {
        m_analysisPage->requestSave();
        return;
    }
    QMessageBox::information(this, QString::fromUtf8("气动设计优化平台"),
                             m_secondary->text() + QString::fromUtf8(" — 原型交互已记录。"));
}
