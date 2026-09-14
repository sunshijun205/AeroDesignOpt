#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include <QVector>

class DataPage;

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
    void onActionClicked(QPushButton *button);

    QStackedWidget *m_pages = nullptr;
    DataPage *m_dataPage = nullptr;
    QPushButton *m_primary = nullptr;
    QPushButton *m_secondary = nullptr;
    QVector<QPushButton *> m_modeButtons;
};

#endif
