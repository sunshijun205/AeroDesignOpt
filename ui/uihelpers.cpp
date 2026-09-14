#include "uihelpers.h"

#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>

QPushButton *makeButton(const QString &text, bool primary, QWidget *parent)
{
    auto *btn = new QPushButton(text, parent);
    if (primary)
        btn->setObjectName(QStringLiteral("PrimaryButton"));
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

QComboBox *makeSelect(const QString &current, const QStringList &others, QWidget *parent)
{
    auto *box = new QComboBox(parent);
    box->addItem(current);
    for (const QString &item : others)
        if (item != current)
            box->addItem(item);
    box->setCurrentText(current);
    return box;
}

QLineEdit *makeInput(const QString &value, QWidget *parent)
{
    auto *edit = new QLineEdit(value, parent);
    return edit;
}

QWidget *makeLabeled(const QString &label, QWidget *control, const QString &unit, QWidget *parent)
{
    auto *wrap = new QWidget(parent);
    auto *lay = new QVBoxLayout(wrap);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(5);
    auto *lbl = new QLabel(label);
    lbl->setObjectName(QStringLiteral("FieldLabel"));
    lay->addWidget(lbl);
    if (unit.isEmpty()) {
        lay->addWidget(control);
    } else {
        auto *row = new QHBoxLayout;
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(6);
        row->addWidget(control, 1);
        auto *unitLbl = new QLabel(unit);
        unitLbl->setObjectName(QStringLiteral("UnitLabel"));
        row->addWidget(unitLbl);
        lay->addLayout(row);
    }
    return wrap;
}

QWidget *makeField(const QString &label, const QString &value, const QString &unit, QWidget *parent)
{
    return makeLabeled(label, makeInput(value), unit, parent);
}

QWidget *makeSelectField(const QString &label, const QString &current, const QStringList &others, QWidget *parent)
{
    return makeLabeled(label, makeSelect(current, others), {}, parent);
}

QCheckBox *makeCheck(const QString &label, bool checked, QWidget *parent)
{
    auto *box = new QCheckBox(label, parent);
    box->setChecked(checked);
    return box;
}

QFrame *makePanel(QWidget *parent)
{
    auto *frame = new QFrame(parent);
    frame->setObjectName(QStringLiteral("Panel"));
    auto *lay = new QVBoxLayout(frame);
    lay->setContentsMargins(16, 16, 16, 16);
    lay->setSpacing(12);
    return frame;
}

QWidget *makePanelTitle(const QString &title, const QString &meta, const QString &subtitle, QWidget *parent)
{
    auto *wrap = new QWidget(parent);
    auto *lay = new QVBoxLayout(wrap);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(3);
    auto *row = new QHBoxLayout;
    row->setContentsMargins(0, 0, 0, 0);
    auto *titleLbl = new QLabel(title);
    titleLbl->setObjectName(QStringLiteral("PanelTitle"));
    row->addWidget(titleLbl);
    row->addStretch();
    if (!meta.isEmpty()) {
        auto *metaLbl = new QLabel(meta);
        metaLbl->setObjectName(QStringLiteral("MetaLabel"));
        row->addWidget(metaLbl);
    }
    lay->addLayout(row);
    if (!subtitle.isEmpty()) {
        auto *sub = new QLabel(subtitle);
        sub->setObjectName(QStringLiteral("NoteLabel"));
        sub->setWordWrap(true);
        lay->addWidget(sub);
    }
    return wrap;
}

QWidget *makeKpis(const QVector<KpiItem> &items, QWidget *parent)
{
    auto *wrap = new QWidget(parent);
    auto *lay = new QHBoxLayout(wrap);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    for (const KpiItem &item : items) {
        auto *card = new QFrame;
        card->setObjectName(QStringLiteral("KpiCard"));
        auto *cl = new QVBoxLayout(card);
        cl->setContentsMargins(14, 12, 14, 12);
        cl->setSpacing(4);
        auto *cap = new QLabel(item.label);
        cap->setObjectName(QStringLiteral("KpiCaption"));
        auto *valRow = new QHBoxLayout;
        valRow->setContentsMargins(0, 0, 0, 0);
        valRow->setSpacing(4);
        auto *val = new QLabel(item.value);
        val->setObjectName(QStringLiteral("KpiValue"));
        valRow->addWidget(val);
        if (!item.unit.isEmpty()) {
            auto *unit = new QLabel(item.unit);
            unit->setObjectName(QStringLiteral("KpiUnit"));
            valRow->addWidget(unit);
        }
        valRow->addStretch();
        cl->addWidget(cap);
        cl->addLayout(valRow);
        lay->addWidget(card, 1);
    }
    return wrap;
}

QWidget *makeHeading(const QString &title, const QString &subtitle, QWidget *parent)
{
    auto *wrap = new QWidget(parent);
    auto *lay = new QVBoxLayout(wrap);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(4);
    auto *titleLbl = new QLabel(title);
    titleLbl->setObjectName(QStringLiteral("PageTitle"));
    lay->addWidget(titleLbl);
    if (!subtitle.isEmpty()) {
        auto *sub = new QLabel(subtitle);
        sub->setObjectName(QStringLiteral("PageSubtitle"));
        sub->setWordWrap(true);
        lay->addWidget(sub);
    }
    return wrap;
}

QWidget *makeFieldGrid(const QList<QWidget *> &fields, int columns, QWidget *parent)
{
    auto *wrap = new QWidget(parent);
    auto *grid = new QGridLayout(wrap);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(14);
    grid->setVerticalSpacing(12);
    for (int i = 0; i < fields.size(); ++i)
        grid->addWidget(fields[i], i / columns, i % columns);
    return wrap;
}

QTableWidget *makeTable(const QStringList &headers, const QVector<QStringList> &rows,
                        const TableOptions &options, QWidget *parent)
{
    auto *table = new QTableWidget(rows.size(), headers.size(), parent);
    table->setHorizontalHeaderLabels(headers);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setFocusPolicy(Qt::NoFocus);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setShowGrid(false);
    for (int r = 0; r < rows.size(); ++r) {
        const QStringList &cells = rows[r];
        for (int c = 0; c < headers.size() && c < cells.size(); ++c) {
            auto *item = new QTableWidgetItem(cells[c]);
            if (options.warnRows.contains(r))
                item->setForeground(QColor("#b46b22"));
            table->setItem(r, c, item);
        }
    }
    int rowH = 34;
    table->verticalHeader()->setDefaultSectionSize(rowH);
    table->setMinimumHeight(rowH * (rows.size() + 1) + 8);
    return table;
}

QScrollArea *wrapScroll(QWidget *content, QWidget *parent)
{
    auto *scroll = new QScrollArea(parent);
    scroll->setWidgetResizable(true);
    scroll->setWidget(content);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    return scroll;
}

QWidget *makeStatusText(const QString &text, bool warn, QWidget *parent)
{
    auto *lbl = new QLabel(text, parent);
    lbl->setObjectName(warn ? QStringLiteral("StatusWarn") : QStringLiteral("StatusGood"));
    lbl->setWordWrap(true);
    return lbl;
}

QLabel *makeChip(const QString &text, bool neutral, QWidget *parent)
{
    auto *lbl = new QLabel(text, parent);
    lbl->setObjectName(neutral ? QStringLiteral("ChipNeutral") : QStringLiteral("Chip"));
    return lbl;
}

void wireDummyAction(QAbstractButton *button, QWidget *dialogParent)
{
    if (!button)
        return;
    const QString label = button->text();
    QObject::connect(button, &QAbstractButton::clicked, button, [label, dialogParent]() {
        QMessageBox::information(dialogParent, QString::fromUtf8("气动设计优化平台"),
                                 label + QString::fromUtf8(" — 原型交互已记录。"));
    });
}

SubTabBar::SubTabBar(const QVector<QPair<QString, QString>> &items,
                     const QString &activeId, QWidget *parent)
    : QFrame(parent), m_currentId(activeId)
{
    setObjectName(QStringLiteral("SubTabBar"));
    auto *lay = new QHBoxLayout(this);
    lay->setContentsMargins(6, 6, 6, 6);
    lay->setSpacing(4);
    auto *group = new QButtonGroup(this);
    group->setExclusive(true);
    for (const auto &item : items) {
        auto *btn = new QPushButton(item.second);
        btn->setObjectName(QStringLiteral("SubTabButton"));
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setChecked(item.first == activeId);
        group->addButton(btn);
        lay->addWidget(btn);
        const QString id = item.first;
        connect(btn, &QPushButton::clicked, this, [this, id]() {
            m_currentId = id;
            emit currentChanged(id);
        });
    }
    lay->addStretch();
}

void SubTabBar::setActive(const QString &id)
{
    m_currentId = id;
}

QString SubTabBar::currentId() const
{
    return m_currentId;
}
