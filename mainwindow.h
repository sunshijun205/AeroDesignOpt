#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include <QVector>

class AnalysisPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void switchMode(int index);
    void updateActions(int index);
    void onPrimaryClicked();
    void onSecondaryClicked();

    QStackedWidget *m_pages = nullptr;
    AnalysisPage *m_analysisPage = nullptr;
    QPushButton *m_primary = nullptr;
    QPushButton *m_secondary = nullptr;
    QVector<QPushButton *> m_modeButtons;
};

#endif
