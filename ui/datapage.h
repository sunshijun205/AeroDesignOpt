#ifndef DATAPAGE_H
#define DATAPAGE_H

#include <QWidget>

class QStackedWidget;

// 数据管理页（UI 原型，静态内容）。
// 对应模块 M7 数据集管理：数据源导入、一致性审查、联合空间覆盖度。
class DataPage : public QWidget
{
    Q_OBJECT
public:
    explicit DataPage(QWidget *parent = nullptr);

private:
    QWidget *buildSourcePage();
    QWidget *buildConsistencyPage();
    QWidget *buildCoveragePage();

    QStackedWidget *m_inner = nullptr;
};

#endif
