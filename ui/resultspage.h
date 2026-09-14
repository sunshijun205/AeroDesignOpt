#ifndef RESULTSPAGE_H
#define RESULTSPAGE_H

#include <QWidget>

class QStackedWidget;

// 结果页（UI 原型）。
class ResultsPage : public QWidget
{
    Q_OBJECT
public:
    explicit ResultsPage(QWidget *parent = nullptr);

private:
    QWidget *buildPolarPage();
    QWidget *buildConvergencePage();
    QWidget *buildComparePage();

    QStackedWidget *m_inner = nullptr;
};

#endif
