#ifndef DELEGATETASKDESCRIPTION_H
#define DELEGATETASKDESCRIPTION_H

#include <QString>
#include <QStyledItemDelegate>

class DelegateTaskDescription : public QStyledItemDelegate
{
    Q_OBJECT

public:
    DelegateTaskDescription(QWidget *parent=0) : QStyledItemDelegate(parent) {}

    QString displayText(const QVariant &value, const QLocale &locale) const
    {
        QString text = value.toString();
        QStringList params;

        params = text.split(";");
        text.clear();

        // Для расчета ЭМП в точке
        if (params.size() == 3) {
            text.append("Расчёт уровней ЭМП с координатами, ");
            text.append("X = ").append(params.at(0)).append(" м, ");
            text.append("Y = ").append(params.at(1)).append(" м, ");
            text.append("Z = ").append(params.at(2)).append(" м");
        }
        // Для расчета Зоны ограничения
        if (params.size() == 8) {
            text.append("Расчёт зоны ограничения, ");
            text.append("Высота среза = ").append(params.at(6)).append(" м, ");
            text.append("Xmin = ").append(params.at(0)).append(" м, ");
            text.append("Xmax = ").append(params.at(1)).append(" м, ");
            text.append("dX = ").append(params.at(2)).append(" м, ");
            text.append("Ymin = ").append(params.at(3)).append(" м, ");
            text.append("Ymax = ").append(params.at(4)).append(" м, ");
            text.append("dY = ").append(params.at(5)).append(" м; ");
            if(params.at(7) == "0")
                text.append("Расчет границ зоны");
            else if(params.at(7) == "3")
                text.append("Текстовый расчёт");
        }
        // Для расчета Вертикального сечения
        if (params.size() == 10) {
            text.append("Расчёт вертикального сечения, ");
            text.append("Азимут = ").append(params.at(8)).append(" град. ");
            text.append("X = ").append(params.at(0)).append(" м, ");
            text.append("Y = ").append(params.at(1)).append(" м, ");
            text.append("Rmin = ").append(params.at(2)).append(" м, ");
            text.append("Rmax = ").append(params.at(3)).append(" м, ");
            text.append("dR = ").append(params.at(4)).append(" м, ");
            text.append("Hmin = ").append(params.at(5)).append(" м, ");
            text.append("Hmax = ").append(params.at(6)).append(" м, ");
            text.append("dH = ").append(params.at(7)).append(" м; ");
            if (params.at(9) == "0")
                text.append("Расчет границ зоны");
            else if (params.at(9) == "3")
                text.append("Текстовый расчёт");
        }
        return QStyledItemDelegate::displayText(text, locale);
    }
};


#endif // DELEGATETASKDESCRIPTION_H
