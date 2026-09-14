#ifndef THEME_H
#define THEME_H

#include <QColor>
#include <QString>

namespace Theme {

inline QColor bg() { return QColor("#eef1f6"); }
inline QColor shell() { return QColor("#f8fafc"); }
inline QColor panel() { return QColor("#ffffff"); }
inline QColor panel2() { return QColor("#f2f5fa"); }
inline QColor line() { return QColor("#d6dde8"); }
inline QColor text() { return QColor("#16202f"); }
inline QColor muted() { return QColor("#697687"); }
inline QColor accent() { return QColor("#2f6fed"); }
inline QColor accentSoft() { return QColor("#e6eefe"); }
inline QColor warn() { return QColor("#b46b22"); }
inline QColor navText() { return QColor("#334255"); }

inline QString styleSheet()
{
    return QStringLiteral(R"(
QMainWindow, QWidget#RootShell {
    background: #eef1f6;
    color: #16202f;
    font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
    font-size: 13px;
}
QLabel { color: #16202f; }
QLabel#BrandLabel {
    font-size: 18px;
    font-weight: 500;
}
QLabel#ProjectLabel {
    color: #697687;
    font-size: 13px;
}
QLabel#MutedLabel, QLabel#FieldLabel, QLabel#UnitLabel, QLabel#MetaLabel, QLabel#NoteLabel {
    color: #697687;
}
QLabel#FieldLabel { font-size: 12px; }
QLabel#UnitLabel {
    font-size: 12px;
    background: #f2f5fa;
    border: 1px solid #d6dde8;
    border-radius: 5px;
    padding: 0 8px;
}
QLabel#KpiCaption { color: #697687; font-size: 11px; }
QLabel#KpiValue { font-size: 18px; font-weight: 500; }
QLabel#KpiUnit { color: #697687; font-size: 11px; }
QLabel#PanelTitle { font-size: 14px; font-weight: 500; }
QLabel#SectionTitle { font-size: 14px; font-weight: 500; }
QLabel#PageTitle { font-size: 21px; font-weight: 500; }
QLabel#PageSubtitle { color: #697687; font-size: 13px; }
QLabel#Chip {
    background: #e6eefe;
    color: #16202f;
    border-radius: 10px;
    padding: 2px 7px;
    font-size: 11px;
}
QLabel#ChipNeutral {
    background: #f2f5fa;
    color: #697687;
    border-radius: 10px;
    padding: 2px 7px;
    font-size: 11px;
}
QLabel#StatusGood { color: #2f6fed; }
QLabel#StatusWarn { color: #b46b22; }
QPushButton {
    border: 1px solid #d6dde8;
    background: #ffffff;
    color: #16202f;
    padding: 8px 13px;
    border-radius: 6px;
}
QPushButton:hover { background: #f2f5fa; }
QPushButton#PrimaryButton {
    background: #2f6fed;
    color: white;
    border: 1px solid #2f6fed;
}
QPushButton#PrimaryButton:hover { background: #2861d6; }
QPushButton#ModeButton {
    min-height: 46px;
    border: 1px solid #d6dde8;
    background: #ffffff;
    color: #334255;
    border-radius: 7px;
    font-size: 16px;
    font-weight: 500;
}
QPushButton#ModeButton:hover { background: #e6eefe; color: #16202f; }
QPushButton#ModeButton:checked {
    background: #2f6fed;
    color: white;
    border: 1px solid #2f6fed;
    font-weight: 500;
}
QPushButton#SubTabButton {
    border: 0;
    background: transparent;
    color: #334255;
    padding: 8px 16px;
    border-radius: 5px;
    font-size: 14px;
    font-weight: 500;
}
QPushButton#SubTabButton:hover { background: #e6eefe; color: #16202f; }
QPushButton#SubTabButton:checked {
    background: #2f6fed;
    color: white;
}
QFrame#TopBar, QFrame#FunctionBar {
    background: #f8fafc;
}
QFrame#TopBar { border-bottom: 1px solid #d6dde8; }
QFrame#FunctionBar { border-bottom: 1px solid #d6dde8; }
QFrame#Panel, QFrame#Section, QFrame#KpiCard, QFrame#SubTabBar {
    background: #ffffff;
    border: 1px solid #d6dde8;
    border-radius: 8px;
}
QFrame#KpiCard { border-radius: 7px; }
QFrame#SubTabBar {
    background: #f2f5fa;
    border-radius: 7px;
}
QFrame#PreviewCanvas {
    background: #f2f5fa;
    border-radius: 6px;
}
QLineEdit, QComboBox {
    height: 33px;
    border: 1px solid #d6dde8;
    border-radius: 5px;
    background: #ffffff;
    color: #16202f;
    padding: 0 9px;
    font-size: 13px;
}
QComboBox::drop-down { border: 0; width: 20px; }
QCheckBox { color: #16202f; font-size: 12px; spacing: 7px; }
QCheckBox::indicator {
    width: 14px;
    height: 14px;
}
QTableWidget {
    background: #ffffff;
    border: 0;
    gridline-color: #d6dde8;
    font-size: 12px;
    selection-background-color: #f2f5fa;
    selection-color: #16202f;
}
QTableWidget::item { padding: 4px 6px; }
QHeaderView::section {
    background: #ffffff;
    color: #697687;
    border: 0;
    border-bottom: 1px solid #d6dde8;
    font-weight: 400;
    font-size: 12px;
    padding: 7px 8px;
    text-align: left;
}
QProgressBar {
    border: 0;
    background: #f2f5fa;
    border-radius: 5px;
    height: 7px;
    text-align: center;
    color: transparent;
    max-height: 7px;
}
QProgressBar::chunk {
    background: #2f6fed;
    border-radius: 5px;
}
QScrollArea { border: 0; background: #eef1f6; }
QScrollBar:vertical {
    background: transparent;
    width: 10px;
    margin: 0;
}
QScrollBar::handle:vertical {
    background: #d6dde8;
    border-radius: 5px;
    min-height: 24px;
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
)");
}

} // namespace Theme

#endif
