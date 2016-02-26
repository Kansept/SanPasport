#ifndef TABLEVIEWSP_H
#define TABLEVIEWSP_H

#include <QTableView>
#include <QKeyEvent>
#include <QDebug>

// ----------------------------------- TableView для загрузки ЗО ----------------------------------- //
class TableViewSp : public QTableView
{
    Q_OBJECT
public:
    TableViewSp(QWidget *parent = 0) : QTableView(parent) { }


signals:
    void currentChanged( const QModelIndex & current, const QModelIndex & previous );
    void keyPressed(int, int);

public:
    void keyPressEvent(QKeyEvent *event)
    {
        emit keyPressed(event->key(), event->modifiers());
    }
};

#endif // TABLEVIEWSP_H
