#ifndef MODELTASKS_H
#define MODELTASKS_H

#include <QColor>
#include <QSqlTableModel>

class ModelTasks : public QSqlTableModel
{
    Q_OBJECT

public:
    enum Name { Id = 1, Name = 2};

    ModelTasks(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());
    ~ModelTasks();
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
};

#endif // MODELTASKS_H
