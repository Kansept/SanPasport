#ifndef MODELANTENNAS_H
#define MODELANTENNAS_H

#include <QSqlTableModel>
#include <QColor>

class ModelAntennas : public QSqlTableModel
{
    Q_OBJECT

public:
    ModelAntennas(QObject * parent = 0, QSqlDatabase db = QSqlDatabase());
    ~ModelAntennas();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
};


#endif // MODELANTENNAS_H
