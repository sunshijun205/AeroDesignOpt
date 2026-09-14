#ifndef DUCTDESIGNPAGE_H
#define DUCTDESIGNPAGE_H

#include <QWidget>

class QStackedWidget;

// 参数化设计页（UI 原型，静态内容）。
// 对应模块 M1 几何生成：统一设计参数（入口/走向/截面）驱动 PicoGK 生成三维外形。
class DuctDesignPage : public QWidget
{
    Q_OBJECT
public:
    explicit DuctDesignPage(QWidget *parent = nullptr);

private:
    QWidget *buildParamsPage();
    QWidget *buildSpinePage();
    QWidget *buildSectionPage();
    QWidget *buildPreviewPage();

    QStackedWidget *m_inner = nullptr;
};

#endif
