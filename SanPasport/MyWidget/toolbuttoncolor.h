#ifndef TOOLBOTTONCOLOR_H
#define TOOLBOTTONCOLOR_H

#include <QToolButton>
#include <QColorDialog>
#include <QColor>
#include <QDebug>


class ToolButtonColor : public QToolButton
{
    Q_OBJECT

public:
    ToolButtonColor(QWidget *){ }

public slots:

    /* ------- Выбрать цвет ------- */
    QColor setColor()
    {
        QColor Color( QColorDialog::getColor(Qt::red));

        if(Color.isValid())
        {
            setStyleSheet("");
            setStyleSheet("background-color: " + Color.name());
        }

        return Color;
    }

    /* ------- Текущий цвет ------- */
    QColor currentColor()
    {
        QColor clrCurrent;
        if(styleSheet().split(":").size() == 2) {
            clrCurrent.setNamedColor(styleSheet().split(":").at(1).simplified()); }

        return clrCurrent;
    }

    /* ------- Установить цвет фона ------- */
    void colorBackGround(QColor clrBack)
    {
        setStyleSheet("");
        setStyleSheet("background-color: " + clrBack.name());
    }

private:
};

#endif // TOOLBOTTONCOLOR_H
