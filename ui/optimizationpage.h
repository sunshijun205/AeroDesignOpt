#ifndef OPTIMIZATIONPAGE_H
#define OPTIMIZATIONPAGE_H

#include <QWidget>

class QStackedWidget;

// 优化页（UI 原型）。
class OptimizationPage : public QWidget
{
    Q_OBJECT
public:
    explicit OptimizationPage(QWidget *parent = nullptr);

private:
    QWidget *buildObjectivePage();
    QWidget *buildVariablesPage();
    QWidget *buildAlgorithmPage();

    QStackedWidget *m_inner = nullptr;
};

#endif
