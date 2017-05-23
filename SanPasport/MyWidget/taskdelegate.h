#ifndef TASKDELEGATE_H
#define TASKDELEGATE_H

#include <QStyledItemDelegate>
#include <QFile>


class TaskParamsDelegate:public QStyledItemDelegate
{
    Q_OBJECT

public:

    TaskParamsDelegate(QWidget *parent=0):QStyledItemDelegate(parent)
    {

    }

    QString displayText(const QVariant &value, const QLocale &locale) const
    {
        QString mytext = value.toString();
        QStringList splstr;

        splstr = mytext.split(";");
        mytext.clear();

        // Для расчета ЭМП в точке
        if (splstr.size() == 3) {
            mytext.append("Расчёт уровней ЭМП с координатами, ");
            mytext.append("X = ").append(splstr.at(0)).append(" м, ");
            mytext.append("Y = ").append(splstr.at(1)).append(" м, ");
            mytext.append("Z = ").append(splstr.at(2)).append(" м");
        }
        // Для расчета Зоны ограничения
        if (splstr.size() == 8) {
            mytext.append("Расчёт зоны ограничения, ");
            mytext.append("Высота среза = ").append(splstr.at(6)).append(" м, ");
            mytext.append("Xmin = ").append(splstr.at(0)).append(" м, ");
            mytext.append("Xmax = ").append(splstr.at(1)).append(" м, ");
            mytext.append("dX = ").append(splstr.at(2)).append(" м, ");
            mytext.append("Ymin = ").append(splstr.at(3)).append(" м, ");
            mytext.append("Ymax = ").append(splstr.at(4)).append(" м, ");
            mytext.append("dY = ").append(splstr.at(5)).append(" м; ");
            if(splstr.at(7) == "0")
                mytext.append("Расчет границ зоны");
            else if(splstr.at(7) == "3")
                mytext.append("Текстовый расчёт");
        }
        // Для расчета Вертикального сечения
        if (splstr.size() == 10) {
            mytext.append("Расчёт вертикального сечения, ");
            mytext.append("Азимут = ").append(splstr.at(8)).append(" град. ");
            mytext.append("X = ").append(splstr.at(0)).append(" м, ");
            mytext.append("Y = ").append(splstr.at(1)).append(" м, ");
            mytext.append("Rmin = ").append(splstr.at(2)).append(" м, ");
            mytext.append("Rmax = ").append(splstr.at(3)).append(" м, ");
            mytext.append("dR = ").append(splstr.at(4)).append(" м, ");
            mytext.append("Hmin = ").append(splstr.at(5)).append(" м, ");
            mytext.append("Hmax = ").append(splstr.at(6)).append(" м, ");
            mytext.append("dH = ").append(splstr.at(7)).append(" м; ");

            if(splstr.at(9) == "0")
                mytext.append("Расчет границ зоны");
            else if(splstr.at(9) == "3")
                mytext.append("Текстовый расчёт");
        }
        return QStyledItemDelegate::displayText(mytext, locale);
    }
};


class TaskStatusDelegate:public QStyledItemDelegate
{
    Q_OBJECT

public:

    TaskStatusDelegate(QWidget *parent=0):QStyledItemDelegate(parent)
    {

    }

    QString displayText(const QVariant &value, const QLocale &locale) const
    {
        QString mytext = value.toString();
        mytext = (mytext == "1")? "да" : "нет";
        return QStyledItemDelegate::displayText(mytext, locale);
    }
};


class TaskPathDelegate:public QStyledItemDelegate
{
    Q_OBJECT

public:

    TaskPathDelegate(QWidget *parent=0):QStyledItemDelegate(parent)
    {

    }

    QString displayText(const QVariant &value, const QLocale &locale) const
    {
        QString mytext = value.toString();

        if (mytext == "") {
            mytext = "";
        } else if (mytext != "") {
            if (QFile(mytext).exists())
                mytext = "готово";
            else if(mytext == "calc")
                mytext = "расчет...";
            else
                mytext = "не найден";
        }

        return QStyledItemDelegate::displayText(mytext, locale);
    }
};

#endif // TASKDELEGATE_H
