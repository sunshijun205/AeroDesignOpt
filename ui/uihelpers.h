#ifndef UIHELPERS_H
#define UIHELPERS_H

#include <QAbstractButton>
#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPair>
#include <QPushButton>
#include <QScrollArea>
#include <QStringList>
#include <QTableWidget>
#include <QVector>
#include <QWidget>

struct KpiItem {
    QString label;
    QString value;
    QString unit;
};

struct TableOptions {
    QVector<int> warnRows;
    QVector<int> chipColumns;
};

QPushButton *makeButton(const QString &text, bool primary = false, QWidget *parent = nullptr);
QComboBox *makeSelect(const QString &current, const QStringList &others = {}, QWidget *parent = nullptr);
QLineEdit *makeInput(const QString &value, QWidget *parent = nullptr);
QWidget *makeLabeled(const QString &label, QWidget *control, const QString &unit = {}, QWidget *parent = nullptr);
QWidget *makeField(const QString &label, const QString &value, const QString &unit = {}, QWidget *parent = nullptr);
QWidget *makeSelectField(const QString &label, const QString &current, const QStringList &others = {}, QWidget *parent = nullptr);
QCheckBox *makeCheck(const QString &label, bool checked = true, QWidget *parent = nullptr);
QFrame *makePanel(QWidget *parent = nullptr);
QWidget *makePanelTitle(const QString &title, const QString &meta = {}, const QString &subtitle = {}, QWidget *parent = nullptr);
QWidget *makeKpis(const QVector<KpiItem> &items, QWidget *parent = nullptr);
QWidget *makeHeading(const QString &title, const QString &subtitle, QWidget *parent = nullptr);
QWidget *makeFieldGrid(const QList<QWidget *> &fields, int columns = 2, QWidget *parent = nullptr);
QTableWidget *makeTable(const QStringList &headers, const QVector<QStringList> &rows,
                        const TableOptions &options = {}, QWidget *parent = nullptr);
QScrollArea *wrapScroll(QWidget *content, QWidget *parent = nullptr);
QFrame *makeCanvas(const QString &caption, int minHeight = 220, QWidget *parent = nullptr);
QWidget *makeStatusText(const QString &text, bool warn = false, QWidget *parent = nullptr);
QLabel *makeChip(const QString &text, bool neutral = false, QWidget *parent = nullptr);
void wireDummyAction(QAbstractButton *button, QWidget *dialogParent);

class SubTabBar : public QFrame
{
    Q_OBJECT
public:
    explicit SubTabBar(const QVector<QPair<QString, QString>> &items,
                       const QString &activeId,
                       QWidget *parent = nullptr);
    void setActive(const QString &id);
    QString currentId() const;

signals:
    void currentChanged(const QString &id);

private:
    QString m_currentId;
};

#endif
