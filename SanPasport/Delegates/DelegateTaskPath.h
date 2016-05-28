#ifndef DELEGATETASKPATH_H
#define DELEGATETASKPATH_H

#include <QString>
#include <QStyledItemDelegate>
#include <QFile>

class DelegateTaskPath : public QStyledItemDelegate
{
    Q_OBJECT

public:
    DelegateTaskPath(QWidget *parent=0) : QStyledItemDelegate(parent) {}

    QString displayText (const QVariant &value, const QLocale &locale) const
    {
        QString text = value.toString();

        if (text == "") {
            text = "";
        } else if (text != "") {
            if (QFile(text).exists())
                text = "готово";
            else if (text == "calc")
                text = "расчет...";
            else
                text = "не найден";
        }

        return QStyledItemDelegate::displayText(text, locale);
    }
};

#endif // DELEGATETASKPATH_H
