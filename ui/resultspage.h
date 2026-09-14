#ifndef RESULTSPAGE_H
#define RESULTSPAGE_H

#include <QWidget>

class QStackedWidget;

// 结果对比页（UI 原型，静态内容）。
// 对应模块 M8 方案管理 + M6 评价聚合：Pareto 前沿、候选对比、方案详情与 CFD 核验。
class ResultsPage : public QWidget
{
    Q_OBJECT
public:
    explicit ResultsPage(QWidget *parent = nullptr);

private:
    QWidget *buildParetoPage();
    QWidget *buildComparePage();
    QWidget *buildDetailPage();

    QStackedWidget *m_inner = nullptr;
};

#endif
