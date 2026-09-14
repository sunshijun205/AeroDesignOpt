#ifndef OPTIMIZATIONPAGE_H
#define OPTIMIZATIONPAGE_H

#include <QWidget>

class QStackedWidget;

// 优化页（UI 原型，静态内容）。
// 对应模块 M5 优化引擎 + M6 评价聚合：设计变量、目标/约束、运行监控。
class OptimizationPage : public QWidget
{
    Q_OBJECT
public:
    explicit OptimizationPage(QWidget *parent = nullptr);

private:
    QWidget *buildVariablesPage();
    QWidget *buildObjectivePage();
    QWidget *buildRunPage();

    QStackedWidget *m_inner = nullptr;
};

#endif
