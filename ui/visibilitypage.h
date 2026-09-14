#ifndef VISIBILITYPAGE_H
#define VISIBILITYPAGE_H

#include <QWidget>

class QStackedWidget;

// 全遮挡评价页（UI 原型，静态内容）。
// 对应模块 M2 全遮挡判定 + M3 几何约束：观察/目标范围设置与遮挡判定结果。
class VisibilityPage : public QWidget
{
    Q_OBJECT
public:
    explicit VisibilityPage(QWidget *parent = nullptr);

private:
    QWidget *buildSpecPage();
    QWidget *buildResultPage();
    QWidget *buildConstraintPage();

    QStackedWidget *m_inner = nullptr;
};

#endif
