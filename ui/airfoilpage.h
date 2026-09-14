#ifndef AIRFOILPAGE_H
#define AIRFOILPAGE_H

#include <QWidget>

class QStackedWidget;

// 翼型设计页（UI 原型）。
class AirfoilPage : public QWidget
{
    Q_OBJECT
public:
    explicit AirfoilPage(QWidget *parent = nullptr);

private:
    QWidget *buildNacaPage();
    QWidget *buildGeometryPage();
    QWidget *buildPreviewPage();

    QStackedWidget *m_inner = nullptr;
};

#endif
