#ifndef QSQUAREWIDGET_H
#define QSQUAREWIDGET_H

#include <QFrame>

class QSquareWidget : public QFrame
{
    Q_OBJECT
public:
    QSquareWidget(QWidget *parent = 0) : QFrame(parent) { }


signals:
    void sgnlRazmer();

public:
    virtual void resizeEvent(QResizeEvent *)
    {
        emit sgnlRazmer();
    }
};

#endif // QSQUAREWIDGET_H

