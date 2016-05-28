#ifndef DELEGATESTATUS_H
#define DELEGATESTATUS_H

#include <QString>
#include <QStyledItemDelegate>

class DelegateStatus : public QStyledItemDelegate
{
    Q_OBJECT

public:
    DelegateStatus(QWidget *parent=0) : QStyledItemDelegate(parent) {}

    QString displayText(const QVariant &value, const QLocale &locale) const
    {
        QString text = value.toString();
        text = (text == "1")? "да" : "нет";
        return QStyledItemDelegate::displayText(text, locale);
    }
};

#endif // DELEGATESTATUS_H
