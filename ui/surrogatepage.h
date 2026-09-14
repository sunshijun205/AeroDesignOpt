#ifndef SURROGATEPAGE_H
#define SURROGATEPAGE_H

#include <QWidget>

class QStackedWidget;

// 气动代理页（UI 原型，静态内容）。
// 对应模块 M4 气动代理：参数+工况 → 总压恢复/畸变/旋流 等性能预测。
class SurrogatePage : public QWidget
{
    Q_OBJECT
public:
    explicit SurrogatePage(QWidget *parent = nullptr);

private:
    QWidget *buildConditionPage();
    QWidget *buildModelPage();
    QWidget *buildResultPage();

    QStackedWidget *m_inner = nullptr;
};

#endif
