#ifndef MYSITPLANVIEW_H
#define MYSITPLANVIEW_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>

class MySitPlanView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit MySitPlanView(QWidget *parent) :  QGraphicsView(parent) {}

signals:
    void sendMousePoint(QPointF point);

public slots:
    void mouseMoveEvent (QMouseEvent *move) {
        emit sendMousePoint(move->pos());
        QGraphicsView::mouseMoveEvent(move);
    }

private:
};

#endif // MYSITPLANVIEW_H
